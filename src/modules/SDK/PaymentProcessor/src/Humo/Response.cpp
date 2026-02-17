/* @file Базовый ответ сервера. */

#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

#include <SDK/PaymentProcessor/Humo/Response.h>

#include <utility>

namespace SDK {
namespace PaymentProcessor {
namespace Humo {

//---------------------------------------------------------------------------
Response::Response(const Request &aRequest, QString aResponseString)
    : m_ResponseString(std::move(aResponseString)), m_Error(ELocalError::NetworkError),
      m_Result(EServerResult::Empty), m_Request(aRequest) {
    QRegularExpression regexp("^(\\w+)=(.*)$");

    foreach (auto line, m_ResponseString.split("\r\n")) {
        QRegularExpressionMatch match = regexp.match(line);
        if (match.hasMatch()) {
            if (match.capturedLength() > 1) {
                addParameter(match.captured(1), match.captured(2));
            }
        }
    }
}

//---------------------------------------------------------------------------
Response::~Response() = default;

//---------------------------------------------------------------------------
bool Response::isOk() {
    return ((m_Error == EServerError::Ok) && (m_Result == EServerResult::Ok));
}

//---------------------------------------------------------------------------
int Response::getError() const {
    return m_Error;
}

//---------------------------------------------------------------------------
int Response::getResult() const {
    return m_Result;
}

//---------------------------------------------------------------------------
const QString &Response::getErrorMessage() const {
    return m_ErrorMessage;
}

//---------------------------------------------------------------------------
QVariant Response::getParameter(const QString &aName) const {
    return m_Parameters.contains(aName) ? m_Parameters.value(aName) : QVariant();
}

//---------------------------------------------------------------------------
const QVariantMap &Response::getParameters() const {
    return m_Parameters;
}

//---------------------------------------------------------------------------
const QString &Response::toString() const {
    return m_ResponseString;
}

//---------------------------------------------------------------------------
QString Response::toLogString() const {
    QStringList result;

    for (auto it = getParameters().begin(); it != getParameters().end(); ++it) {
        result << QString("%1 = \"%2\"").arg(it.key()).arg(it.value().toString());
    }

    return result.join(", ");
}

//---------------------------------------------------------------------------
void Response::addParameter(const QString &aName, const QString &aValue) {
    m_Parameters[aName] = aValue;

    if (aName == CResponse::Parameters::Error) {
        m_Error = static_cast<EServerError::Enum>(aValue.toInt());
    } else if (aName == CResponse::Parameters::ErrorCode) {
        m_Error = static_cast<EServerError::Enum>(aValue.toInt());
    }

    if (aName == CResponse::Parameters::Result) {
        m_Result = static_cast<EServerResult::Enum>(aValue.toInt());
    }

    if (aName == CResponse::Parameters::ErrorMessage) {
        m_ErrorMessage = aValue;
    }
}

//---------------------------------------------------------------------------
const Request &Response::getRequest() const {
    return m_Request;
}

//------------------------------------------------------------------------------
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
// Definitions for CResponse::Parameters constants
namespace SDK {
namespace PaymentProcessor {
namespace Humo {
namespace CResponse {
namespace Parameters {
extern const char Result[] = "RESULT";
extern const char Error[] = "ERROR";
extern const char ErrorCode[] = "ERROR_CODE";
extern const char ErrorMessage[] = "ERRMSG";
extern const char ErrorMessage2[] = "ERROR_MSG";
} // namespace Parameters
} // namespace CResponse
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK
