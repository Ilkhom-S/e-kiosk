#pragma once

#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtSql/QtSql>

#include <Common/ILog.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

class QSqlDatabase;
class IPayment;

//---------------------------------------------------------------------------
namespace CMySqlDatabaseProxy {
const QString DriverName = "QMYSQL";
const QString DefaultLog = "MySql";
const QString DateFormat = "yyyy-MM-dd hh:mm:ss";
const QString DefaultUser = "root";
const QString DefaultPass = "";
const QString DefaultHost = "localhost";
const int DefaultPort = 3306;
const QString ConnectionName = "DefaultConnection";
} // namespace CMySqlDatabaseProxy

//---------------------------------------------------------------------------
class MySqlDatabaseProxy : public IDatabaseProxy {
    friend class IDatabaseProxy;

public:
    MySqlDatabaseProxy();
    virtual ~MySqlDatabaseProxy() override;

    /// Установить интерфейс контроля над ошибками БД
    virtual void setQueryChecker(IDatabaseQueryChecker *aQueryChecker) override;

    virtual bool open(const QString &dbName,
                      const QString &aUser = CMySqlDatabaseProxy::DefaultUser,
                      const QString &aPassword = CMySqlDatabaseProxy::DefaultPass,
                      const QString &aHost = CMySqlDatabaseProxy::DefaultHost,
                      const int aPort = CMySqlDatabaseProxy::DefaultPort) override;
    virtual void close() override;

    virtual bool isConnected() const override;
    virtual const QString &getCurrentBaseName() const override;

    // Создаёт экземпляр запроса к БД.
    virtual IDatabaseQuery *createQuery() override;

    /// Создает и подготавливает экземпляр запроса к БД.
    virtual IDatabaseQuery *createQuery(const QString &aQueryString) override;

    /*!< Выполнение DML запроса. Помещает в rowsAffected количество затронутых строк. */
    virtual bool execDML(const QString &strQuery, long &rowsAffected) override;
    /*!< Выполнение запроса, содержащего, к примеру, COUNT(*). В result записывает значение ячейки
     * (1,1). */
    virtual bool execScalar(const QString &strQuery, long &result) override;
    /*!< Выполнение произвольного запроса. Если запрос успешно выполнен, то результат функции будет
     * не нулевым. */
    virtual IDatabaseQuery *execQuery(const QString &strQuery) override;

    /*!< Пытается создать новую транзакцию. */
    virtual bool transaction() override;
    /*!< Принимает изменения, внесённые во время последней транзакции. */
    virtual bool commit() override;
    /*!< Сбрасывает изменения, внесённые во время последней транзакции. */
    virtual bool rollback() override;

public:
    /// Проверка целостности базы
    virtual bool checkIntegrity(QStringList &aListErrors) override;

protected:
    virtual bool safeExec(QSqlQuery *query, const QString &queryMessage);

private:
    QRecursiveMutex m_Mutex;
    QSqlDatabase *m_Db;
    QString m_CurrentBase;
    ILog *m_Log;
    IDatabaseQueryChecker *m_QueryChecker;
};

//---------------------------------------------------------------------------
