/* @file Абстрактный запрос к серверу. */

#include "Request.h"

#include <QtCore/QStringList>

namespace SDK {
namespace PaymentProcessor {
namespace Humo {

//---------------------------------------------------------------------------
Request::Request() : m_IsOk(true), m_IsCriticalError(false) {}

//---------------------------------------------------------------------------
Request::~Request() {}

//---------------------------------------------------------------------------
bool Request::isOk() const {
    return m_IsOk;
}

//---------------------------------------------------------------------------
bool Request::isCriticalError() const {
    return m_IsCriticalError;
}

//---------------------------------------------------------------------------
void Request::addParameter(const QString &aName,
                           const QVariant &aValue,
                           const QVariant &aLogValue /*= QVariant()*/) {
    m_Parameters[aName] = aValue;

    if (!aLogValue.isNull()) {
        m_LogParameters[aName] = aLogValue;
    }
}

//---------------------------------------------------------------------------
void Request::addRawParameter(const QString &aName,
                              const QVariant &aValue,
                              const QVariant &aLogValue /*= QVariant()*/) {
    m_RawParameters[aName] = aValue;

    if (!aLogValue.isNull()) {
        m_RawLogParameters[aName] = aLogValue;
    }
}

//---------------------------------------------------------------------------
void Request::removeParameter(const QString &aName, bool aRAWParameter /*= false*/) {
    if (aRAWParameter) {
        m_RawParameters.remove(aName);
    } else {
        m_Parameters.remove(aName);
    }
}

//---------------------------------------------------------------------------
QVariant Request::getParameter(const QString &aName, bool aRAWParameter /*= false*/) const {
    return aRAWParameter ? m_RawParameters.value(aName, QVariant())
                         : m_Parameters.value(aName, QVariant());
}

//---------------------------------------------------------------------------
const QVariantMap &Request::getParameters(bool aRAWParameter /*= false*/) const {
    return aRAWParameter ? m_RawParameters : m_Parameters;
}

//---------------------------------------------------------------------------
void Request::clear() {
    m_Parameters.clear();
    m_LogParameters.clear();
    m_RawParameters.clear();
    m_RawLogParameters.clear();
}

//---------------------------------------------------------------------------
QString Request::toString() const {
    QString result;

    for (auto it = m_Parameters.begin(); it != m_Parameters.end(); ++it) {
        result += QString("%1=%2\r\n").arg(it.key()).arg(it.value().toString());
    }

    return result;
}

//---------------------------------------------------------------------------
QString Request::toLogString() const {
    QStringList result;

    for (auto it = m_Parameters.begin(); it != m_Parameters.end(); ++it) {
        result << QString("%1 = \"%2\"")
                      .arg(it.key())
                      .arg(m_LogParameters.contains(it.key())
                               ? m_LogParameters.value(it.key()).toString()
                               : it.value().toString());
    }

    for (auto it = m_RawParameters.begin(); it != m_RawParameters.end(); ++it) {
        result << QString("RAW(%1) = \"%2\"")
                      .arg(it.key())
                      .arg(m_RawLogParameters.contains(it.key())
                               ? m_RawLogParameters.value(it.key()).toString()
                               : it.value().toString());
    }

    return result.join(", ");
}

//------------------------------------------------------------------------------
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK
