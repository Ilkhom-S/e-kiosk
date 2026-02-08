/* @file USB-сканер. */

#include <Hardware/Scanners/HHPDetectingData.h>
#include <Hardware/Scanners/USBScanner.h>

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
USBScanner::USBScanner() {
    m_DeviceName = CHHP::DefaultName;

    m_DetectingData->set(CUSBVendors::HHP, CHHP::DetectingData().data());
}

//--------------------------------------------------------------------------------
QStringList USBScanner::getModelList() {
    return CHHP::DetectingData().getModelList(CUSBVendors::HHP);
}

//--------------------------------------------------------------------------------
bool USBScanner::getData(QByteArray &aAnswer) {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::MaxReadingSize, CUSBScanner::USBAnswerSize);
    m_IOPort->setDeviceConfiguration(configuration);

    return TUSBScanner::getData(aAnswer);
}

//--------------------------------------------------------------------------------
