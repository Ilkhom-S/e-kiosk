/* @file USB-сканер. */

// System
#include <Hardware/Scanners/HHPDetectingData.h>
#include <Hardware/Scanners/USBScanner.h>

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
USBScanner::USBScanner() {
    mDeviceName = CHHP::DefaultName;

    mDetectingData->set(CUSBVendors::HHP, CHHP::DetectingData().data());
}

//--------------------------------------------------------------------------------
QStringList USBScanner::getModelList() {
    return CHHP::DetectingData().getModelList(CUSBVendors::HHP);
}

//--------------------------------------------------------------------------------
bool USBScanner::getData(QByteArray &aAnswer) {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::MaxReadingSize, CUSBScanner::USBAnswerSize);
    mIOPort->setDeviceConfiguration(configuration);

    return TUSBScanner::getData(aAnswer);
}

//--------------------------------------------------------------------------------
