/* @file Реализация интерфейсов для работы с БД. */

#include "DatabaseUtils.h"

#include <QtCore/QFile>
#include <QtCore/QMutexLocker>
#include <QtCore/QRecursiveMutex>
#include <QtCore/QRegularExpression>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>

#include <Common/ExceptionFilter.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>
#include <memory>

#include "System/IApplication.h"

namespace CDatabaseUtils {
const QString EmptyDatabaseScript = ":/scripts/empty_db.sql";
const QString DatabasePatchParam = "db_patch";
const QString DatabaseUpdateParam = "db_update";

/// Миграционные патчи базы данных.
///
/// empty_db.sql содержит базовую схему базы данных.
/// Для добавления новой миграции:
/// 1. Создайте файл scripts/db_patch_1.sql с SQL инструкциями для миграции
/// 2. Добавьте запись в массив ниже:
///    {1, ":/scripts/db_patch_1.sql"},
/// 3. Добавьте ресурс в Database.qrc (<file>scripts/db_patch_1.sql</file>)
/// 4. Миграция будет автоматически применена при обновлении базы
const struct {
    int version;
    QString script;
} Patches[] = {
    // Миграции добавляются здесь по мере необходимости
};

} // namespace CDatabaseUtils

//---------------------------------------------------------------------------
DatabaseUtils::DatabaseUtils(IDatabaseProxy &aProxy, IApplication *aApplication)
    : m_Database(aProxy), m_Application(aApplication), m_Log(aApplication->getLog()),
      m_PaymentLog(ILog::getInstance("Payments")) {}

//---------------------------------------------------------------------------
DatabaseUtils::~DatabaseUtils() = default;

//---------------------------------------------------------------------------
bool DatabaseUtils::initialize() {
    try {
        if (!m_Database.isConnected()) {
            throw std::runtime_error("not connected.");
        }

        if (!m_Database.transaction()) {
            throw std::runtime_error("Cannot start database transaction.");
        }

        // Проверяем, созданы ли таблицы. Если нет, применяем схему из empty_db.sql
        if (databaseTableCount() == 0) {
            LOG(m_Log, LogLevel::Normal, "Creating database schema from empty_db.sql.");
            updateDatabase(CDatabaseUtils::EmptyDatabaseScript);
        }

        // Применяем миграции (если они добавлены в Patches[])
        for (const auto &patch : CDatabaseUtils::Patches) {
            if (databasePatch() < patch.version) {
                LOG(m_Log,
                    LogLevel::Normal,
                    QString("Applying database migration to version %1.").arg(patch.version));
                updateDatabase(patch.script);
            }
        }
    } catch (...) {
        EXCEPTION_FILTER(m_Log);

        m_Database.rollback();

        return false;
    }

    if (!m_Database.commit()) {
        LOG(m_Log, LogLevel::Error, "Failed to commit changes to the terminal database.");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::updateDatabase(const QString &aSqlScriptName) {
    QFile ftemp(aSqlScriptName);
    if (!ftemp.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Cannot open database script from resources.");
    }

    QString resSQL = ftemp.readAll();

    QRegularExpression rx(R"((\/\*.*\*\/|\-\-.*\n))");
    resSQL.replace(rx, "");

    // Удалим перевод строк с сохранением возможности писать ("CREATE\nTABLE")
    resSQL.replace("\r", "");
    resSQL.replace("\n", " ");

    QStringList creatingSteps = resSQL.split(";");

    long rowsAffected(0);

    foreach (const QString &step, creatingSteps) {
        if (step.trimmed().isEmpty()) {
            continue;
        }

        if (!m_Database.execDML(step, rowsAffected)) {
            throw std::runtime_error("Cannot execute regular sql query.");
        }
    }

    return true;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databaseTableCount() const {
    QString queryMessage = "SELECT count(*) FROM sqlite_master WHERE type = 'table'";
    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.execQuery(queryMessage));
    if (!dbQuery) {
        LOG(m_Log, LogLevel::Error, "Failed to check database tables presence.");
        return 0;
    }

    if (dbQuery->first()) {
        return dbQuery->value(0).toInt();
    }

    return 0;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databasePatch() const {
    QString queryMessage = "SELECT `value` FROM device_param WHERE `name` = 'db_patch'";
    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.execQuery(queryMessage));
    if (!dbQuery) {
        LOG(m_Log, LogLevel::Error, "Failed to check database patch version.");
        return 0;
    }

    if (dbQuery->first()) {
        return dbQuery->value(0).toInt();
    }

    return 0;
}

//---------------------------------------------------------------------------
IDatabaseQuery *DatabaseUtils::prepareQuery(const QString &aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    std::unique_ptr<IDatabaseQuery> query(m_Database.createQuery(aQuery));

    return query ? query.release() : nullptr;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::execQuery(IDatabaseQuery *aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    if (aQuery) {
        return aQuery->exec();
    }

    return false;
}

//---------------------------------------------------------------------------
void DatabaseUtils::releaseQuery(IDatabaseQuery *aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    delete aQuery;
}

//---------------------------------------------------------------------------
