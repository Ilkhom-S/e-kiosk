/* @file Прокси-класс для работы со сканером в скриптах. */

#include <QtCore/QBuffer>
#include <QtCore/QStringList>
#include <QtGui/QImage>
#include <QtQml/QJSEngine>

#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IHIDService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Scripting/HIDService.h>

namespace PPSDK = SDK::PaymentProcessor;

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
HIDService::HIDService(ICore *aCore) : m_Core(aCore), m_Service(m_Core->getHIDService()) {
    connect(m_Service, SIGNAL(error()), SIGNAL(error()));

    // Сигналы из ядра завернем в общий скриптовый сигнал hiddata
    connect(m_Service, SIGNAL(ejected()), this, SLOT(onEjected()));
    connect(m_Service, SIGNAL(inserted(QVariantMap)), this, SLOT(onInserted(QVariantMap)));

    connect(m_Service,
            SIGNAL(data(const QVariantMap &)),
            this,
            SLOT(onData(const QVariantMap &)),
            Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
void HIDService::enable(const QString &aName) {
    m_Service->setEnable(true, aName);
}

//------------------------------------------------------------------------------
void HIDService::disable(const QString &aName) {
    m_Service->setEnable(false, aName);
}

//------------------------------------------------------------------------------
void HIDService::updateParameters(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString HIDService::getExternalData() {
    qint64 providerId =
        m_Parameters[SDK::PaymentProcessor::CPayment::Parameters::Provider].toLongLong();

    return m_Core->getPaymentService()->getProvider(providerId).externalDataHandler;
}

//------------------------------------------------------------------------------
/*QJSValue JS_setField(QJSContext * aContext, QJSEngine *, void * aParameters)
{
        //reinterpret_cast<QVariantMap *>(aParameters)->insert(aContext->argument(0).toString(),
aContext->argument(1).toString());

        return QJSValue();
}*/

//------------------------------------------------------------------------------
void HIDService::onData(const QVariantMap &aDataMap) {
    QVariantMap parameters;
    QString value;

    if (aDataMap.contains(CHardwareSDK::HID::Text)) {
        value = m_Service->valueToString(aDataMap.value(CHardwareSDK::HID::Text));

        parameters.insert(HID::SOURCE, HID::SOURCE_SCANNER);
        parameters.insert(HID::STRING, value);
        parameters.insert(HID::RAW, value);
        parameters.insert(HID::RAW_BASE64, QString(value.toLatin1().toBase64()));

        parameters.insert(HID::EXTERNAL_DATA, false);

        QString externalDataHandler = getExternalData();
        if (!externalDataHandler.trimmed().isEmpty() && !value.isEmpty()) {
            QJSEngine script;

            // TODO PORT_QT5

            /*script.globalObject().setProperty("value", value);
            QJSValue setFunc = script.newFunction(JS_setField, &parameters);
            script.globalObject().setProperty("setField", setFunc);

            if (!script.canEvaluate(externalDataHandler))
            {
                    toLog(LogLevel::Warning, QString("Can't parse expression:
            %1").arg(externalDataHandler)); return;
            }

            script.evaluate(externalDataHandler);
            if (script.hasUncaughtException())
            {
                    toLog(LogLevel::Error, QString("An exception occured while calling (line %1):
            %2\nBacktrace:\n%3.") .arg(script.uncaughtExceptionLineNumber())
                            .arg(script.uncaughtException().toString())
                            .arg(script.uncaughtExceptionBacktrace().join("\n")));

                    return;
            }*/

            parameters.insert(HID::EXTERNAL_DATA, true);
        }
    }

    if (aDataMap.contains(CHardwareSDK::HID::Image)) {
        bool faceDetected = aDataMap.value(CHardwareSDK::HID::FaceDetected, false).value<bool>();

        automage = aDataMap.value(CHardwareSDK::HID::Image).value<QImage>();
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        image.convertToFormat(QImage::Format_RGB16).save(&buffer, "jpg");

        parameters.insert(HID::SOURCE, HID::SOURCE_CAMERA);
        parameters.insert(HID::RAW, image);
        parameters.insert(HID::RAW_BASE64, QString(buffer.data().toBase64()));
        parameters.insert(HID::CAMERA_FACE_DETECTED, faceDetected);

        if (faceDetected) {
            buffer.open(QIODevice::WriteOnly);
            aDataMap.value(CHardwareSDK::HID::ImageWithFaceArea)
                .value<QImage>()
                .convertToFormat(QImage::Format_RGB16)
                .save(&buffer, "jpg");
            parameters.insert(HID::CAMERA_FACE_DETECTED_IMAGE, QString(buffer.data().toBase64()));
        }
    }

    emit HIDData(parameters);
}

//------------------------------------------------------------------------------
void HIDService::onInserted(const QVariantMap &aData) {
    QVariantMap data;

    data.insert(HID::SOURCE, HID::SOURCE_CARD);
    data.insert(HID::SIGNAL, HID::SIGNAL_INSERT);

    foreach (QString name, aData.keys()) {
        data.insert(name, aData[name]);
    }

    emit HIDData(data);
}

//------------------------------------------------------------------------------
void HIDService::onEjected() {
    QVariantMap data;

    data.insert(HID::SOURCE, HID::SOURCE_CARD);
    data.insert(HID::SIGNAL, HID::SIGNAL_EJECT);

    emit HIDData(data);
}

//------------------------------------------------------------------------------
void HIDService::executeExternalHandler(const QVariantMap &aExpression) {
    emit externalHandler(aExpression);
}

//------------------------------------------------------------------------------

} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
// Definitions for HID constants
namespace SDK {
namespace PaymentProcessor {
namespace Scripting {
namespace HID {
extern const char STRING[] = "hid_string";
extern const char EXTERNAL_DATA[] = "hid_external_data";
extern const char RAW[] = "hid_raw";
extern const char RAW_BASE64[] = "hid_raw_base64";
extern const char SOURCE[] = "hid_source";
extern const char SOURCE_CAMERA[] = "camera";
extern const char SOURCE_SCANNER[] = "scanner";
extern const char SOURCE_CARD[] = "card";
extern const char SIGNAL[] = "signal";
extern const char SIGNAL_INSERT[] = "insert";
extern const char SIGNAL_EJECT[] = "eject";

extern const char CAMERA_FACE_DETECTED[] = "hid_camera_face_detected";
extern const char CAMERA_FACE_DETECTED_IMAGE[] = "hid_camera_face_image_base64";
} // namespace HID
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
