/* @file Реализация запроса статуса платежа. */

#include "PaymentResponse.h"

#include <QtCore/QStringDecoder>

#include <SDK/PaymentProcessor/Humo/ErrorCodes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

#include <Crypt/ICryptEngine.h>

#include "Payment.h"
#include "PaymentRequest.h"

using SDK::PaymentProcessor::SProvider;
namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
PaymentResponse::PaymentResponse(const Request &aRequest, const QString &aResponseString)
    : Response(aRequest, aResponseString) {
    // Добавляем параметры, прописанные в конфиге как входящие, в свойства платежа, связанного с
    // запросом.
    const PaymentRequest *request = dynamic_cast<const PaymentRequest *>(&aRequest);

    if (request) {
        setLog(request->getPayment()->getPaymentFactory()->getLog());

        if (request->getPayment()->getProviderSettings().processor.requests.contains(
                request->getName())) {
            auto getFieldCodec =
                [](const SProvider::SProcessingTraits::SRequest::SResponseField &aField)
                -> QStringDecoder {
                switch (aField.codepage) {
                case SProvider::SProcessingTraits::SRequest::SResponseField::Utf8:
                    return QStringDecoder(QStringDecoder::Utf8);

                default:
                    return QStringDecoder("windows-1251");
                }
            };

            auto convertFieldValue =
                [&getFieldCodec](
                    const SProvider::SProcessingTraits::SRequest::SResponseField &aField,
                    const QVariant &aValue) -> QVariant {
                QStringDecoder decoder = getFieldCodec(aField);
                switch (aField.encoding) {
                case SProvider::SProcessingTraits::SRequest::SResponseField::Url:
                    return QString(decoder.decode(
                        QByteArray::fromPercentEncoding(aValue.toString().toLatin1())));

                case SProvider::SProcessingTraits::SRequest::SResponseField::Base64:
                    return QString(
                        decoder.decode(QByteArray::fromBase64(aValue.toString().toLatin1())));

                default:
                    return aValue;
                }
            };

            foreach (auto field,
                     request->getPayment()
                         ->getProviderSettings()
                         .processor.requests[request->getName()]
                         .responseFields) {
                QVariant value = getParameter(field.name);

                // проверка наличия поля в ответе
                if (!value.isValid()) {
                    // В платеже может сохраниться значение поля от предыдущего запроса. Сбросим.
                    request->getPayment()->setParameter(
                        Payment::SParameter(field.name, QString(), true));

                    if (field.required) {
                        toLog(LogLevel::Error,
                              QString(
                                  "Payment %1. Can't find expected param '%2' in server response.")
                                  .arg(request->getPayment()->getID())
                                  .arg(field.name));

                        if (!getError() || getError() == ELocalError::NetworkError) {
                            // Добавляем ошибку, если другой нет
                            addParameter(CResponse::Parameters::Error,
                                         QString::number(ELocalError::AbsentExpectedParam));
                        }
                    } else {
                        toLog(LogLevel::Warning,
                              QString(
                                  "Payment %1. Can't find optional param '%2' in server response.")
                                  .arg(request->getPayment()->getID())
                                  .arg(field.name));
                    }

                    continue;
                }

                QString valueString = value.toString();

                // Поле может быть зашифровано, проверяем.
                if (field.crypted) {
                    ICryptEngine *cryptEngine =
                        request->getPayment()->getPaymentFactory()->getCryptEngine();

                    QByteArray decryptedValue;

                    QString error;

                    if (cryptEngine->decrypt(
                            request->getPayment()->getProviderSettings().processor.keyPair,
                            valueString.toLatin1(),
                            decryptedValue,
                            error)) {
                        mCryptedFields << field.name;

                        valueString = QString::fromLatin1(decryptedValue);
                    } else {
                        toLog(LogLevel::Error,
                              QString("Payment %1. Failed to decrypt parameter %2. Error: %3.")
                                  .arg(request->getPayment()->getID())
                                  .arg(field.name)
                                  .arg(error));
                    }

                    request->getPayment()->setParameter(
                        Payment::SParameter(field.name, valueString, true, true));
                } else {
                    auto convertedValue = convertFieldValue(field, value);
                    request->getPayment()->setParameter(
                        Payment::SParameter(field.name, convertedValue, true));

                    // Единственное поле из стандартного протокола, которое может быть
                    // перекодировано.
                    if (field.name == CResponse::Parameters::ErrorMessage) {
                        addParameter(field.name, convertedValue.toString());
                    }
                }
            }
        }

        QList<QPair<QString, QString>> protocolParameters;

        protocolParameters
            << QPair<QString, QString>("TRANSID", PPSDK::CPayment::Parameters::TransactionId)
            << QPair<QString, QString>("AUTHCODE", PPSDK::CPayment::Parameters::AuthCode)
            << QPair<QString, QString>("GATEWAY_IN", PPSDK::CPayment::Parameters::MNPGatewayIn)
            << QPair<QString, QString>("GATEWAY_OUT", PPSDK::CPayment::Parameters::MNPGatewayOut);

        foreach (auto parameter, protocolParameters) {
            QVariant value = getParameter(parameter.first);
            if (value.isValid()) {
                request->getPayment()->setParameter(
                    Payment::SParameter(parameter.second, value, true));
            }
        }
    }
}

//---------------------------------------------------------------------------
QString PaymentResponse::toLogString() const {
    QStringList result;

    // Значения зашифрованных полей мы должны скрывать.
    for (auto it = getParameters().begin(); it != getParameters().end(); ++it) {
        result << QString("%1 = \"%2\"")
                      .arg(it.key())
                      .arg(mCryptedFields.contains(it.key()) ? "**CRYPTED**"
                                                             : it.value().toString());
    }

    return result.join(", ");
}

//---------------------------------------------------------------------------
