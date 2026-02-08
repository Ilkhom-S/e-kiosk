#include "MySqlDatabaseProxy.h"

#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>
#include <QtCore/QVariant>

#include <Common/ExceptionFilter.h>
#include <Common/ILog.h>

#include <memory>

#include "DatabaseQuery.h"

MySqlDatabaseProxy::MySqlDatabaseProxy()
    : m_Mutex(), m_Db(nullptr), m_Log(ILog::getInstance(CMySqlDatabaseProxy::DefaultLog)),
      m_QueryChecker(nullptr) {}

//---------------------------------------------------------------------------
MySqlDatabaseProxy::~MySqlDatabaseProxy() {
    delete m_Db;
    m_Db = nullptr;
}

//---------------------------------------------------------------------------
void MySqlDatabaseProxy::setQueryChecker(IDatabaseQueryChecker *aQueryChecker) {
    m_QueryChecker = aQueryChecker;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::open(const QString &dbName,
                              const QString &user,
                              const QString &password,
                              const QString &host,
                              const int aPort) {
    try {
        m_Db = new QSqlDatabase(QSqlDatabase::addDatabase(CMySqlDatabaseProxy::DriverName,
                                                          CMySqlDatabaseProxy::ConnectionName));

        m_CurrentBase = dbName;

        m_Db->setDatabaseName(dbName);
        m_Db->setUserName(user);
        m_Db->setPassword(password);
        m_Db->setHostName(host);
        m_Db->setPort(aPort);

        bool openResult = m_Db->open();

        if (openResult) {
            LOG(m_Log, LogLevel::Normal, QString("Database has been opened: %1.").arg(dbName));
            return true;
        }
        LOG(m_Log,
            LogLevel::Error,
            QString("Following error occured: %1.").arg(m_Db->lastError().driverText()));

    } catch (...) {
        EXCEPTION_FILTER(m_Log);
    }

    LOG(m_Log, LogLevel::Fatal, QString("Can't open database: %1.").arg(dbName));

    return false;
}

//---------------------------------------------------------------------------
void MySqlDatabaseProxy::close() {
    if (!m_Db) {
        return;
    }

    if (m_Db->isOpen()) {
        m_Db->close();
    }

    delete m_Db;
    m_Db = nullptr;

    QSqlDatabase::removeDatabase(CMySqlDatabaseProxy::ConnectionName);

    LOG(m_Log, LogLevel::Normal, QString("Database has been closed: %1.").arg(m_CurrentBase));
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::isConnected() const {
    return m_Db->isOpen();
}

//---------------------------------------------------------------------------
const QString &MySqlDatabaseProxy::getCurrentBaseName() const {
    return m_CurrentBase;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::safeExec(QSqlQuery *query, const QString &queryMessage) {
    QMutexLocker locker(&m_Mutex);

    try {
        if (!m_QueryChecker->isGood(query->exec(queryMessage))) {
            LOG(m_Log,
                LogLevel::Error,
                QString("Can't execute query: %1. Error: %2.")
                    .arg(queryMessage)
                    .arg(query->lastError().text()));

            return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::execDML(const QString &strQuery, long &rowsAffected) {
    if (!m_Db) {
        return false;
    }

    QSqlQuery dbQuery(*m_Db);

    bool execResult = safeExec(&dbQuery, strQuery);

    rowsAffected = dbQuery.numRowsAffected();

    return execResult;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::execScalar(const QString &strQuery, long &result) {
    if (!m_Db) {
        return false;
    }

    QSqlQuery dbQuery(*m_Db);

    result = 0;

    bool execResult = safeExec(&dbQuery, strQuery);

    if (dbQuery.first()) {
        result = static_cast<long>(dbQuery.value(0).toLongLong());
    }

    return execResult;
}

//---------------------------------------------------------------------------
IDatabaseQuery *MySqlDatabaseProxy::execQuery(const QString &strQuery) {
    if (!m_Db) {
        return nullptr;
    }

    std::unique_ptr<IDatabaseQuery> dbQuery(new DatabaseQuery(*m_Db, m_QueryChecker));

    auto *dbQtQuery = dynamic_cast<QSqlQuery *>(dbQuery.get());

    return safeExec(dbQtQuery, strQuery) ? dbQuery.release() : nullptr;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::transaction() {
    if (!m_Db) {
        return false;
    }

    QMutexLocker locker(&m_Mutex);

    return m_Db->transaction();
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::commit() {
    if (!m_Db) {
        return false;
    }

    QMutexLocker locker(&m_Mutex);

    return m_Db->commit();
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::rollback() {
    if (!m_Db) {
        return false;
    }

    QMutexLocker locker(&m_Mutex);

    return m_Db->rollback();
}

//---------------------------------------------------------------------------
IDatabaseQuery *MySqlDatabaseProxy::createQuery() {
    if (!m_Db) {
        throw std::runtime_error("cannot create query without valid database");
    }

    return new DatabaseQuery(*m_Db, m_QueryChecker);
}

//---------------------------------------------------------------------------
IDatabaseQuery *MySqlDatabaseProxy::createQuery(const QString &aQueryString) {
    IDatabaseQuery *query = createQuery();

    if (!m_QueryChecker->isGood(query->prepare(aQueryString))) {
        delete query;
        query = nullptr;
    }

    return query;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::checkIntegrity(QStringList &aListErrors) {
    Q_UNUSED(aListErrors)

    return true;
}

//---------------------------------------------------------------------------
