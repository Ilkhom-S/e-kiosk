/* @file Сервиса, владеющий клиентом БД. */

#include "Services/DatabaseService.h"

#include <QtCore/QFile>
#include <QtCore/QRegularExpression>

#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

#include "DatabaseUtils/DatabaseUtils.h"
#include "Services/EventService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PP = SDK::PaymentProcessor;

namespace CDatabaseService {
/// Максимальное кол-во ошибок базы, после которых терминал блокирует свою работу
const int Maximum_DatabaseErrors = 13;
} // namespace CDatabaseService

//---------------------------------------------------------------------------
DatabaseService *DatabaseService::instance(IApplication *aApplication) {
    return static_cast<DatabaseService *>(
        aApplication->getCore()->getService(CServices::DatabaseService));
}

//---------------------------------------------------------------------------
DatabaseService::DatabaseService(IApplication *aApplication)
    : m_Application(aApplication), m_Database(0), m_ErrorCounter(0) {}

//---------------------------------------------------------------------------
DatabaseService::~DatabaseService() {
    if (m_Database) {
        IDatabaseProxy::freeInstance(m_Database);
        m_Database = 0;
    }
}

//---------------------------------------------------------------------------
bool DatabaseService::initialize() {
    LOG(m_Application->getLog(), LogLevel::Normal, "Initializing database...");

    PP::TerminalSettings *terminalSettings =
        SettingsService::instance(m_Application)->getAdapter<PP::TerminalSettings>();

    // Инициализация базы данных.
    m_Database = IDatabaseProxy::getInstance(this);
    if (!m_Database) {
        LOG(m_Application->getLog(), LogLevel::Error, "Can't get database proxy instance.");
        return false;
    }

    m_DbUtils = QSharedPointer<DatabaseUtils>(new DatabaseUtils(*m_Database, m_Application));

    SDK::PaymentProcessor::SDatabaseSettings dbSettings = terminalSettings->getDatabaseSettings();

    LOG(m_Application->getLog(),
        LogLevel::Normal,
        QString("Opening terminal database (host: %1, port: %2, name: %3)...")
            .arg(dbSettings.host)
            .arg(dbSettings.port)
            .arg(dbSettings.name));

    bool integrityFailed = false;
    QStringList errorsList;

    try {
        for (int retryCount = 0; retryCount < 2; ++retryCount) {
            // Подключаемся к БД.
            if (!m_Database->open(dbSettings.name,
                                  dbSettings.user,
                                  dbSettings.password,
                                  dbSettings.host,
                                  dbSettings.port)) {
                throw QString("cannot open database");
            }

            errorsList.clear();

            // Проверка на ошибку полностью испорченного формата базы
            integrityFailed = !m_Database->checkIntegrity(errorsList) ||
                              errorsList.filter(QRegularExpression("*malformed*")).size() ||
                              !m_DbUtils->initialize();

            if (integrityFailed) {
                LOG(m_Application->getLog(),
                    LogLevel::Error,
                    "Failed check database integrity. Backup broken database and create new.");

                // Закрываем БД и переименовываем файл базы
                QString databaseFile = m_Database->getCurrentBaseName();
                m_Database->close();
                QFile::rename(databaseFile,
                              databaseFile + QString(".backup_%1")
                                                 .arg(QDateTime::currentDateTime().toString(
                                                     "yyyyMMdd_hhmmss_zzz")));

                // Выставить ошибочный статус устройства "терминал"
                EventService::instance(m_Application)
                    ->sendEvent(
                        SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::Critical,
                                                     getName(),
                                                     "Database integrity check failed"));

                continue;
            }

            if (retryCount && !integrityFailed) {
                // Отмечаем статус устройства, что БД была восстановлена
                EventService::instance(m_Application)
                    ->sendEvent(SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::OK,
                                                             getName(),
                                                             "New database was created."));
            }

            // TODO - Запускаем процедуру восстановления базы
            //  http://blog.niklasottosson.com/?p=852
            break;
        }
    } catch (QString &error) {
        LOG(m_Application->getLog(),
            LogLevel::Error,
            QString("Failed to initialize database manager: %1.").arg(error));
        return false;
    }

    return !integrityFailed;
}

//------------------------------------------------------------------------------
void DatabaseService::finishInitialize() {}

//---------------------------------------------------------------------------
bool DatabaseService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool DatabaseService::shutdown() {
    m_DbUtils.clear();

    if (m_Database) {
        IDatabaseProxy::freeInstance(m_Database);
        m_Database = 0;
    }

    return true;
}

//---------------------------------------------------------------------------
QString DatabaseService::getName() const {
    return CServices::DatabaseService;
}

//---------------------------------------------------------------------------
const QSet<QString> &DatabaseService::getRequiredServices() const {
    static QSet<QString> requiredServices = QSet<QString>() << CServices::SettingsService
                                                            << CServices::EventService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap DatabaseService::getParameters() const {
    return QVariantMap();
}

//---------------------------------------------------------------------------
void DatabaseService::resetParameters(const QSet<QString> &) {}

/// Выполнение запроса по строке.
bool DatabaseService::execQuery(const QString &aQuery) {
    auto query = prepareQuery(aQuery);

    return execQuery(query);
}

//---------------------------------------------------------------------------
QSharedPointer<IDatabaseQuery> DatabaseService::prepareQuery(const QString &aQuery) {
    auto queryDeleter = [&](IDatabaseQuery *aQuery) { m_DbUtils->releaseQuery(aQuery); };

    return QSharedPointer<IDatabaseQuery>(m_DbUtils->prepareQuery(aQuery), queryDeleter);
}

//---------------------------------------------------------------------------
bool DatabaseService::execQuery(QSharedPointer<IDatabaseQuery> aQuery) {
    return m_DbUtils->execQuery(aQuery.data());
}

//---------------------------------------------------------------------------
QSharedPointer<IDatabaseQuery> DatabaseService::createAndExecQuery(const QString &aQuery) {
    auto query = prepareQuery(aQuery);

    if (execQuery(query)) {
        return query;
    }

    return QSharedPointer<IDatabaseQuery>();
}

//---------------------------------------------------------------------------
bool DatabaseService::isGood(bool aQueryResult) {
    if (!aQueryResult && ++m_ErrorCounter >= CDatabaseService::Maximum_DatabaseErrors) {
        const char message[] = "Database error counter has reached a limit value. Lock the "
                               "terminal due to a DB error.";
        LOG(m_Application->getLog(), LogLevel::Error, message);

        SDK::PaymentProcessor::Event e(SDK::PaymentProcessor::EEventType::TerminalLock,
                                       "DatabaseService");
        m_Application->getCore()->getEventService()->sendEvent(e);

        static bool feedbackSent = false;

        if (!feedbackSent) {
            m_Application->getCore()->getTerminalService()->sendFeedback(CServices::DatabaseService,
                                                                         message);

            feedbackSent = true;
        }
    }

    return aQueryResult;
}

//---------------------------------------------------------------------------