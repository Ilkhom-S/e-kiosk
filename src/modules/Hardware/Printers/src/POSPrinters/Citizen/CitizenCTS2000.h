/* @file Принтер Citizen CTS-2000. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
template <class T> class CitizenCTS2000 : public CitizenBase<POSPrinter<T>> {
    SET_SUBSERIES("CitizenCTS2000")

public:
    CitizenCTS2000();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);
};

//--------------------------------------------------------------------------------
template <class T> CitizenCTS2000<T>::CitizenCTS2000() {
    // статусы ошибок
    this->m_Parameters.errors.clear();

    this->m_Parameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->m_Parameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->m_Parameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    this->m_Parameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    this->m_Parameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    this->m_Parameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

    // параметры моделей
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 5);
    this->m_DeviceName = "Citizen CT-S2000";
    this->m_ModelID = '\x51';

    // модели
    this->m_ModelData.data().clear();
    this->m_ModelData.add(this->m_ModelID, true, this->m_DeviceName);
}

//--------------------------------------------------------------------------------
template <class T>
void CitizenCTS2000<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    POSPrinter<T>::setDeviceConfiguration(aConfiguration);

    int lineSpacing = this->getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

    int feeding = 5;
    if (lineSpacing >= 75)
        feeding = 2;
    else if (lineSpacing >= 55)
        feeding = 3;
    else if (lineSpacing >= 45)
        feeding = 4;

    this->setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CitizenCTS2000<TSerialPrinterBase>> SerialCitizenCTS2000;
typedef CitizenCTS2000<TLibUSBPrinterBase> LibUSBCitizenCTS2000;

//--------------------------------------------------------------------------------
