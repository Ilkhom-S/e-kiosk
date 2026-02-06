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
    this->mParameters.errors.clear();

    this->mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
    this->mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
    this->mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
    this->mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

    this->mParameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
    this->mParameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

    // параметры моделей
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 5);
    this->mDeviceName = "Citizen CT-S2000";
    this->mModelID = '\x51';

    // модели
    this->mModelData.data().clear();
    this->mModelData.add(this->mModelID, true, this->mDeviceName);
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
