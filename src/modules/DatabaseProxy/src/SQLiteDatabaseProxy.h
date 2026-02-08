#pragma once

#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtSql/QtSql>

#include <Common/ILogable.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

class QSqlDatabase;

//---------------------------------------------------------------------------
namespace CSQLiteDatabaseProxy {
const QString DriverName = "QSQLITE";
const int BusyTimeout = 5000; // ms
} // namespace CSQLiteDatabaseProxy

//---------------------------------------------------------------------------
class SQLiteDatabaseProxy : public IDatabaseProxy, protected ILogable {
    friend class IDatabaseProxy;

public:
    SQLiteDatabaseProxy();

    virtual ~SQLiteDatabaseProxy() override;

    /// Установить интерфейс контроля над ошибками БД
    virtual void setQueryChecker(IDatabaseQueryChecker *aQueryChecker) override;

    /// IDatabaseProxy: Открытие соединения с БД.
    virtual bool open(const QString &aDbName = CIDatabaseProxy::DefaultDatabase,
                      const QString &aUser = CIDatabaseProxy::DefaultUser,
                      const QString &aPassword = CIDatabaseProxy::DefaultPassword,
                      const QString &aHost = CIDatabaseProxy::DefaultHost,
                      const int aPort = CIDatabaseProxy::DefaultPort) override;

    /// IDatabaseProxy: Закрытие соединения с БД.
    virtual void close() override;

    /// IDatabaseProxy: Возвращает true, если база открыта.
    virtual bool isConnected() const override;

    /// IDatabaseProxy: Возвращает имя открытой БД.
    virtual const QString &getCurrentBaseName() const override;

    /// IDatabaseProxy: Создаёт экземпляр запроса к БД.
    virtual IDatabaseQuery *createQuery() override;

    /// Создает и подготавливает экземпляр запроса к БД.
    virtual IDatabaseQuery *createQuery(const QString &aQueryString) override;

    /// IDatabaseProxy: Выполнение DML запроса. Помещает в rowsAffected количество затронутых строк.
    virtual bool execDML(const QString &aQuery, long &aRowsAffected) override;

    /// IDatabaseProxy: Выполнение запроса, содержащего, к примеру, COUNT(*). В result записывает
    /// значение ячейки (1,1).
    virtual bool execScalar(const QString &aQuery, long &aResult) override;

    /// IDatabaseProxy: Выполнение произвольного запроса. Если запрос успешно выполнен, то результат
    /// функции будет не нулевым.
    virtual IDatabaseQuery *execQuery(const QString &aQuery) override;

    /// IDatabaseProxy: Пытается создать новую транзакцию.
    virtual bool transaction() override;

    /// IDatabaseProxy: Принимает изменения, внесённые во время последней транзакции.
    virtual bool commit() override;

    /// IDatabaseProxy: Сбрасывает изменения, внесённые во время последней транзакции.
    virtual bool rollback() override;

public:
    /// Проверка целостности базы
    virtual bool checkIntegrity(QStringList &aListErrors) override;

protected:
    virtual bool safeExec(QSqlQuery *aQuery, const QString &aQueryMessage);

private:
    QSharedPointer<QSqlDatabase> m_Db;
    QRecursiveMutex m_Mutex;
    QString m_CurrentBase;
    IDatabaseQueryChecker *m_QueryChecker;
};

//---------------------------------------------------------------------------
