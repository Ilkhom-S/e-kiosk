#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtSql/QtSql>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// System
#include <DatabaseProxy/IDatabaseQuery.h>

class IDatabaseQueryChecker;

//---------------------------------------------------------------------------
class DatabaseQuery : public IDatabaseQuery, public QSqlQuery
{
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
    ILog *m_log;
    IDatabaseQueryChecker *mQueryChecker;
};

//---------------------------------------------------------------------------
