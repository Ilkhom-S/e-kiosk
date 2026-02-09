/* @file Принтер Custom VKP-80. */

#include <Hardware/Printers/CustomVKP80.h>

template class Custom_VKP80<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> Custom_VKP80<T>::Custom_VKP80() {
    this->m_Parameters = POSPrinters::CommonParameters;

    // статусы ошибок
    this->m_Parameters.errors.clear();

    this->m_Parameters.errors[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
    this->m_Parameters.errors[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
    this->m_Parameters.errors[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
    // this->m_Parameters.errors[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

    this->m_Parameters.errors[20][4].insert('\x03', DeviceStatusCode::Error::CoverIsOpened);
    this->m_Parameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);

    this->m_Parameters.errors[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
    this->m_Parameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Port);
    this->m_Parameters.errors[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
    this->m_Parameters.errors[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

    this->m_Parameters.errors[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
    this->m_Parameters.errors[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

    // теги
    this->m_Parameters.tagEngine.appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
    this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleWidth, "\x1B\x21", "\x20");
    this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
    this->m_Parameters.tagEngine.appendSingle(Tags::Type::Amount, "", "\xFE");

    // параметры моделей
    this->m_DeviceName = "Custom VKP-80";
    this->m_ModelID = '\xB9';

    this->setConfigParameter(CHardware::Printer::FeedingAmount, 0);
    this->setConfigParameter(CHardwareSDK::Printer::LineSize, 42);

    // модели
    this->m_ModelData.data().clear();
    this->m_ModelData.add('\x5D', true, this->m_DeviceName, "resolution 200 dpi");
    this->m_ModelData.add('\x5E', true, this->m_DeviceName, "resolution 300 dpi");
    this->m_ModelData.add('\xB9', true, this->m_DeviceName, "default");
    this->m_ModelData.add('\x95', true, this->m_DeviceName, "modification VKP80II-EE");
}

//--------------------------------------------------------------------------------
template <class T> void Custom_VKP80<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    this->setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardware::Codepage)) {
        using namespace CHardware::Codepages;

        QString codepage = aConfiguration[CHardware::Codepage].toString();
        QString codecName = (codepage == Custom_KZT) ? Custom_KZT : CP866;

        this->m_Decoder = CodecByName[codecName];
    }
}

//--------------------------------------------------------------------------------
template <class T> bool Custom_VKP80<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    if (this->m_Overflow) {
        PollingExpector expector;
        QByteArray data;

        if (!expector.wait(
                [&]() -> bool { return this->getAnswer(data, 10) && data.contains(ASCII::XOn); },
                CCustom_VKP80::XOnWaiting)) {
            return false;
        }

        this->m_Overflow = false;
    }

    bool result = this->printReceipt(aLexemeReceipt);

    if (!this->getConfigParameter(CHardware::Printer::OutCall).toBool()) {
        auto condition = [&]() -> bool {
            TStatusCodes statusCodes;
            return this->getStatus(statusCodes) &&
                   !statusCodes.contains(PrinterStatusCode::OK::MotorMotion);
        };
        PollingExpector().wait(condition, CCustom_VKP80::PrintingWaiting);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool Custom_VKP80<T>::updateParametersOut() {
    return this->updateParameters();
}

//--------------------------------------------------------------------------------
