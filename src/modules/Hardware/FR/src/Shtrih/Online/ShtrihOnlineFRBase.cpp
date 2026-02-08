/* @file Онлайн ФР семейства Штрих. */

#include "ShtrihOnlineFRBase.h"

//--------------------------------------------------------------------------------
template class ShtrihOnlineFRBase<ShtrihTCPFRBase>;
template class ShtrihOnlineFRBase<ShtrihSerialFRBase>;

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
template <class T> ShtrihOnlineFRBase<T>::ShtrihOnlineFRBase() {
    // данные семейства ФР
    m_SupportedModels = getModelList();
    m_DeviceName = CShtrihFR::Models::OnlineDefault;
    setConfigParameter(CHardwareSDK::CanOnline, true);
    m_NotEnableFirmwareUpdating = false;
    m_PrinterStatusEnabled = true;
    m_IsOnline = true;
    m_OFDFiscalFields.remove(CFR::FiscalFields::Cashier);
    m_OFDFiscalFields.remove(CFR::FiscalFields::TaxSystem);
    m_NeedReceiptProcessingOnCancel = true;

    setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

    // типы оплаты
    m_PayTypeData.add(EPayTypes::Cash, 1);
    m_PayTypeData.add(EPayTypes::EMoney, 2);
    m_PayTypeData.add(EPayTypes::PrePayment, 14);
    m_PayTypeData.add(EPayTypes::PostPayment, 15);
    m_PayTypeData.add(EPayTypes::CounterOffer, 16);

    // данные команд
    m_CommandData.add(CShtrihOnlineFR::Commands::Service, CShtrihFR::Timeouts::Default, false);

    // ошибки
    m_ErrorData = PErrorData(new CShtrihOnlineFR::Errors::Data);
    m_UnprocessedErrorData.add(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData,
                              CShtrihOnlineFR::Errors::NoRequiedDataInFS);
    m_UnprocessedErrorData.add(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData,
                              CShtrihFR::Errors::BadModeForCommand);
}

