/* @file Класс для тестирования сканеров. */

#include "HIDTest.h"

#include <QtCore/QStringDecoder>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IHIDService.h>

namespace CHIDTest {
const QString TestRead = QT_TRANSLATE_NOOP("HIDTest", "#test_read");
} // namespace CHIDTest

//------------------------------------------------------------------------------
HIDTest::HIDTest(SDK::Driver::IDevice *aDevice, const QString &aInstancePath) {
    mHID = dynamic_cast<SDK::Driver::IHID *>(aDevice);

    mTestNames << qMakePair(CHIDTest::TestRead,
                            aInstancePath.contains(SDK::Driver::CComponents::Camera)
                                ? tr("#wait_photo")
                                : tr("#read_barcode"));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> HIDTest::getTestNames() const {
    return mTestNames;
}

//------------------------------------------------------------------------------
bool HIDTest::run(const QString &aName) {
    if (aName == CHIDTest::TestRead) {
        if (mHID->enable(true)) {
            mHID->subscribe(SDK::Driver::IHID::DataSignal, this, SLOT(onData(const QVariantMap &)));
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void HIDTest::stop() {
    mHID->unsubscribe(SDK::Driver::IHID::DataSignal, this);
    mHID->enable(false);
}

//------------------------------------------------------------------------------
bool HIDTest::isReady() {
    return mHID && mHID->isDeviceReady();
}

//------------------------------------------------------------------------------
bool HIDTest::hasResult() {
    return true;
}

//------------------------------------------------------------------------------
void HIDTest::onData(const QVariantMap &aDataMap) {
    QString value;

    if (aDataMap.contains(CHardwareSDK::HID::Text)) {
        const QVariant data = aDataMap.value(CHardwareSDK::HID::Text);

        switch (data.typeId()) {
        case QMetaType::QString:
            value = data.value<QString>();
            break;

        case QMetaType::QByteArray: {
            QStringDecoder decoderUtf8(QStringDecoder::Utf8);
            QString utf8 = decoderUtf8.decode(data.toByteArray());
            if (decoderUtf8.hasError()) {
                QStringDecoder decoderCp1251("windows-1251");
                utf8 = decoderCp1251.decode(data.toByteArray());
            }
            value = utf8;
        } break;
        default:
            value = data.toString();
        }

        if (!value.isEmpty()) {
            emit result("", QString("%1 %2").arg(tr("#readed")).arg(value));
        }
    }

    if (aDataMap.contains(CHardwareSDK::HID::Image)) {
        bool faceDetected = aDataMap.value(CHardwareSDK::HID::FaceDetected, false).toBool();

        emit result("",
                    aDataMap.value(faceDetected ? CHardwareSDK::HID::ImageWithFaceArea
                                                : CHardwareSDK::HID::Image));
    }
}

//------------------------------------------------------------------------------
