/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#include "Paymaster.h"

using namespace SDK::Driver;

template class Paymaster<Atol2OnlineFRBase>;
template class Paymaster<Atol3OnlineFRBase>;

//--------------------------------------------------------------------------------
template <class T> Paymaster<T>::Paymaster() {
    m_DeviceName = CAtolFR::Models::Paymaster;
    m_SupportedModels = QStringList() << m_DeviceName;
    m_FRBuildUnifiedTaxes = 4799;

    setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

    using namespace SDK::Driver::IOPort::COM;

    // данные порта
    m_PortParameters[EParameters::BaudRate].clear();
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
}

//--------------------------------------------------------------------------------
template <class T> void Paymaster<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    AtolVKP80BasedFR<T>::setDeviceConfiguration(aConfiguration);

    QString printerModel =
        getConfigParameter(CHardware::FR::PrinterModel, CAtolOnlinePrinters::Default).toString();

    if (aConfiguration.contains(CHardware::FR::PrinterModel) &&
        (printerModel != CAtolOnlinePrinters::Default) && !isNotPrinting()) {
        m_PPTaskList.append([&]() { m_NotPrintingError = !setNotPrintDocument(false); });
    }
}

//--------------------------------------------------------------------------------
template <class T> char Paymaster<T>::getPrinterId() {
    QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();
    char result = CAtolOnlinePrinters::Models.data().key(printerModel, 0);

    if (!result) {
        toLog(LogLevel::Error, m_DeviceName + ": Unknown printer model " + printerModel);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::updateParameters() {
    bool result = AtolVKP80BasedFR<T>::updateParameters();

    if (m_FRBuild) {
        m_OldFirmware = m_FRBuild < CPaymaster::MinFRBuild;
    }

    if (!result) {
        return false;
    }

    QByteArray data;

#define SET_LCONFIG_FISCAL_FIELD(aName)                                                            \
    if (getTLV(CFR::FiscalFields::aName, data)) {                                                  \
        m_FFEngine.setConfigParameter(CFiscalSDK::aName, m_Codec->toUnicode(data));                \
        QString value = m_FFEngine.getConfigParameter(CFiscalSDK::aName, data).toString();         \
        toLog(LogLevel::Normal,                                                                    \
              m_DeviceName + QString(": Add %1 = \"%2\" to config data")                           \
                                 .arg(m_FFData.getTextLog(CFR::FiscalFields::aName))               \
                                 .arg(value));                                                     \
    }

    SET_LCONFIG_FISCAL_FIELD(OFDURL);

    QString automaticNumber =
        getConfigParameter(CFiscalSDK::AutomaticNumber).toString().simplified();

    if (!automaticNumber.isEmpty()) {
        if (getTLV(CFR::FiscalFields::AutomaticNumber, data)) {
            setDeviceParameter(CDeviceData::FR::AutomaticNumber, data.simplified().toULongLong());

            if (data != automaticNumber) {
                toLog(
                    LogLevel::Warning,
                    QString("%1: The automatic number in FR = %2 is different from AP number = %3")
                        .arg(m_DeviceName)
                        .arg(data.data())
                        .arg(automaticNumber));

                // TODO: если будет возвращаться результат - то заходить сюда только в фискальном
                // режиме return setTLV(FiscalFields::AutomaticNumber);
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void Paymaster<T>::processDeviceData() {
    AtolVKP80BasedFR<T>::processDeviceData();

    QByteArray data;
    char mode = m_Mode;

    if (enterInnerMode(CAtolFR::InnerModes::Programming) &&
        getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) && !data.isEmpty()) {
        if (!CAtolOnlinePrinters::Models.data().contains(data[0])) {
            toLog(LogLevel::Error,
                  QString("%1: Unknown printer model Id %2").arg(m_DeviceName).arg(uchar(data[0])));
        } else {
            QString printerModel = CAtolOnlinePrinters::Models[data[0]];
            QString configModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

            if ((configModel.isEmpty() || (configModel == CAtolOnlinePrinters::Default)) &&
                (printerModel != configModel)) {
                setConfigParameter(CHardware::FR::PrinterModel, printerModel);

                emit configurationChanged();
            }
        }
    }

    enterInnerMode(mode);
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::enterExtendedMode() {
    char mode = m_Mode;

    if (!enterInnerMode(CAtolFR::InnerModes::Programming)) {
        return false;
    }

    if (!setFRParameter(CAtolOnlineFR::FRParameters::SetAutoZReportTiming,
                        CAtolOnlineFR::AutoZReportTimingEnable)) {
        enterInnerMode(mode);

        toLog(LogLevel::Error, m_DeviceName + ": Failed to enable auto Z-report timing");
        return false;
    }

    QByteArray data;
    bool result = getFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming, data) &&
                  !data.isEmpty() && (data[0] == CAtolOnlineFR::ZReportInBuffer);

    if (!result) {
        result = setFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming,
                                CAtolOnlineFR::ZReportInBuffer);

        if (!result) {
            toLog(LogLevel::Error, m_DeviceName + "Failed to enter to Z-report mode");
        } else {
            result = reboot();
        }
    }

    enterInnerMode(mode);

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::setNotPrintDocument(bool aEnabled, bool /*aZReport*/) {
    char printingValue = aEnabled ? CAtolOnlinePrinters::Memory : getPrinterId();

    if (!printingValue) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Failed to set printer model due to printer model Id is invalid");
        return false;
    }

    char mode = m_Mode;

    if (!enterInnerMode(CAtolFR::InnerModes::Programming)) {
        return false;
    }

    QByteArray data;
    bool result = getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) &&
                  !data.isEmpty() && (data[0] == printingValue);

    if (!result) {
        result =
            setFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, printingValue) && reboot();
    }

    enterInnerMode(mode);

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::printDeferredZReports() {
    if (!AtolVKP80BasedFR<T>::printDeferredZReports()) {
        return false;
    }

    if (!processCommand(CAtolOnlineFR::Commands::ClearZBuffer)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to clear Z-buffer");
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::perform_ZReport(bool /*aPrintDeferredReports*/) {
    toLog(LogLevel::Normal, "AtolFR: Processing Z-report");

    // если ККМ работает в расширенном режиме - печатаем отложенные Z-отчеты
    bool printDeferredZReportSuccess = printDeferredZReports();
    bool ZReportSuccess = execZReport(true) && printDeferredZReports();
    bool result = printDeferredZReportSuccess || ZReportSuccess;

    // ошибки наполнения и переполнения буфера Z-Отчетов можно будет сбросить, если будет доработан
    // функционал запроса свободного места в Z-буфере checkZBufferState();

    if (result) {
        m_ZBufferOverflow = false;

        if (printDeferredZReportSuccess) {
            m_ZBufferFull = false;
        }
    }

    exitInnerMode();

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool Paymaster<T>::setFRParameters() {
    if (!AtolVKP80BasedFR<T>::setFRParameters()) {
        return false;
    }

    return setFRParameter(CAtolOnlineFR::FRParameters::SetReportsProcessing,
                          CAtolOnlineFR::PresentReports);
}

//--------------------------------------------------------------------------------
template <class T>
void Paymaster<T>::parseShortStatusFlags(char aFlags, TStatusCodes &aStatusCodes) {
    AtolVKP80BasedFR<T>::parseShortStatusFlags(aFlags, aStatusCodes);

    QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

    if (aFlags & CPaymaster::ShortStatusError::PaperJam) {
        aStatusCodes.insert(PrinterStatusCode::Error::PaperJam);
    }

    if ((aFlags & CPaymaster::ShortStatusError::Presenter) &&
        (printerModel == CAtolOnlinePrinters::CitizenPPU700)) {
        aStatusCodes.insert(PrinterStatusCode::Error::Presenter);
    }
}

//--------------------------------------------------------------------------------
