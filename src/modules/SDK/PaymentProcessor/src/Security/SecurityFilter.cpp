/* @file Класс-фильтр содержимого полей ввода пользователя. */

#include "SecurityFilter.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
SecurityFilter::SecurityFilter(const SProvider &aProvider,
                               SProviderField::SecuritySubsystem aSubsystem)
    : m_Provider(aProvider), m_Subsystem(aSubsystem) {}

//------------------------------------------------------------------------------
bool SecurityFilter::haveFilter(const QString &aParameterName) const {
    QRegularExpression regExp = getMask(aParameterName);

    return !regExp.pattern().isEmpty() && regExp.isValid();
}

//------------------------------------------------------------------------------
QString SecurityFilter::apply(const QString &aParameterName, const QString &aValue) const {
    QRegularExpression regExp = getMask(aParameterName);

    if (!regExp.pattern().isEmpty() && regExp.isValid()) {
        QRegularExpressionMatch match = regExp.match(aValue);
        if (match.hasMatch()) {
            QString value = aValue;

            QStringList capturedTexts = match.capturedTexts();
            for (int i = 1; i < capturedTexts.size(); i++) {
                const QString &captured = capturedTexts[i];
                if (!captured.isEmpty()) {
                    int pos = match.capturedStart(i);
                    value.replace(pos, captured.size(), QString("*").repeated(captured.size()));
                }
            }

            return value;
        }
        qDebug() << QString("RegExp '%1' not found value in: %2").arg(regExp.pattern()).arg(aValue);
    }

    return aValue;
}

//------------------------------------------------------------------------------
QRegularExpression SecurityFilter::getMask(const QString &aParameterName) const {
    foreach (auto field, m_Provider.fields) {
        if (aParameterName.contains(field.id, Qt::CaseInsensitive)) {
            auto subsystem =
                field.security.contains(m_Subsystem) ? m_Subsystem : SProviderField::Default;
            QString regExp = field.security.value(subsystem, QString());

            if (!regExp.isEmpty()) {
                QRegularExpression rx(regExp);

                if (rx.isValid()) {
                    return rx;
                }

                qDebug() << QString("RegExp '%1' not valid: %2.").arg(regExp).arg(rx.errorString());
            }
        }
    }

    return {};
}

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
