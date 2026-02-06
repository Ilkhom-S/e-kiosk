/* @file Базовый интерфейс работы с БД. */

#pragma once

#include <QtCore/QString>

class IDatabaseQuery;

//---------------------------------------------------------------------------
class IDatabaseUtils {
public:
    virtual bool initialize() = 0;

    /// Подготавливает к выполнению запрос.
    virtual IDatabaseQuery *prepareQuery(const QString &aQuery) = 0;

    /// Thread защищённое выполнение произвольного запроса.
    virtual bool execQuery(IDatabaseQuery *aQuery) = 0;

    /// Освобождение памяти из-под запроса.
    virtual void releaseQuery(IDatabaseQuery *aQuery) = 0;

    // protected:
    virtual ~IDatabaseUtils() {};
};

//---------------------------------------------------------------------------
