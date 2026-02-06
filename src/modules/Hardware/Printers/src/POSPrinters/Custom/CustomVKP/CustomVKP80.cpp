/* @file Принтер Custom VKP-80. */

#include "CustomVKP80.h"

template class CustomVKP80<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> CustomVKP80<T>::CustomVKP80() {
    this->mParameters = POSPrinters::CommonParameters;

    // статусы ошибок
    this->mParameters.errors.clear();

    this->mParameters.errors[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
    this->mParameters.errors[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
    this->mParameters.errors[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
    // this->mParameters.errors[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

    this->mParameters.errors[20][4].insert('\x03', DeviceStatusCode::Error::CoverIsOpened);
    this->mParameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);

    this->mParameters.errors[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
    this->mParameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Port);
    this->mParameters.errors[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
    this->mParameters.errors[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

    this->mParameters.errors[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
    this->mParameters.errors[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

    // теги
    this->mParameters.tagEngine.appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
    this->mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth, "\x1B\x21", "\x20");
    this->mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
    this->mParameters.tagEngine.appendSingle(Tags::Type::Amount, "", "\xFE");

    // параметры моделей
    this->mDeviceName = "Custom VKP-80";
    this->mModelID = '\xB9';

    this->setConfigParameter(CHardware::Printer::FeedingAmount, 0);
    this->setConfigParameter(CHardwareSDK::Printer::LineSize, 42);

    // модели
    this->mModelData.data().clear();
    this->mModelData.add('\x5D', true, this->mDeviceName, "resolution 200 dpi");
    this->mModelData.add('\x5E', true, this->mDeviceName, "resolution 300 dpi");
    this->mModelData.add('\xB9', true, this->mDeviceName, "default");
    this->mModelData.add('\x95', true, this->mDeviceName, "modification VKP80II-EE");
}

//--------------------------------------------------------------------------------
template <class T> void CustomVKP80<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    this->setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardware::Codepage)) {
        using namespace CHardware::Codepages;

        QString codepage = aConfiguration[CHardware::Codepage].toString();
        QString codecName = (codepage == CustomKZT) ? CustomKZT : CP866;

        this->mDecoder = CodecByName[codecName];
    }
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    if (this->mOverflow) {
        PollingExpector expector;
        QByteArray data;

        if (!expector.wait(
                [&]() -> bool { return this->getAnswer(data, 10) && data.contains(ASCII::XOn); },
                CCustomVKP80::XOnWaiting)) {
            return false;
        }

        this->mOverflow = false;
    }

    bool result = this->printReceipt(aLexemeReceipt);

    if (!this->getConfigParameter(CHardware::Printer::OutCall).toBool()) {
        auto condition = [&]() -> bool {
            TStatusCodes statusCodes;
            return this->getStatus(statusCodes) &&
                   !statusCodes.contains(PrinterStatusCode::OK::MotorMotion);
        };
        PollingExpector().wait(condition, CCustomVKP80::PrintingWaiting);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool CustomVKP80<T>::updateParametersOut() {
    return this->updateParameters();
}

//--------------------------------------------------------------------------------
