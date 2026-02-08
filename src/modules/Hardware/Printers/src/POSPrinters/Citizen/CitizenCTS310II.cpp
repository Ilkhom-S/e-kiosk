/* @file Принтер Citizen CT-S310II. */

#include "CitizenCTS310II.h"

CitizenCTS310II::CitizenCTS310II() {
    // статусы ошибок
    m_Parameters.errors.clear();

    m_Parameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    m_Parameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    m_Parameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[3][1].insert('\x04', DeviceStatusCode::Error::Mechanism_Position);
    m_Parameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    m_Parameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    m_Parameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    m_Parameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

    // параметры моделей
    setConfigParameter(CHardware::Printer::FeedingAmount, 3);
    m_DeviceName = "Citizen CT-S310II";
    m_ModelID = '\x3D';

    // модели
    m_ModelData.data().clear();
    m_ModelData.add(m_ModelID, true, m_DeviceName);
}

//--------------------------------------------------------------------------------