/* @file Интерфейс сервиса для работы с БД. */

#pragma once

#include <QtCore/QSharedPointer>

class IDatabaseQuery;

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IDatabaseService {
public:
    /// Выполнение запроса по строке.
    virtual bool execQuery(const QString &aQuery) = 0;

    /// Подготавливает запрос к привязке параметров.
    virtual QSharedPointer<IDatabaseQuery> prepareQuery(const QString &aQuery) = 0;

    /// Создание запроса по строке и его выполнение.
    virtual QSharedPointer<IDatabaseQuery> createAndExecQuery(const QString &aQuery) = 0;

    /// Выполнение переданного запроса.
    virtual bool execQuery(QSharedPointer<IDatabaseQuery> aQuery) = 0;

protected:
    virtual ~IDatabaseService() {}
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
