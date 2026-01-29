/* @file Класс-фильтр содержимого полей ввода пользователя. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "SecurityFilter.h"

namespace SDK
{
    namespace PaymentProcessor
    {

        //------------------------------------------------------------------------------
        SecurityFilter::SecurityFilter(const SProvider &aProvider, SProviderField::SecuritySubsystem aSubsystem)
            : mProvider(aProvider), mSubsystem(aSubsystem)
        {
        }

        //------------------------------------------------------------------------------
        bool SecurityFilter::haveFilter(const QString &aParameterName) const
        {
            QRegularExpression regExp = getMask(aParameterName);

            return !regExp.pattern().isEmpty() && regExp.isValid();
        }

        //------------------------------------------------------------------------------
        QString SecurityFilter::apply(const QString &aParameterName, const QString &aValue) const
        {
            QRegularExpression regExp = getMask(aParameterName);

            if (!regExp.pattern().isEmpty() && regExp.isValid())
            {
                QRegularExpressionMatch match = regExp.match(aValue);
                if (match.hasMatch())
                {
                    QString value = aValue;

                    QStringList capturedTexts = match.capturedTexts();
                    for (int i = 1; i < capturedTexts.size(); i++)
                    {
                        QString captured = capturedTexts[i];
                        if (!captured.isEmpty())
                        {
                            int pos = match.capturedStart(i);
                            value.replace(pos, captured.size(), QString("*").repeated(captured.size()));
                        }
                    }

                    return value;
                }
                else
                {
                    qDebug() << QString("RegExp '%1' not found value in: %2").arg(regExp.pattern()).arg(aValue);
                }
            }

            return aValue;
        }

        //------------------------------------------------------------------------------
        QRegularExpression SecurityFilter::getMask(const QString &aParameterName) const
        {
            foreach (auto field, mProvider.fields)
            {
                if (aParameterName.contains(field.id, Qt::CaseInsensitive))
                {
                    auto subsystem = field.security.contains(mSubsystem) ? mSubsystem : SProviderField::Default;
                    QString regExp = field.security.value(subsystem, QString());

                    if (!regExp.isEmpty())
                    {
                        QRegularExpression rx(regExp);

                        if (rx.isValid())
                        {
                            return rx;
                        }

                        qDebug() << QString("RegExp '%1' not valid: %2.").arg(regExp).arg(rx.errorString());
                    }
                }
            }

            return QRegularExpression();
        }

        //------------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK
