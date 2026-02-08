/* @file Дефолтное HID-устройство на COM-порту. */

#include <Hardware/Scanners/SerialScanner.h>

SerialScanner::SerialScanner() {
    m_DeviceName = "Generic serial HID";
    m_AutoDetectable = false;
}

//--------------------------------------------------------------------------------
bool SerialScanner::getData(QByteArray &aAnswer) {
    if (m_IOPort->getType() == SDK::Driver::EPortTypes::VirtualCOM) {
        QVariantMap configuration;
        configuration.insert(CHardware::Port::COM::WaitResult, true);
        m_IOPort->setDeviceConfiguration(configuration);
    }

    return TSerialScanner::getData(aAnswer);
}

//--------------------------------------------------------------------------------
