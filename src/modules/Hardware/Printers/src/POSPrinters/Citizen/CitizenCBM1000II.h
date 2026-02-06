/* @file Принтер Citizen CBM-1000II. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
template <class T> class CitizenCBM1000II : public CitizenBase<POSPrinter<T>> {
    SET_SUBSERIES("CitizenCBM1000II")

public:
    CitizenCBM1000II();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);
};

//--------------------------------------------------------------------------------
template <class T> CitizenCBM1000II<T>::CitizenCBM1000II() {
    using namespace SDK::Driver::IOPort::COM;

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
    this->mDeviceName = "Citizen CBM-1000II";
    this->mModelID = '\x30';
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 3);

    // модели
    this->mModelData.data().clear();
    this->mModelData.add(this->mModelID, true, this->mDeviceName);

    this->setConfigParameter(CHardware::Printer::FeedingAmount, 5);
}

//--------------------------------------------------------------------------------
template <class T>
void CitizenCBM1000II<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
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
class SerialCitizenCBM1000II : public SerialPOSPrinter<CitizenCBM1000II<TSerialPrinterBase>> {
public:
    SerialCitizenCBM1000II() {
        using namespace SDK::Driver::IOPort::COM;

        // параметры порта
        mPortParameters.insert(EParameters::BaudRate,
                               POSPrinters::TSerialDevicePortParameter()
                                   << EBaudRate::BR38400 << EBaudRate::BR19200 << EBaudRate::BR4800
                                   << EBaudRate::BR9600);
    }
};

//--------------------------------------------------------------------------------
typedef CitizenCBM1000II<TLibUSBPrinterBase> LibUSBCitizenCBM1000II;

//--------------------------------------------------------------------------------