//--------------------------------------------------------------------------------
template <class T> QStringList ShtrihOnlineFRBase<T>::getModelList() {
    return CShtrihFR::Models::CData().getNonEjectorModels(true);
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::setNotPrintDocument(bool aEnabled, bool /*aZReport*/) {
    if (!aEnabled) {
        return true;
    }

    SleepHelper::msleep(CShtrihOnlineFR::NotPrintDocumentPause);
    bool result = setFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, true);
    SleepHelper::msleep(CShtrihOnlineFR::NotPrintDocumentPause);

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::updateParameters() {
    if (!ShtrihFRBase<T>::updateParameters()) {
        return false;
    }

    m_NotEnableFirmwareUpdating =
        containsDeviceParameter(CDeviceData::SDCard) && !enableFirmwareUpdating();

    QByteArray addressData;
    QByteArray portData;
    getFRParameter(CShtrihOnlineFR::FRParameters::OFDAddress, addressData);
    getFRParameter(CShtrihOnlineFR::FRParameters::OFDPort, portData);
    m_OFDDataError = !checkOFDData(addressData, revert(portData));

    // Формировать QR-код средствами ФР
    setFRParameter(CShtrihOnlineFR::FRParameters::QRCode, true);

    // Печатать сквозной номер документа
    setFRParameter(CShtrihOnlineFR::FRParameters::PrintEndToEndNumber, true);

    // Печатать данные ОФД в чеках
    setFRParameter(CShtrihOnlineFR::FRParameters::PrintOFDData, true);

    // Печатать все реквизиты пользователя в чеках
    setFRParameter(CShtrihOnlineFR::FRParameters::PrintUserData,
                   CShtrihOnlineFR::PrintFullUserData);

    // Печатать фискальные теги, вводимые на платеже
    setFRParameter(CShtrihOnlineFR::FRParameters::PrintCustom_Fields, true);

    // Отключить строгий ФЛК
    setFRParameter(CShtrihOnlineFR::FRParameters::StrongFormatChecking, false);

    if (!isFiscal()) {
        return true;
    }

    if (m_OperatorPresence && !setCashier()) {
        QByteArray data;

        if (!getFRParameter(
                CShtrihOnlineFR::FRParameters::Cashier, data, CShtrihOnlineFR::CashierSeries) ||
            data.simplified().isEmpty()) {
            toLog(LogLevel::Error, m_DeviceName + ": Cannot work with invalid cashier");
            return false;
        }
    }

    QByteArray data;

    if (getFRParameter(CShtrihOnlineFR::FRParameters::AutomaticNumber, data) &&
        !clean(data).isEmpty()) {
        setDeviceParameter(CDeviceData::FR::AutomaticNumber, clean(data));
    }

    if (!processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalizationTotal, &data) ||
        (data.size() <= 46)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get fiscalization total");
        return false;
    }

    if (!checkTaxSystems(data[40]) || !checkOperationModes(data[41])) {
        return false;
    }

    if (processCommand(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData, data.mid(43, 4))) {
        bool needWaitReady = getLongStatus() && (m_Mode == CShtrihFR::InnerModes::SessionClosed) ||
                             (m_Mode == CShtrihFR::InnerModes::DataEjecting);

        if (needWaitReady) {
            waitReady(CShtrihOnlineFR::ReadyWaiting);
        }

        TGetFiscalTLVData getFiscalTLVData = [&](QByteArray &aData) -> TResult {
            TResult result =
                processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &aData);
            aData = aData.mid(3);
            return result;
        };
        TProcessTLVAction checkingAgentFlags = [&](const CFR::STLV &aTLV) -> bool {
            return (aTLV.field != CFR::FiscalFields::AgentFlagsReg) ||
                   checkAgentFlags(aTLV.data[0]);
        };

        processTLVData(getFiscalTLVData, checkingAgentFlags);
    } else if (isErrorUnprocessed(m_LastCommand, m_LastError)) {
        m_ProcessingErrors.pop_back();
        m_LastError = 0;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::setCashier() {
    QString cashier;

    if (!m_FFEngine.checkCashier(cashier)) {
        return false;
    }

    if (!setFRParameter(
            CShtrihOnlineFR::FRParameters::Cashier, cashier, CShtrihOnlineFR::CashierSeries)) {
        toLog(LogLevel::Warning,
              m_DeviceName + QString(": Failed to set cashier fiscal field (%2)").arg(cashier));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> int ShtrihOnlineFRBase<T>::getSessionNumber() {
    QByteArray data;

    if (processCommand(CShtrihOnlineFR::Commands::FS::GetSessionParameters, &data) &&
        (data.size() > 5)) {
        return int(uchar(data[4])) | (int(uchar(data[5])) << 8);
    }

    return 0;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::setTaxValue(TVAT /*aVAT*/, int /*aGroup*/) {
    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::getStatus(TStatusCodes &aStatusCodes) {
    if (!ShtrihFRBase<T>::getStatus(aStatusCodes)) {
        return false;
    }

    uint bootFirmware = getDeviceParameter(CDeviceData::BootFirmware).toUInt();

    if ((m_Model != CShtrihFR::Models::ID::MStarTK2) &&
        (bootFirmware < CShtrihOnlineFR::MinBootFirmware)) {
        aStatusCodes.insert(DeviceStatusCode::Warning::BootFirmware);
    }

    if (m_PrinterStatusEnabled) {
        TResult result = processCommand(CShtrihOnlineFR::Commands::GetPrinterStatus);

        if (result == CommandResult::Device)
            aStatusCodes.insert(PrinterStatusCode::Error::PrinterFR);
        else if (result == CommandResult::Answer)
            aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
        else if (!result) {
            return false;
        }
    }

    if (m_NotEnableFirmwareUpdating) {
        aStatusCodes.insert(FRStatusCode::Warning::FirmwareUpdating);
    }

    QByteArray data =
        perform_Status(aStatusCodes, CShtrihOnlineFR::Commands::FS::GetOFDInterchangeStatus, 6);

    if (data == CFR::Result::Fail) {
        return false;
    } else if (data != CFR::Result::Error) {
        int OFDNotSentCount =
            (data.size() < 7) ? -1 : revert(data.mid(5, 2)).toHex().toUShort(0, 16);
        checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);
    }

    data = perform_Status(aStatusCodes, CShtrihOnlineFR::Commands::FS::GetStatus, 7);

    if (data == CFR::Result::Fail) {
        return false;
    } else if (data != CFR::Result::Error) {
        checkFSFlags(data[7], aStatusCodes);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void ShtrihOnlineFRBase<T>::processDeviceData() {
    ShtrihFRBase<T>::processDeviceData();

    QByteArray data;

    if (processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data)) {
        m_FSSerialNumber = CFR::FSSerialToString(data.mid(13, 16));
        int FDCount = revert(data.right(4)).toHex().toUInt(0, 16);

        setDeviceParameter(CDeviceData::FR::FiscalDocuments, FDCount);
    }

    if ((m_Model != CShtrihFR::Models::ID::MStarTK2) &&
        processCommand(
            CShtrihOnlineFR::Commands::Service, CShtrihOnlineFR::Service::BootFirmware, &data)) {
        if (data == QByteArray(data.size(), ASCII::NUL)) {
            toLog(LogLevel::Warning,
                  m_DeviceName + ": Failed to get boot firmware version due to the answer is NUL.");
        } else {
            uint bootFirmware = revert(data.mid(2)).toHex().toUInt(0, 16);
            setDeviceParameter(CDeviceData::BootFirmware, bootFirmware);
        }
    }

    if (processCommand(CShtrihOnlineFR::Commands::FS::GetValidity, &data)) {
        QDate date =
            QDate::from_String(hexToBCD(data.mid(3, 3)).prepend("20"), CShtrihOnlineFR::DateFormat);

        if (date.isValid()) {
            setDeviceParameter(CDeviceData::FS::ValidityData, CFR::FSValidityDateOff(date));
        }

        // признак фискализированности ККМ
        m_Fiscalized = data[7];
    }

    if (processCommand(CShtrihOnlineFR::Commands::FS::GetVersion, &data) && (data.size() > 19)) {
        setDeviceParameter(CDeviceData::FS::Version,
                           QString("%1, type %2")
                               .arg(clean(data.mid(3, 16)).data())
                               .arg(data[19] ? "serial" : "debug"));
    }

    using namespace CShtrihOnlineFR::FRParameters;

    if (getFRParameter(FFDFR, data)) {
        m_FFDFR = EFFD::Enum(int(data[0]));
        m_FFDFS = m_FFDFR;
    }

    if (getFRParameter(SerialNumber, data)) {
        m_Serial = CFR::serialToString(data);
    }

    if (getFRParameter(INN, data))
        m_INN = CFR::INNToString(data);
    if (getFRParameter(RNM, data))
        m_RNM = CFR::RNMToString(data);

#define SET_LCONFIG_FISCAL_FIELD(aName)                                                            \
    QString aName##Log = m_FFData.getTextLog(CFR::FiscalFields::aName);                             \
    if (getFRParameter(aName, data)) {                                                             \
        m_FFEngine.setConfigParameter(CFiscalSDK::aName, m_Codec->toUnicode(data));                  \
        QString value = m_FFEngine.getConfigParameter(CFiscalSDK::aName, data).toString();          \
        toLog(LogLevel::Normal,                                                                    \
              m_DeviceName +                                                                        \
                  QString(": Add %1 = \"%2\" to config data").arg(aName##Log).arg(value));         \
    } else                                                                                         \
        toLog(LogLevel::Error,                                                                     \
              m_DeviceName + QString(": Failed to add %1 to config data").arg(aName##Log));

#define SET_BCONFIG_FISCAL_FIELD(aName)                                                            \
    if (value & CShtrihOnlineFR::OperationModeMask::aName) {                                       \
        m_FFEngine.setConfigParameter(CFiscalSDK::aName, 1);                                        \
        toLog(LogLevel::Normal,                                                                    \
              m_DeviceName + QString(": Add %1 = 1 to config data")                                 \
                                .arg(m_FFData.getTextLog(CFR::FiscalFields::aName)));               \
    }

    SET_LCONFIG_FISCAL_FIELD(FTSURL);
    SET_LCONFIG_FISCAL_FIELD(OFDURL);
    SET_LCONFIG_FISCAL_FIELD(OFDName);
    SET_LCONFIG_FISCAL_FIELD(LegalOwner);
    SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
    SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

    if (getFRParameter(OperationModes, data) && !data.isEmpty()) {
        char value = data[0];

        SET_BCONFIG_FISCAL_FIELD(LotteryMode);
        SET_BCONFIG_FISCAL_FIELD(GamblingMode);
        SET_BCONFIG_FISCAL_FIELD(ExcisableUnitMode);
        SET_BCONFIG_FISCAL_FIELD(InAutomateMode);
    }

    if (m_FFEngine.getConfigParameter(CFiscalSDK::InAutomateMode).toInt() && m_OperatorPresence) {
        m_WrongFiscalizationSettings = true;
        toLog(LogLevel::Error,
              m_DeviceName + ": There is \"In automate mode\" flag and operator is present.");
    }

    removeDeviceParameter(CDeviceData::SDCard);

    if ((m_Model != CShtrihFR::Models::ID::MStarTK2) && getFRParameter(SD::Status, data)) {
        if (data[0] == CShtrihOnlineFR::SDNotConnected) {
            setDeviceParameter(CDeviceData::SDCard, CDeviceData::NotConnected);
        } else if (data[0]) {
            setDeviceParameter(CDeviceData::Error, uchar(data[0]), CDeviceData::SDCard);
        } else {
            auto getSDData = [&](const CShtrihFR::FRParameters::SData &aData) -> uint {
                if (!getFRParameter(aData, data))
                    return 0;
                return revert(data).toHex().toUInt(0, 16);
            };

            QString SDCardData =
                QString("cluster %1 KB, space %2 MB (%3 MB free), io errors %4, retry count %5")
                    .arg(getSDData(SD::ClusterSize) / 1024)
                    .arg(getSDData(SD::TotalSize) / CShtrihOnlineFR::SectorsInMB)
                    .arg(getSDData(SD::FreeSize) / CShtrihOnlineFR::SectorsInMB)
                    .arg(getSDData(SD::IOErrors))
                    .arg(getSDData(SD::RetryCount));

            setDeviceParameter(CDeviceData::SDCard, SDCardData);
        }
    }

    m_CanProcessZBuffer = m_ModelData.ZBufferSize && containsDeviceParameter(CDeviceData::SDCard);

    checkDateTime();
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::checkFirmwareUpdatingData(const CShtrihFR::FRParameters::SData &aData,
                                                      int aValue,
                                                      const QString &aLogData,
                                                      bool &aNeedReboot) {
    QByteArray data;

    if (!getFRParameter(aData, data)) {
        toLog(LogLevel::Error,
              QString("%1: Failed to get %2 updating data").arg(m_DeviceName).arg(aLogData));
        return false;
    }

    int value = revert(data).toHex().toUShort(0, 16);

    if (value != aValue) {
        toLog(LogLevel::Normal,
              QString("%1: Need reboot due to %2 updating data").arg(m_DeviceName).arg(aLogData));
        aNeedReboot = true;

        if (!setFRParameter(aData, aValue)) {
            toLog(LogLevel::Error,
                  QString("%1: Failed to set %2 updating data").arg(m_DeviceName).arg(aLogData));
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::enableFirmwareUpdating() {
    using namespace CShtrihOnlineFR::FRParameters;
    using namespace CShtrihOnlineFR::FirmwareUpdating;

    bool needReboot = false;

    if (!checkFirmwareUpdatingData(FirmwareUpdating::Working, Working, "working", needReboot) ||
        !checkFirmwareUpdatingData(FirmwareUpdating::Interval, Interval, "interval", needReboot) ||
        !checkFirmwareUpdatingData(FirmwareUpdating::Enabling, Enabling, "enabling", needReboot) ||
        !checkFirmwareUpdatingData(FirmwareUpdating::Single, Single, "single", needReboot)) {
        return false;
    }

    return true;
    // до выяснения причин невозможности установки параметров обновления прошивки
    /*
    m_NeedReboot = needReboot && (m_Model == CShtrihFR::Models::ID::PayVKP80KFA);

    return !needReboot || m_NeedReboot || reboot();
    */
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::reboot() {
    QVariantMap portConfig = m_IOPort->getDeviceConfiguration();
    TResult result =
        processCommand(CShtrihOnlineFR::Commands::Service, CShtrihOnlineFR::Service::Reboot);

    CommandResult::TResults errors = CommandResult::TResults()
                                     << CommandResult::Port << CommandResult::Transport
                                     << CommandResult::Protocol << CommandResult::Driver
                                     << CommandResult::Device;

    if (!errors.contains(result)) {
        m_IOPort->close();
        SleepHelper::msleep(CShtrihOnlineFR::RebootPause);

        if (m_IOPort->getType() == EPortTypes::TCP) {
            portConfig.insert(CHardware::Port::OpeningTimeout,
                              CShtrihOnlineFR::TCPReopeningTimeout);
            m_IOPort->setDeviceConfiguration(portConfig);
        }

        result = m_IOPort->open();

        m_IOPort->setDeviceConfiguration(portConfig);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> void ShtrihOnlineFRBase<T>::setErrorFlags() {
    if (m_ErrorData->value(m_LastError).type == FRError::EType::FS) {
        m_FSError = true;
    }
}

//--------------------------------------------------------------------------------
template <class T> void ShtrihOnlineFRBase<T>::checkSalesName(QString &aName) {
    aName = aName.leftJustified(CShtrihFR::FixedStringSize, QChar(ASCII::Space), false);
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::sale(const SUnitData &aUnitData, EPayOffTypes::Enum aPayOffType) {
    if (m_ModelData.date < CShtrihOnlineFR::MinFWDate::V2) {
        return ShtrihFRBase<T>::sale(aUnitData, aPayOffType);
    }

    char taxIndex = char(m_TaxData[aUnitData.VAT].group);
    char section = (aUnitData.section == -1) ? CShtrihFR::SectionNumber : char(aUnitData.section);
    QByteArray sum = getHexReverted(aUnitData.sum, 5, 2);
    QString name = aUnitData.name;

    QByteArray commandData;
    commandData.append(char(aPayOffType));                       // тип операции (1054)
    commandData.append(getHexReverted(1, 6, 6));                 // количество (1023)
    commandData.append(sum);                                     // цена (1079)
    commandData.append(sum);                                     // сумма операций (1023)
    commandData.append(CShtrihOnlineFR::FiscalTaxData);          // налог (1102..1107)
    commandData.append(taxIndex);                                // налоговая ставка
    commandData.append(section);                                 // отдел
    commandData.append(char(aUnitData.payOffSubjectMethodType)); // признак способа расчета (1214)
    commandData.append(char(aUnitData.payOffSubjectType));       // признак предмета расчета (1212)
    commandData.append(m_Codec->from_Unicode(name));               // наименование товара (1030)

    if (!processCommand(CShtrihOnlineFR::Commands::FS::Sale, commandData)) {
        toLog(LogLevel::Error,
              QString("%1: Failed to sale for %2 (%3, VAT = %4), feed, cut and exit")
                  .arg(m_DeviceName)
                  .arg(aUnitData.sum, 0, 'f', 2)
                  .arg(name)
                  .arg(aUnitData.VAT));
        return false;
    }

    return setOFDParametersOnSale(aUnitData);
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::closeDocument(double aSum, EPayTypes::Enum aPayType) {
    if (m_ModelData.date < CShtrihOnlineFR::MinFWDate::V2) {
        return ShtrihFRBase<T>::closeDocument(aSum, aPayType);
    }

    QByteArray commandData;

    for (int i = 1; i <= CShtrihOnlineFR::PayTypeQuantity; ++i) {
        double sum = (i == m_PayTypeData[aPayType].value) ? aSum : 0;
        commandData.append(getHexReverted(sum, 5, 2)); // сумма
    }

    char taxSystem = char(m_FFEngine.getConfigParameter(CFiscalSDK::TaxSystem).toInt());

    commandData.append(ASCII::NUL);                          // Округление до рубля
    commandData.append(CShtrihOnlineFR::ClosingFiscalTaxes); // налоги
    commandData.append(taxSystem);                           // СНО

    if (!processCommand(CShtrihOnlineFR::Commands::FS::CloseDocument, commandData)) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to close document, feed, cut and exit");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::perform_Fiscal(const QStringList &aReceipt,
                                          const SPaymentData &aPaymentData,
                                          quint32 *aFDNumber) {
    // СНО ставится либо параметром системной таблицы, либо параметром команды закрытия чека.
    m_OFDFiscalFields.remove(CFR::FiscalFields::TaxSystem);

    if ((m_ModelData.date < CShtrihOnlineFR::MinFWDate::V2) ||
        (m_Model == CShtrihFR::Models::ID::MStarTK2)) {
        char taxSystem = char(aPaymentData.taxSystem);

        if ((taxSystem != ETaxSystems::None) && (m_TaxSystems.size() != 1) &&
            !setFRParameter(CShtrihOnlineFR::FRParameters::TaxSystem, taxSystem)) {
            toLog(LogLevel::Error,
                  QString("%1: Failed to set taxation system %2 (%3)")
                      .arg(m_DeviceName)
                      .arg(toHexLog(taxSystem))
                      .arg(CFR::TaxSystems[taxSystem]));
            return false;
        }
    }

    bool setCustom_FieldsOK = false;
    char setCustom_Fields = ASCII::NUL;
    QByteArray data;

    if (isFS36() && (aPaymentData.taxSystem == ETaxSystems::Main) &&
        getFRParameter(CShtrihOnlineFR::FRParameters::SetCustom_Fields, data) && !data.isEmpty()) {
        QStringList noPayLog;

        foreach (const SUnitData &unitData, aPaymentData.unitDataList) {
            if (CFR::PayOffSubjectTypesNo36.contains(unitData.payOffSubjectType)) {
                noPayLog << QString("%1 (%2, %3)")
                                .arg(unitData.name)
                                .arg(unitData.sum)
                                .arg(CFR::PayOffSubjectTypes[char(unitData.payOffSubjectType)]);
            }
        }

        setCustom_Fields = data[0];
        char newSetCustom_Fields =
            setCustom_Fields | CShtrihOnlineFR::FRParameters::DontSendPayOffSubjectType;

        if (!noPayLog.isEmpty() && (setCustom_Fields != newSetCustom_Fields)) {
            QString log =
                m_DeviceName +
                QString(": Failed to make fiscal document due to cannot sale unit(s): %1, because ")
                    .arg(noPayLog.join("; "));

            if (m_FFDFS > EFFD::F105) {
                toLog(LogLevel::Error,
                      log + QString("FFD FS = %1 > 1.05").arg(CFR::FFD[m_FFDFS].description));
                return false;
            } else if (!m_FiscalServerPresence) {
                toLog(LogLevel::Error, log + "it is not fiscal server");
                return false;
            } else if (!setFRParameter(CShtrihOnlineFR::FRParameters::SetCustom_Fields,
                                       newSetCustom_Fields)) {
                toLog(LogLevel::Error, log + "impossible to set custom fields data");
                return false;
            } else {
                setCustom_FieldsOK = true;
            }
        }
    }

    if (!ShtrihFRBase<T>::perform_Fiscal(aReceipt, aPaymentData)) {
        return false;
    }

    if (aFDNumber && processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data) &&
        (data.size() >= 33)) {
        *aFDNumber = revert(data.mid(29, 4)).toHex().toUInt(0, 16);
    }

    if (setCustom_FieldsOK &&
        !setFRParameter(CShtrihOnlineFR::FRParameters::SetCustom_Fields, setCustom_Fields)) {
        m_PPTaskList.append([&]() {
            setFRParameter(CShtrihOnlineFR::FRParameters::SetCustom_Fields, setCustom_Fields);
        });
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::getFiscalFields(quint32 aFDNumber,
                                            TFiscalPaymentData &aFPData,
                                            TComplexFiscalPaymentData &aPSData) {
    if (!processCommand(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData,
                        getHexReverted(aFDNumber, 4))) {
        return false;
    }

    TGetFiscalTLVData getFiscalTLVData = [&](QByteArray &aData) -> TResult {
        TResult result = processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &aData);
        aData = aData.mid(3);
        return result;
    };

    return processFiscalTLVData(getFiscalTLVData, &aFPData, &aPSData);
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::canForceStatusBufferEnable() {
    return (m_Model == CShtrihFR::Models::ID::ShtrihM01F) ||
           (m_Model == CShtrihFR::Models::ID::ShtrihM02F) ||
           (m_Model == CShtrihFR::Models::ID::ShtrihMini01F) ||
           (m_Model == CShtrihFR::Models::ID::ShtrihLight02F);
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::execZReport(bool aAuto) {
    QVariantMap outData;

    if (!prepareZReport(aAuto, outData)) {
        return false;
    }

    bool result = checkNotPrinting(aAuto, true);

    if (!result && aAuto) {
        m_NeedCloseSession = m_NeedCloseSession || (m_Mode == CShtrihFR::InnerModes::SessionExpired);

        return false;
    }

    if (m_OperatorPresence) {
        processCommand(CShtrihFR::Commands::SectionReport);
        waitForPrintingEnd(true, CShtrihOnlineFR::MaxWaitForPrintingSectionReport);
        processCommand(CShtrihFR::Commands::TaxReport);
        waitForPrintingEnd(true, CShtrihOnlineFR::MaxWaitForPrintingTaxReport);
        processCommand(CShtrihFR::Commands::CashierReport);
        waitForPrintingEnd(true, CShtrihOnlineFR::MaxWaitForPrintingCashierReport);
    }

    m_NeedCloseSession = false;
    result = processCommand(CShtrihFR::Commands::ZReport);

    if (result) {
        m_ZBufferOverflow = false;
        SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
        result = waitForChangeZReportMode();
    }

    if (getLongStatus()) {
        m_NeedCloseSession = m_Mode == CShtrihFR::InnerModes::SessionExpired;
    }

    if (result) {
        emit FRSessionClosed(outData);
    }

    toLog(result ? LogLevel::Normal : LogLevel::Error,
          result ? "ShtrihFR: Z-report is successfully processed"
                 : "ShtrihFR: error in processing Z-report");

    checkNotPrinting(false, true);

    return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::processAnswer(const QByteArray &aCommand, char aError) {
    switch (aError) {
    case CShtrihOnlineFR::Errors::FSOfflineEnd: {
        m_ProcessingErrors.push_back(aError);

        m_FSOfflineEnd = true;

        break;
    }
    case CShtrihOnlineFR::Errors::NeedZReport: {
        m_ProcessingErrors.push_back(aError);

        return execZReport(true);
    }
    case CShtrihOnlineFR::Errors::WrongFSState: {
        m_ProcessingErrors.push_back(aError);

        if (aCommand[0] != CShtrihFR::Commands::OpenFRSession) {
            return (getSessionState() == ESessionState::Closed) && openFRSession();
        }

        break;
    }
    }

    bool result = ShtrihFRBase<T>::processAnswer(aCommand, aError);

    if (!m_ProcessingErrors.isEmpty() &&
        (m_ProcessingErrors.last() == CShtrihFR::Errors::BadModeForCommand) && getLongStatus() &&
        (m_Mode == CShtrihFR::InnerModes::DataEjecting)) {
        TGetFiscalTLVData getFiscalTLVData = [&](QByteArray &aData) -> TResult {
            TResult result =
                processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &aData);
            aData = aData.mid(3);
            return result;
        };

        result = processTLVData(getFiscalTLVData);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::setTLV(int aField, bool aForSale) {
    bool result;

    if (!m_FFEngine.checkFiscalField(aField, result)) {
        return result;
    }

    QString fieldLog;
    QByteArray commandData = m_FFEngine.getTLVData(aField, &fieldLog);
    QByteArray command = aForSale ? CShtrihOnlineFR::Commands::FS::SetOFDParameterLinked
                                  : CShtrihOnlineFR::Commands::FS::SetOFDParameter;

    if (!processCommand(command, commandData)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to set " + fieldLog);
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ShtrihOnlineFRBase<T>::openSession() {
    TStatusCodes statusCodes;

    if (!ProtoShtrihFR<T>::getStatus(statusCodes) ||
        statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter)) {
        waitForPrintingEnd();
        SleepHelper::msleep(CShtrihFR::Pause::Cutting);
    }

    checkNotPrinting(true);

    bool result = processCommand(CShtrihFR::Commands::OpenFRSession);
    waitForPrintingEnd();

    checkNotPrinting();

    return result;
}

//--------------------------------------------------------------------------------
