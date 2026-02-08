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
extern const char Result[];
extern const char Error[];
extern const char ErrorCode[];
extern const char ErrorMessage[];
extern const char ErrorMessage2[];
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
    QString m_ResponseString;

    /// Параметры ответа.
    QVariantMap m_Parameters;

    /// Значения полей "ошибка" и "результат".
    int m_Error;
    int m_Result;

    /// Текст ошибки сервера.
    QString m_ErrorMessage;

    const Request &m_Request;
};

//------------------------------------------------------------------------------
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK
