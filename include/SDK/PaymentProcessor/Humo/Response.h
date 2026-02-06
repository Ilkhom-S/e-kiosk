/* @file Базовый ответ сервера. */

#pragma once

#include <QtCore/QVariantMap>

#include "ErrorCodes.h"

namespace SDK {
namespace PaymentProcessor {
namespace Humo {

//---------------------------------------------------------------------------
namespace CResponse {
namespace Parameters {
const char Result[] = "RESULT";
const char Error[] = "ERROR";
const char ErrorCode[] = "ERROR_CODE";
const char ErrorMessage[] = "ERRMSG";
const char ErrorMessage2[] = "ERROR_MSG";
} // namespace Parameters
} // namespace CResponse

//---------------------------------------------------------------------------
class Request;

//---------------------------------------------------------------------------
class Response {
    Q_DISABLE_COPY(Response)

public:
    Response(const Request &aRequest, const QString &aResponseString);
    virtual ~Response();

    /// Предикат истинен, если ответ сервера не содержит ошибок.
    virtual bool isOk();

    /// Возвращает значения поля "ошибка".
    virtual int getError() const;

    /// Возвращает значение поля "результат".
    virtual int getResult() const;

    /// Возвращает текст ошибки, если он есть.
    virtual const QString &getErrorMessage() const;

    /// Возвращает указанный параметр. Если отсутствует - возвращает пустой QVariant.
    virtual QVariant getParameter(const QString &aName) const;

    /// Возвращает полный список параметров.
    virtual const QVariantMap &getParameters() const;

    /// Возвращает строку, из которой был сконструирован объект.
    virtual const QString &toString() const;

    /// Возвращает содержимое запроса в пригодном для логирования виде.
    virtual QString toLogString() const;

protected:
    /// Добавление параметра.
    void addParameter(const QString &aName, const QString &aValue);

    /// Возвращает запрос, связанный с ответом.
    const Request &getRequest() const;

private:
    /// Строка, из которой был создан класс.
    QString mResponseString;

    /// Параметры ответа.
    QVariantMap mParameters;

    /// Значения полей "ошибка" и "результат".
    int mError;
    int mResult;

    /// Текст ошибки сервера.
    QString mErrorMessage;

    const Request &mRequest;
};

//------------------------------------------------------------------------------
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK
