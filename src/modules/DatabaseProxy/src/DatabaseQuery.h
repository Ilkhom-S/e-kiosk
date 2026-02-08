#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtSql/QtSql>

#include <Common/ILog.h>

#include <DatabaseProxy/IDatabaseQuery.h>

class IDatabaseQueryChecker;

//---------------------------------------------------------------------------
class DatabaseQuery : public IDatabaseQuery, public QSqlQuery {
public:
    DatabaseQuery(QSqlDatabase db, IDatabaseQueryChecker *aQueryChecker);
    virtual ~DatabaseQuery();

    virtual bool prepare(const QString &aQuery) override;
    virtual void bindValue(const QString &aName, const QVariant &aValue) override;
    virtual void bindValue(int aPos, const QVariant &aValue) override;
    virtual bool exec() override;
    virtual void clear() override;

    virtual bool first() override;
    virtual bool next() override;
    virtual bool last() override;

    virtual bool isValid() override;
    virtual int numRowsAffected() const override;

    virtual QVariant value(int i) const override;

private:
    ILog *m_Log;
    IDatabaseQueryChecker *m_QueryChecker;
};

//---------------------------------------------------------------------------
