
#include "DatabaseQuery.h"

#include <DatabaseProxy/IDatabaseProxy.h>

DatabaseQuery::DatabaseQuery(QSqlDatabase db, IDatabaseQueryChecker *aQueryChecker)
    : QSqlQuery(db), m_QueryChecker(aQueryChecker) {
    m_Log = ILog::getInstance(CIDatabaseQuery::DefaultLog);
}

//---------------------------------------------------------------------------
DatabaseQuery::~DatabaseQuery() {}

//---------------------------------------------------------------------------
bool DatabaseQuery::first() {
    return QSqlQuery::first();
}

//---------------------------------------------------------------------------
bool DatabaseQuery::next() {
    return QSqlQuery::next();
}

//---------------------------------------------------------------------------
bool DatabaseQuery::last() {
    return QSqlQuery::last();
}

//---------------------------------------------------------------------------
bool DatabaseQuery::isValid() {
    return QSqlQuery::isValid();
}

//---------------------------------------------------------------------------
int DatabaseQuery::numRowsAffected() const {
    return QSqlQuery::numRowsAffected();
}

//---------------------------------------------------------------------------
QVariant DatabaseQuery::value(int i) const {
    return QSqlQuery::value(i);
}

//---------------------------------------------------------------------------
bool DatabaseQuery::prepare(const QString &aQuery) {
    if (!QSqlQuery::prepare(aQuery)) {
        m_QueryChecker->isGood(false);

        LOG(m_Log,
            LogLevel::Error,
            QString("Can't prepare query: %1. Error: %2.")
                .arg(aQuery)
                .arg(QSqlQuery::lastError().databaseText()));

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void DatabaseQuery::bindValue(const QString &aName, const QVariant &aValue) {
    QSqlQuery::bindValue(aName, aValue.isNull() ? QString("") : aValue);
}

//---------------------------------------------------------------------------
void DatabaseQuery::bindValue(int aPos, const QVariant &aValue) {
    QSqlQuery::bindValue(aPos, aValue.isNull() ? QString("") : aValue);
}

//---------------------------------------------------------------------------
bool DatabaseQuery::exec() {
    if (!QSqlQuery::exec()) {
        m_QueryChecker->isGood(false);

        LOG(m_Log,
            LogLevel::Error,
            QString("Can't execute query: %1. Error: %2.")
                .arg(QSqlQuery::lastQuery())
                .arg(QSqlQuery::lastError().databaseText()));

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void DatabaseQuery::clear() {
    QSqlQuery::clear();
}

//---------------------------------------------------------------------------
