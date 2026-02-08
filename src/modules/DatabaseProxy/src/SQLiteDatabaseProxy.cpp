
#include "SQLiteDatabaseProxy.h"

#include <QtCore/QDir>
#include <QtCore/QMutexLocker>

#include <Common/BasicApplication.h>
#include <Common/SleepHelper.h>

#include <memory>

#include "DatabaseQuery.h"

namespace CSQLiteDatabaseProxy {
const int MaxTryCount = 3;
} // namespace CSQLiteDatabaseProxy

//---------------------------------------------------------------------------
SQLiteDatabaseProxy::SQLiteDatabaseProxy()
    : ILogable(CIDatabaseProxy::LogName), , m_QueryChecker(nullptr) {}

//---------------------------------------------------------------------------
SQLiteDatabaseProxy::~SQLiteDatabaseProxy() = default;

//---------------------------------------------------------------------------
void SQLiteDatabaseProxy::setQueryChecker(IDatabaseQueryChecker *aQueryChecker) {
    m_QueryChecker = aQueryChecker;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::open(const QString &aDbName,
                               const QString &aUser,
                               const QString &aPassword,
                               const QString &aHost,
                               const int /*aPort*/) {
    m_CurrentBase = aDbName;

    if (QDir::isRelativePath(m_CurrentBase)) {
        m_CurrentBase = QDir::cleanPath(BasicApplication::getInstance()->getWorkingDirectory() +
                                        QDir::separator() + m_CurrentBase);
    }

    if (m_Db && m_Db->isOpen()) {
        toLog(LogLevel::Normal, "Before open new database, current database must be closed.");

        close();
    }

    m_Db = QSharedPointer<QSqlDatabase>(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")));

    m_Db->setDatabaseName(m_CurrentBase);
    m_Db->setUserName(aUser);
    m_Db->setPassword(aPassword);
    m_Db->setHostName(aHost);
    m_Db->setConnectOptions(
        QString("QSQLITE_BUSY_TIMEOUT=%1").arg(CSQLiteDatabaseProxy::BusyTimeout));

    if (m_Db->open()) {
        toLog(LogLevel::Normal, QString("Database opened: %1.").arg(m_CurrentBase));

        return true;
    }
    toLog(LogLevel::Error,
          QString("Cannot open database %1. Following error occured: %2.")
              .arg(m_CurrentBase)
              .arg(m_Db->lastError().driverText()));

    return false;
}

//---------------------------------------------------------------------------
void SQLiteDatabaseProxy::close() {
    if (isConnected()) {
        m_Db->close();
    }

    m_Db.clear();
    m_CurrentBase.clear();

    toLog(LogLevel::Normal, QString("Database has been closed: %1.").arg(m_CurrentBase));
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::isConnected() const {
    return (m_Db && m_Db->isOpen());
}

//---------------------------------------------------------------------------
const QString &SQLiteDatabaseProxy::getCurrentBaseName() const {
    return m_CurrentBase;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::safeExec(QSqlQuery *aQuery, const QString &aQueryMessage) {
    QMutexLocker locker(&m_Mutex);

    int tryCount = 0;

    while (tryCount < CSQLiteDatabaseProxy::MaxTryCount) {
        if (!aQuery->exec(aQueryMessage)) {
            m_QueryChecker->isGood(!aQuery->lastError().isValid() ||
                                   aQuery->lastError().type() == QSqlError::NoError);

            toLog(LogLevel::Error,
                  QString("Can't execute query: %1. Error: %2.")
                      .arg(aQueryMessage)
                      .arg(aQuery->lastError().text()));

            if (aQuery->lastError().text().contains("disk I/O error")) {
                toLog(LogLevel::Normal, "Trying to recover after database disk I/O error.");

                SleepHelper::msleep(100);
            } else {
                return false;
            }
        } else {
            break;
        }

        ++tryCount;
    }

    return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::execDML(const QString &aQuery, long &aRowsAffected) {
    QMutexLocker locker(&m_Mutex);

    if (!isConnected()) {
        toLog(LogLevel::Error,
              QString("Cannot execute sql query in unconnected state. Query: %1.").arg(aQuery));

        return false;
    }

    QSqlQuery dbQuery(*m_Db);

    if (safeExec(&dbQuery, aQuery)) {
        aRowsAffected = dbQuery.num_RowsAffected();

        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::execScalar(const QString &aQuery, long &aResult) {
    QMutexLocker locker(&m_Mutex);

    if (!isConnected()) {
        toLog(LogLevel::Error,
              QString("Cannot execute sql query in unconnected state. Query: %1.").arg(aQuery));

        return false;
    }

    QSqlQuery dbQuery(*m_Db);

    if (safeExec(&dbQuery, aQuery) && dbQuery.first() && (dbQuery.record().count() != 0)) {
        aResult = static_cast<long>(dbQuery.value(0).toLongLong());

        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
IDatabaseQuery *SQLiteDatabaseProxy::execQuery(const QString &aQuery) {
    QMutexLocker locker(&m_Mutex);

    if (!isConnected()) {
        toLog(LogLevel::Error,
              QString("Cannot execute sql query in unconnected state. Query: %1.").arg(aQuery));

        return nullptr;
    }

    std::unique_ptr<IDatabaseQuery> dbQuery(createQuery());

    auto *dbQtQuery = dynamic_cast<QSqlQuery *>(dbQuery.get());

    if (safeExec(dbQtQuery, aQuery)) {
        return dbQuery.release();
    }

    return nullptr;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::transaction() {
    if (!isConnected()) {
        toLog(LogLevel::Error, "Cannot start transaction in unconnected state.");

        return false;
    }

    if (!m_QueryChecker->isGood(m_Db->transaction())) {
        toLog(LogLevel::Error,
              QString("Cannot start transaction. Error: %1.").arg(m_Db->lastError().text()));

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::commit() {
    if (!isConnected()) {
        toLog(LogLevel::Error, "Cannot commit transaction in unconnected state.");

        return false;
    }

    if (!m_QueryChecker->isGood(m_Db->commit())) {
        toLog(LogLevel::Error,
              QString("Cannot commit transaction. Error: %1.").arg(m_Db->lastError().text()));

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::rollback() {
    if (!isConnected()) {
        toLog(LogLevel::Error, "Cannot rollback transaction in unconnected state.");

        return false;
    }

    if (!m_QueryChecker->isGood(m_Db->rollback())) {
        toLog(LogLevel::Error,
              QString("Cannot rollback transaction. Error: %1.").arg(m_Db->lastError().text()));

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
IDatabaseQuery *SQLiteDatabaseProxy::createQuery() {
    if (!isConnected()) {
        toLog(LogLevel::Error, "Cannot create sql query in unconnected state.");

        return nullptr;
    }

    return new DatabaseQuery(*m_Db, m_QueryChecker);
}

//---------------------------------------------------------------------------
IDatabaseQuery *SQLiteDatabaseProxy::createQuery(const QString &aQueryString) {
    IDatabaseQuery *query = createQuery();

    if (!m_QueryChecker->isGood(query->prepare(aQueryString))) {
        delete query;
        query = nullptr;
    }

    return query;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::checkIntegrity(QStringList &aListErrors) {
    QScopedPointer<IDatabaseQuery> query(createQuery());

    if (!query->prepare("PRAGMA integrity_check")) {
        aListErrors << dynamic_cast<DatabaseQuery *>(query.data())->lastError().databaseText();

        toLog(LogLevel::Error, "Failed to create query for integrity check.");

        return false;
    }

    if (!m_QueryChecker->isGood(query->exec())) {
        aListErrors << dynamic_cast<DatabaseQuery *>(query.data())->lastError().databaseText();

        toLog(LogLevel::Error, "Failed exec integrity check.");

        return false;
    }

    bool result = true;

    for (query->first(); query->isValid(); query->next()) {
        QString message = query->value(0).toString();
        if (message.compare("ok", Qt::CaseInsensitive) != 0) {
            aListErrors << message;

            toLog(LogLevel::Error, QString("DATABASE INTEGRITY FAILED: %1.").arg(message));

            result = false;
        }
    }

    return result;
}

//---------------------------------------------------------------------------
