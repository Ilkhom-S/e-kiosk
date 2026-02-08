/* @file Принтер Citizen CPP-8001. */

#include "CitizenCPP8001.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
CitizenCPP8001::CitizenCPP8001() {
    // параметры порта
    m_PortParameters.insert(EParameters::BaudRate,
                            POSPrinters::TSerialDevicePortParameter()
                                << EBaudRate::BR38400 << EBaudRate::BR19200 << EBaudRate::BR4800
                                << EBaudRate::BR9600);

    // статусы ошибок
    m_Parameters.errors.clear();

    m_Parameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    m_Parameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    m_Parameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    m_Parameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    m_Parameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

    // параметры моделей
    m_DeviceName = "Citizen CPP-8001";
    m_ModelID = '\x20';

    // модели
    m_ModelData.data().clear();
    m_ModelData.add(m_ModelID, true, m_DeviceName);

    setConfigParameter(CHardware::Printer::FeedingAmount, 6);
}

//--------------------------------------------------------------------------------
void CitizenCPP8001::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    POSPrinter::setDeviceConfiguration(aConfiguration);

    int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

    int feeding = 6;
    if (lineSpacing >= 75)
        feeding = 3;
    else if (lineSpacing >= 60)
        feeding = 4;
    else if (lineSpacing >= 50)
        feeding = 5;

    setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------