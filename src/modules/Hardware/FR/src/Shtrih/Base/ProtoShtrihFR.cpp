/* @file Прото-ФР семейства Штрих на COM-порту. */

#include "ProtoShtrihFR.h"

#include <QtCore/qmath.h>

using namespace SDK::Driver;
using namespace ProtocolUtils;

// TODO: реализовать тег Bold командой Печать жирной строки (12h)

//--------------------------------------------------------------------------------
template class ProtoShtrihFR<ShtrihSerialFRBase>;
template class ProtoShtrihFR<ShtrihTCPFRBase>;

//--------------------------------------------------------------------------------
template <class T> ProtoShtrihFR<T>::ProtoShtrihFR() {
    // кодек
    m_Codec = CodecByName[CHardware::Codepages::Win1251];

    // параметры семейства ФР
    m_NextReceiptProcessing = false;
    m_Mode = CShtrihFR::InnerModes::Work;
    m_Submode = CShtrihFR::InnerSubmodes::PaperOn;
    m_Type = CShtrihFR::Types::NoType;
    m_Model = CShtrihFR::Models::ID::NoModel;
    m_NonNullableAmount = 0;
    m_FontNumber = CShtrihFR::Fonts::Default;
    m_TransportTimeout = CShtrihFR::Timeouts::Transport;
    m_NeedReceiptProcessingOnCancel = false;

    // налоги
    m_TaxData.add(18, 1);
    m_TaxData.add(10, 2);
    m_TaxData.add(0, 0);

    // данные команд
    m_CommandData.add(CShtrihFR::Commands::GetModelInfo, CShtrihFR::Timeouts::Default, false);

    // данные ошибок
    m_UnprocessedErrorData.add(CShtrihFR::Commands::GetFRParameter,
                              CShtrihFR::Errors::WrongParametersInCommand);
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::getPrintingSettings() {
    QByteArray commandData(1, m_FontNumber);
    QByteArray answer;

    if (!processCommand(CShtrihFR::Commands::GetFontSettings, commandData, &answer) ||
        (answer.size() <= 4)) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to getting font settings");
        return false;
    }

    ushort paperWidth = revert(answer.mid(2, 2)).toHex().toUShort(0, 16);
    ushort letterSize = ushort(answer[4]);
    m_LineSize = paperWidth / letterSize;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::updateParameters() {
    processDeviceData();
    setFRParameters();

    if (!getPrintingSettings()) {
        return false;
    }

    if (!isFiscal()) {
        return true;
    }

    TLoadSectionName loadSectionName = [&](int aIndex, QByteArray &aData) -> bool {
        return getFRParameter(CShtrihFR::FRParameters::SectionName, aData, char(++aIndex));
    };

    if (m_OperatorPresence && !loadSectionNames(loadSectionName)) {
        return false;
    }

    return checkTaxes();
}

//---------------------------------------------------------------------------
template <class T> QDateTime ProtoShtrihFR<T>::getDateTime() {
    QByteArray data;

    if (getLongStatus(data)) {
        QString dateTime = hexToBCD(data.mid(25, 6)).insert(4, "20");

        return QDateTime::fromString(dateTime, CShtrihFR::DateTimeFormat);
    }

    return QDateTime();
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::setTaxValue(TVAT aVAT, int aGroup) {
    if (!setFRParameter(CShtrihFR::FRParameters::Taxes::Value,
                        getHexReverted(int(aVAT * 100), 2),
                        char(aGroup))) {
        toLog(LogLevel::Error,
              QString("ShtrihFR: Failed to set tax value %1% (%2 tax group)")
                  .arg(aVAT, 5, 'f', 2, ASCII::Zero)
                  .arg(aGroup));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::checkTax(TVAT aVAT, CFR::Taxes::SData &aData) {
    if (!aData.group) {
        return true;
    }

    char group = char(aData.group);
    QByteArray rawValue;
    QByteArray rawDescription;

    if (!getFRParameter(CShtrihFR::FRParameters::Taxes::Value, rawValue, group) ||
        !getFRParameter(CShtrihFR::FRParameters::Taxes::Description, rawDescription, group)) {
        return false;
    }

    int value = int(aVAT * 100);
    int FRValue = revert(rawValue).toHex().toUShort(0, 16);
    aData.deviceVAT = FRValue / 100;

    if (value != FRValue) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Wrong tax value = %1% (%2 tax group), need %3%")
                                .arg(FRValue / 100.0, 5, 'f', 2, ASCII::Zero)
                                .arg(aData.group)
                                .arg(aVAT, 5, 'f', 2, ASCII::Zero));

        if (!setTaxValue(aVAT, group)) {
            return false;
        }
    }

    QString description = aData.description;
    QString FRDescription = m_Codec->toUnicode(rawDescription.replace(ASCII::NUL, ""));

    if ((description != FRDescription) &&
        !setFRParameter(CShtrihFR::FRParameters::Taxes::Description,
                        description.leftJustified(57, QChar(ASCII::NUL)),
                        group,
                        true)) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to set description for tax value %1% (%2 tax group)")
                                .arg(aVAT, 5, 'f', 2, ASCII::Zero)
                                .arg(aData.group));
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoShtrihFR<T>::performFiscal(const QStringList &aReceipt,
                                     const SPaymentData &aPaymentData,
                                     quint32 * /*aFDNumber*/) {
    EDocumentState::Enum documentState = getDocumentState();

    if ((documentState == EDocumentState::Error) ||
        ((documentState == EDocumentState::Opened) &&
         !processCommand(CShtrihFR::Commands::CancelDocument))) {
        return false;
    }

    if (!processReceipt(aReceipt, false) || !openDocument(aPaymentData.payOffType)) {
        return false;
    }

    bool result = true;

    foreach (auto unitData, aPaymentData.unitDataList) {
        result = result && sale(unitData, aPaymentData.payOffType);
    }

    result = result && setOFDParameters() &&
             closeDocument(getTotalAmount(aPaymentData), aPaymentData.payType);

    waitForPrintingEnd(true);
    // getDocumentState() == EDocumentState::Opened;

    if (!result && aPaymentData.back() && (m_LastError == CShtrihFR::Errors::NotEnoughMoney)) {
        emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);
    }

    if (!result) {
        cancelFiscal();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::cancelFiscal() {
    return processCommand(CShtrihFR::Commands::CancelDocument) &&
           (!m_NeedReceiptProcessingOnCancel || receiptProcessing());
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray data = performStatus(aStatusCodes, CShtrihFR::Commands::GetLongStatus, 16);

    if (data == CFR::Result::Fail) {
        return false;
    } else if (data != CFR::Result::Error) {
        m_Mode = data[15] & CShtrihFR::InnerModes::Mask;
        m_Submode = data[16];

        ushort flags = revert(data.mid(13, 2)).toHex().toUShort(0, 16);
        appendStatusCodes(flags, aStatusCodes);
    } else if (m_LastError == CShtrihFR::Errors::Cutter) {
        aStatusCodes.insert(PrinterStatusCode::Error::Cutter);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void ProtoShtrihFR<T>::appendStatusCodes(ushort aFlags, TStatusCodes &aStatusCodes) {
    using namespace PrinterStatusCode;

    // ошибки чековой ленты, выявленные весовым и оптическим датчиками соответственно
    bool paperWeightSensor =
        (~aFlags & CShtrihFR::Statuses::WeightSensor::NoChequePaper) && isPaperWeightSensor();
    bool paperOpticalSensor =
        (~aFlags & CShtrihFR::Statuses::OpticalSensor::NoChequePaper) && isPaperOpticalSensor();

    if (paperWeightSensor || paperOpticalSensor) {
        aStatusCodes.insert(Error::PaperEnd);
        toLog(LogLevel::Error,
              QString("ShtrihFR: Paper tape error, report %1")
                  .arg((paperWeightSensor && paperOpticalSensor)
                           ? "both optical & weight sensors"
                           : (paperWeightSensor ? "weight sensor" : "optical sensor")));
    }

    // рычаг чековой ленты
    if ((~aFlags & CShtrihFR::Statuses::PaperLeverNotDropped) && isPaperLeverExist()) {
        aStatusCodes.insert(DeviceStatusCode::Error::MechanismPosition);
        toLog(LogLevel::Error, "ShtrihFR: Paper lever error");
    }

    // крышка корпуса
    if ((aFlags & CShtrihFR::Statuses::CoverNotClosed) && isCoverSensor()) {
        aStatusCodes.insert(DeviceStatusCode::Error::CoverIsOpened);
    }

    if ((m_Submode == CShtrihFR::InnerSubmodes::PaperEndPassive) ||
        (m_Submode == CShtrihFR::InnerSubmodes::PaperEndActive)) {
        aStatusCodes.insert(Error::PaperEnd);
    }
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::isPaperWeightSensor() const {
    bool result =
        !((m_Type == CShtrihFR::Types::Printer) ||
          (m_Type == CShtrihFR::Types::KKM && ((m_Model == CShtrihFR::Models::ID::ShtrihElvesFRK) ||
                                              (m_Model == CShtrihFR::Models::ID::Yarus01K) ||
                                              (m_Model == CShtrihFR::Models::ID::Yarus02K) ||
                                              (m_Model == CShtrihFR::Models::ID::ShtrihKioskFRK_2) ||
                                              (m_Model == CShtrihFR::Models::ID::NeoService) ||

                                              (m_Model == CShtrihFR::Models::ID::PayOnline01FA) ||
                                              (m_Model == CShtrihFR::Models::ID::PayVKP80KFA))));

    bool weightSensorsEnabled =
        !containsConfigParameter(CHardware::Printer::Settings::PaperWeightSensors) ||
        (getConfigParameter(CHardware::Printer::Settings::PaperWeightSensors).toString() ==
         CHardwareSDK::Values::Use);

    return result && weightSensorsEnabled;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::isPaperOpticalSensor() const {
    return !((m_Type == CShtrihFR::Types::KKM) &&
             ((m_Model == CShtrihFR::Models::ID::ATOLElvesMiniFRF) ||
              (m_Model == CShtrihFR::Models::ID::ATOLFelixRF)));
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::isPaperLeverExist() const {
    return (m_Type == CShtrihFR::Types::Printer) ||
           ((m_Type == CShtrihFR::Types::KKM) &&
            ((m_Model == CShtrihFR::Models::ID::ShtrihFRF) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihFRK) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihElvesFRK) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihFRFKazah) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihFRFBelorus) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihComboFRK) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihKioskFRK) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihKioskFRK_2) ||
             (m_Model == CShtrihFR::Models::ID::Yarus01K) ||
             (m_Model == CShtrihFR::Models::ID::NeoService) ||

             (m_Model == CShtrihFR::Models::ID::PayOnline01FA) ||
             (m_Model == CShtrihFR::Models::ID::PayVKP80KFA) ||
             (m_Model == CShtrihFR::Models::ID::ShtrihFR01F)));
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::isCoverSensor() const {
    return (m_Type == CShtrihFR::Types::KKM) &&
           ((m_Model == CShtrihFR::Models::ID::ShtrihFRF) ||
            (m_Model == CShtrihFR::Models::ID::ShtrihFRK) ||
            (m_Model == CShtrihFR::Models::ID::ShtrihFRFKazah) ||
            (m_Model == CShtrihFR::Models::ID::ShtrihFRFBelorus) ||
            (m_Model == CShtrihFR::Models::ID::Shtrih950K) ||
            (m_Model == CShtrihFR::Models::ID::ShtrihMiniFRK) ||
            (m_Model == CShtrihFR::Models::ID::ShtrihMini01F) ||
            (m_Model == CShtrihFR::Models::ID::NeoService) ||

            (m_Model == CShtrihFR::Models::ID::ShtrihFR01F));
}

//--------------------------------------------------------------------------------
template <class T>
TResult ProtoShtrihFR<T>::execCommand(const QByteArray &aCommand,
                                      const QByteArray &aCommandData,
                                      QByteArray *aAnswer) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);
    m_Protocol.setTransportTimeout(m_TransportTimeout);

    QByteArray commandData = aCommand;

    if (m_CommandData[aCommand].password) {
        QByteArray passwordData =
            QByteArray(1, CShtrihFR::AdminPassword).leftJustified(4, ASCII::NUL);
        commandData.append(passwordData);
    };

    m_LastError = 0;

    commandData += aCommandData;
    QByteArray answer;

    int repeatCount = 0;

    do {
        if (repeatCount) {
            toLog(LogLevel::Normal, m_DeviceName + QString(": iteration %1").arg(repeatCount + 1));
        }

        m_LastCommandResult =
            m_Protocol.processCommand(commandData, answer, m_CommandData[aCommand].timeout);

        if (!m_LastCommandResult) {
            return m_LastCommandResult;
        }

        if (aAnswer) {
            *aAnswer = answer;
        }

        if (answer.size() < CShtrihFR::MinAnswerDataSize) {
            toLog(LogLevel::Error,
                  QString("ShtrihFR: Answer data size = %1, need min %2")
                      .arg(answer.size())
                      .arg(CShtrihFR::MinAnswerDataSize));
            m_LastCommandResult = CommandResult::Answer;
        }

        QByteArray answerCommand = answer.left(aCommand.size());

        if (aCommand != answerCommand) {
            toLog(LogLevel::Error,
                  QString("ShtrihFR: Invalid answer command = 0x%1, need = 0x%2.")
                      .arg(answerCommand.toHex().toUpper().data())
                      .arg(aCommand.toHex().toUpper().data()));
            m_LastCommandResult = CommandResult::Answer;
        }
    } while (!m_LastCommandResult && aAnswer && (repeatCount++ < CShtrihFR::MaxRepeatPacket));

    if (!m_LastCommandResult) {
        return aAnswer ? m_LastCommandResult : CommandResult::OK;
    }

    int errorPosition = aCommand.size();
    m_LastError = answer[errorPosition];
    m_LastCommand = aCommand;

    if (!m_LastError) {
        return CommandResult::OK;
    }

    toLog(LogLevel::Error, m_DeviceName + ": Error: " + m_ErrorData->value(m_LastError).description);

    if (!isErrorUnprocessed(aCommand, m_LastError)) {
        setErrorFlags();
    }

    if (isNotError(aCommand[0])) {
        m_LastError = 0;

        return CommandResult::OK;
    }

    if (!m_ProcessingErrors.isEmpty() && (m_ProcessingErrors.last() == m_LastError)) {
        return CommandResult::Device;
    }

    char error = m_LastError;

    if (isErrorUnprocessed(aCommand, error) || !processAnswer(aCommand, error)) {
        m_LastError = error;
        m_LastCommand = aCommand;

        return CommandResult::Device;
    }

    TResult result = processCommand(aCommand, aCommandData, aAnswer);

    if (result) {
        m_ProcessingErrors.pop_back();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::isNotError(char aCommand) {
    if ((m_LastError == CShtrihFR::Errors::BadModeForCommand) &&
        (aCommand == CShtrihFR::Commands::CancelDocument) &&
        (getDocumentState() == EDocumentState::Closed)) {
        toLog(LogLevel::Normal, "ShtrihFR: Fiscal document already closed");
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
template <class T> double ProtoShtrihFR<T>::getAmountInCash() {
    QByteArray data;

    if (!getRegister(CShtrihFR::Registers::TotalCashSum, data)) {
        return -1;
    }

    bool OK;
    double result = revert(data).toHex().toULongLong(&OK, 16) / 100.0;

    return OK ? result : -1;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::processPayout(double aAmount) {
    return processCommand(CShtrihFR::Commands::Encashment, getHexReverted(aAmount, 5, 2));
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::printLine(const QByteArray &aString) {
    QDateTime beginning = QDateTime::currentDateTime();

    QByteArray commandData;
    commandData.append(CShtrihFR::PrintOnChequeTape);
    commandData.append(m_FontNumber);
    commandData.append(aString.leftJustified(CShtrihFR::FixedStringSize, ASCII::Space));

    TResult result = processCommand(CShtrihFR::Commands::PrintString, commandData);

    if ((result != CommandResult::Port) && (result != CommandResult::NoAnswer) &&
        m_ModelData.linePrintingTimeout) {
        int pause =
            m_ModelData.linePrintingTimeout - int(beginning.msecsTo(QDateTime::currentDateTime()));

        if (pause > 0) {
            toLog(LogLevel::Debug,
                  m_DeviceName + QString(": Pause after printing line = %1 ms").arg(pause));
            SleepHelper::msleep(pause);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::cut() {
    QByteArray commandData(1, CShtrihFR::FullCutting);

    if (!processCommand(CShtrihFR::Commands::Cut, commandData)) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to cut");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> EDocumentState::Enum ProtoShtrihFR<T>::getDocumentState() {
    if (!getLongStatus()) {
        return EDocumentState::Error;
    }

    return (m_Mode == CShtrihFR::InnerModes::DocumentOpened) ? EDocumentState::Opened
                                                            : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::openDocument(EPayOffTypes::Enum aPayOffType) {
    char FDType = CShtrihFR::PayOffType::Data[aPayOffType].FDType;

    if (!processCommand(CShtrihFR::Commands::OpenDocument, QByteArray(1, FDType))) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to open document, feed, cut and exit");
        return false;
    }

    return getDocumentState() == EDocumentState::Opened;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::closeDocument(double aSum, EPayTypes::Enum aPayType) {
    QByteArray commandData;

    for (int i = 1; i <= CShtrihFR::PayTypeQuantity; ++i) {
        double sum = (i == m_PayTypeData[aPayType].value) ? aSum : 0;
        commandData.append(getHexReverted(sum, 5, 2)); // сумма
    }

    commandData.append(getHexReverted(0, 2, 2));       // скидка
    commandData.append(CShtrihFR::ClosingFiscalTaxes); // налоги
    commandData.append(CShtrihFR::UnitName);           // текст продажи

    if (!processCommand(CShtrihFR::Commands::CloseDocument, commandData)) {
        toLog(LogLevel::Error, "ShtrihFR: Failed to close document, feed, cut and exit");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void ProtoShtrihFR<T>::checkSalesName(QString &aName) {
    if ((aName.size() > m_LineSize) && processReceipt(QStringList() << aName, false)) {
        aName.clear();
    }

    aName = aName.leftJustified(CShtrihFR::FixedStringSize, QChar(ASCII::Space), true);
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoShtrihFR<T>::sale(const SUnitData &aUnitData, EPayOffTypes::Enum aPayOffType) {
    int taxIndex = m_TaxData[aUnitData.VAT].group;
    QString name = aUnitData.name;
    char section = (aUnitData.section == -1) ? CShtrihFR::SectionNumber : char(aUnitData.section);
    checkSalesName(name);

    QByteArray commandData;
    commandData.append(getHexReverted(1, 5, 3));             // количество
    commandData.append(getHexReverted(aUnitData.sum, 5, 2)); // сумма
    commandData.append(section);                             // отдел
    commandData.append(getHexReverted(taxIndex, 4));         // налоги
    commandData.append(m_Codec->fromUnicode(name));           // текст продажи

    char command = CShtrihFR::PayOffType::Data[aPayOffType].command;

    if (!processCommand(command, commandData)) {
        toLog(LogLevel::Error,
              QString("%1: Failed to sale for %2 (%3, VAT = %4), feed, cut and exit")
                  .arg(m_DeviceName)
                  .arg(aUnitData.sum, 0, 'f', 2)
                  .arg(name)
                  .arg(aUnitData.VAT));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::performZReport(bool /*aPrintDeferredReports*/) {
    return execZReport(false);
}

//--------------------------------------------------------------------------------
template <class T> void ProtoShtrihFR<T>::parseDeviceData(const QByteArray &aData) {
    // данные прошивки ФР
    CShtrihFR::SSoftInfo FRInfo;
    FRInfo.version = aData.mid(3, 2).insert(1, ASCII::Dot);
    FRInfo.build = revert(aData.mid(5, 2)).toHex().toUShort(0, 16);
    QString FRDate = hexToBCD(aData.mid(7, 3)).insert(4, "20");
    FRInfo.date = QDate::fromString(FRDate, CFR::DateFormat);

    m_OldFirmware = (m_ModelData.build && (FRInfo.build != m_ModelData.build)) ||
                   ((m_ModelData.date < QDate::currentDate()) && (FRInfo.date < m_ModelData.date));

    setDeviceParameter(CDeviceData::Version, FRInfo.version, CDeviceData::Firmware, true);
    setDeviceParameter(CDeviceData::Build, FRInfo.build, CDeviceData::Firmware);
    setDeviceParameter(
        CDeviceData::Date, FRInfo.date.toString(CFR::DateLogFormat), CDeviceData::Firmware);

    setDeviceParameter(CDeviceData::FR::FreeReregistrations, uchar(aData[41]));
}

//--------------------------------------------------------------------------------
template <class T> void ProtoShtrihFR<T>::processDeviceData() {
    QByteArray data;

    if (getLongStatus(data)) {
        parseDeviceData(data);
    }

    QByteArray answer;

    if (processCommand(CShtrihFR::Commands::GetModelInfo, &answer)) {
        QString protocol = hexToBCD(answer.mid(4, 2), ASCII::LF).simplified().insert(1, ASCII::Dot);
        uchar languageId = uchar(answer[7]);

        if (m_Model >= 0) {
            setDeviceParameter(CDeviceData::ModelNumber, m_Model);
        }

        setDeviceParameter(CDeviceData::ProtocolVersion, protocol);
        setDeviceParameter(CDeviceData::FR::Language, CShtrihFR::Languages[languageId]);
    }
}

//--------------------------------------------------------------------------------
template <class T> void ProtoShtrihFR<T>::setFRParameters() {
    if (!CShtrihFR::FRParameters::Fields.data().contains(m_Model)) {
        toLog(LogLevel::Normal,
              QString("ShtrihFR: Cannot set any fields for the device with model Id %1 as no data "
                      "of system tables")
                  .arg(m_Model));
        return;
    }

    QString nullingSumInCash = getConfigParameter(CHardwareSDK::FR::NullingSumInCash).toString();

    if (nullingSumInCash != CHardwareSDK::Values::Auto) {
        // 0. автообнуление денежной наличности при закрытии смены
        setFRParameter(m_Parameters.autoNulling, nullingSumInCash == CHardwareSDK::Values::Use);
    }

    // 1. Печать рекламного текста (шапка чека, клише) - да
    //    на самом деле это доступность всей таблицы 4 - шапка чека + реклама в конце
    // setFRParameter(m_Parameters.documentCapEnable, true);

    // 2. Печать текстовых строк на ленте операционного журнала (контрольной) - нет
    setFRParameter(m_Parameters.printOnControlTape, false);

    // 3. Печать только на чековой ленте - да
    setFRParameter(m_Parameters.printOnChequeTapeOnly, true);

    // 4. Печать необнуляемой суммы (на Z-отчете) - да
    setFRParameter(m_Parameters.printNotNullingSum, true);

    // 5. Отрезка чека после завершения печати - да
    QByteArray data;

    if (getFRParameter(m_Parameters.cutting, data)) {
        char cutting = (m_Model == CShtrihFR::Models::ID::PayVKP80KFA)
                           ? CShtrihFR::FRParameters::PartialCutting
                           : CShtrihFR::FRParameters::FullCutting;

        if (data[0] != cutting) {
            setFRParameter(m_Parameters.cutting, cutting);
            m_NeedReboot = m_Model == CShtrihFR::Models::ID::PayVKP80KFA;
        }
    }

    // 6. Печать заголовка чека (шапка чека, клише) - в конце чека
    //    на самом деле это фискальные параметры чека - серийник ККМ, ИНН дилера и др.
    setFRParameter(m_Parameters.documentCapPlace, true);

    // 7. Печать единичного количества - нет
    setFRParameter(m_Parameters.printOneAmount, false);

    // 8. Промотка ленты перед отрезкой чека - нет, проматываем сами
    setFRParameter(m_Parameters.feedBeforeCutting, false);

    // 9. Использование весовых датчиков при отсутствии контроля бумаги - да
    setFRParameter(m_Parameters.weightSensorEnable, isPaperWeightSensor());

    // 10. Длинный Z-отчет с гашением - да
    setFRParameter(m_Parameters.ZReportType, m_OperatorPresence);

    // 11. Начисление налогов - на каждую операцию в чеке/налог вычисляется в ФР.
    setFRParameter(m_Parameters.taxesCalculation, CShtrihFR::TaxForEachOperationInFR);

    // 12. Печать налогов - налоговые ставки, оборот, названия, накопления.
    setFRParameter(m_Parameters.taxesPrinting, CShtrihFR::PrintAllTaxData);

    // 13. Количество строк рекламного текста - 0, не нужно оно.
    //     тут надо было бы поставить 0, но это даст ошибку 33h - значение вне диапазона.
    //     пишем 2, это также может вызвать ошибку, если прошивка старая.
    setFRParameter(m_Parameters.documentCapAmount, CShtrihFR::MaxAdvertStringsSize);

    // 14. Печать клише (шапка чека) - да
    setFRParameter(m_Parameters.printDocumentCap, true);

    // 15. Отрезка чека при открытом чеке - да
    //     если фискальный чек на каком-то этапе обломился, надо выдать юзеру то, что есть
    setFRParameter(m_Parameters.cuttingWithOpenedDocument, true);
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoShtrihFR<T>::setFRParameter(const CShtrihFR::FRParameters::SData &aData,
                                      const QVariant &aValue,
                                      char aSeries,
                                      bool aCleanLogValue) {
    if (m_ProcessingErrors.contains(CShtrihFR::Errors::RAM)) {
        return false;
    }

    if (!CShtrihFR::FRParameters::Fields.data().contains(m_Model)) {
        toLog(LogLevel::Normal,
              m_DeviceName +
                  ": Failed to set fields due to no data in field specification for model Id = " +
                  QString::number(m_Model));
        return true;
    }

    if (aData.field == CShtrihFR::FRParameters::NA) {
        toLog(LogLevel::Normal,
              m_DeviceName +
                  QString(": Failed to set field %1 for table %2 due to it is not available")
                      .arg(aData.description, m_Parameters.getMaxNADescriptionSize())
                      .arg(aData.table));
        return true;
    }

    QByteArray commandData;
    commandData.append(char(aData.table));
    commandData.append(getHexReverted(aSeries, 2));
    commandData.append(char(aData.field));

    if (aValue.typeId() == QMetaType::QByteArray)
        commandData.append(aValue.toByteArray());
    else if (aValue.typeId() == QMetaType::QString)
        commandData.append(m_Codec->fromUnicode(aValue.toString()));
    else
        commandData.append(char(aValue.toInt()));

    QString logValue = aValue.toString();

    if (aCleanLogValue) {
        logValue = clean(logValue);
    }

    if (!processCommand(CShtrihFR::Commands::SetFRParameter, commandData)) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to set %1 = %2").arg(aData.log(aSeries)).arg(logValue));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoShtrihFR<T>::getFRParameter(const CShtrihFR::FRParameters::SData &aData,
                                      QByteArray &aValue,
                                      char aSeries) {
    if (m_ProcessingErrors.contains(CShtrihFR::Errors::RAM)) {
        return false;
    }

    if (!CShtrihFR::FRParameters::Fields.data().contains(m_Model)) {
        toLog(LogLevel::Normal,
              m_DeviceName +
                  ": Failed to get fields due to no data in field specification for model Id = " +
                  QString::number(m_Model));
        return true;
    }

    if (aData.field == CShtrihFR::FRParameters::NA) {
        toLog(LogLevel::Normal,
              m_DeviceName +
                  QString(": Failed to get field %1 for table %2 due to it is not available")
                      .arg(aData.description, m_Parameters.getMaxNADescriptionSize())
                      .arg(aData.table));
        return true;
    }

    QByteArray commandData;
    commandData.append(char(aData.table));
    commandData.append(getHexReverted(aSeries, 2));
    commandData.append(char(aData.field));
    QByteArray data;

    if (!processCommand(CShtrihFR::Commands::GetFRParameter, commandData, &data)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get " + aData.log(aSeries));
        return false;
    }

    aValue = data.mid(2);

    return !aValue.isEmpty();
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::processXReport() {
    if ((m_Model == CShtrihFR::Models::ID::NeoService) &&
        (getSessionState() == ESessionState::Closed)) {
        return false;
    }

    if (!processCommand(CShtrihFR::Commands::XReport)) {
        return false;
    }

    return waitForChangeXReportMode();
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::waitForChangeXReportMode() {
    QTime clockTimer;
    clockTimer.start();

    do {
        QTime clock = QTime::currentTime();

        // 3.1. запрашиваем статус
        TStatusCodes statusCodes;

        if (getStatus(statusCodes)) {
            // 3.2. анализируем режим и подрежим, если печать X-отчета окончена - выходим
            if (!statusCodes.intersect(CFR::XReportFiscalErrors).isEmpty()) {
                toLog(LogLevel::Error, "ShtrihFR: Failed to print X-Report, exit!");
                return false;
            }

            if ((m_Submode == CShtrihFR::InnerSubmodes::PaperEndPassive) ||
                (m_Submode == CShtrihFR::InnerSubmodes::PaperEndActive)) {
                // 3.3. подрежим - закончилась бумага
                return false;
            }
            // если режим или подрежим - печать или печать отчета или X-отчета, то
            else if ((m_Mode == CShtrihFR::InnerModes::PrintFullZReport) ||
                     (m_Mode == CShtrihFR::InnerModes::PrintEKLZReport) ||
                     (m_Submode == CShtrihFR::InnerSubmodes::PrintingFullReports) ||
                     (m_Submode == CShtrihFR::InnerSubmodes::Printing)) {
                toLog(LogLevel::Normal, "ShtrihFR: service X-report process, wait...");
            } else if ((m_Mode == CShtrihFR::InnerModes::SessionOpened) ||
                       (m_Mode == CShtrihFR::InnerModes::SessionClosed) ||
                       (m_Mode == CShtrihFR::InnerModes::SessionExpired) ||
                       (m_Submode == CShtrihFR::InnerSubmodes::PaperOn)) {
                // 3.3. режим - тот, который ожидаем, если X-отчет допечатался, все хорошо
                return true;
            } else {
                // 3.4. режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
                toLog(LogLevel::Error,
                      QString("ShtrihFR: X-report, unknown mode.submode = %1.%2")
                          .arg(int(m_Mode))
                          .arg(int(m_Submode)));
                return false;
            }

            // спим до периода опроса
            int sleepTime =
                CShtrihFR::Interval::ReportPoll - abs(clock.msecsTo(QTime::currentTime()));

            if (sleepTime > 0) {
                SleepHelper::msleep(sleepTime);
            }
        }
    } while (clockTimer.elapsed() < CShtrihFR::Timeouts::MaxXReportNoAnswer);

    toLog(LogLevel::Normal, "ShtrihFR: Timeout for X report.");

    // вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
    return false;
}

//--------------------------------------------------------------------------------
template <class T>
QVariantMap ProtoShtrihFR<T>::getSessionOutData(const QByteArray &aLongStatusData) {
    toLog(LogLevel::Normal, m_DeviceName + ": Getting session out data");

    QVariantMap result;
    result.insert(CFiscalPrinter::Serial, m_Serial);
    result.insert(CFiscalPrinter::RNM, m_RNM);

    result.insert(CFiscalPrinter::FRDateTime, getDateTime().toString(CFR::DateTimeLogFormat));

    QDateTime systemDateTime = QDateTime::currentDateTime();
    result.insert(CFiscalPrinter::SystemDateTime, systemDateTime.toString(CFR::DateTimeLogFormat));

    ushort lastClosedSession = 1 + revert(aLongStatusData.mid(36, 2)).toHex().toUShort(0, 16);
    result.insert(CFiscalPrinter::ZReportNumber, lastClosedSession);

    double paymentAmount = 0;
    QByteArray data;

    if (getRegister(CShtrihFR::Registers::PaymentAmount, data)) {
        paymentAmount = revert(data).toHex().toInt(0, 16) / 100.0;
        result.insert(CFiscalPrinter::PaymentAmount, paymentAmount);
    }

    if (getRegister(CShtrihFR::Registers::PaymentCount, data)) {
        int paymentCount = revert(data).toHex().toInt(0, 16);
        result.insert(CFiscalPrinter::PaymentCount, paymentCount);
    }

    if (m_Fiscalized) {
        m_NonNullableAmount += paymentAmount;
        result.insert(CFiscalPrinter::NonNullableAmount, m_NonNullableAmount);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::prepareZReport(bool aAuto, QVariantMap &aOutData) {
    QByteArray data;

    if (!getLongStatus(data)) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to get status therefore failed to process %1Z-report.")
                      .arg(aAuto ? "auto-" : ""));
        m_NeedCloseSession = m_NeedCloseSession || (m_Mode == CShtrihFR::InnerModes::SessionExpired);

        return false;
    }

    aOutData = getSessionOutData(data);

    toLog(LogLevel::Normal,
          m_DeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::execZReport(bool aAuto) {
    QVariantMap outData;

    if (!prepareZReport(aAuto, outData)) {
        return false;
    }

    bool success = processCommand(CShtrihFR::Commands::ZReport);

    if (getLongStatus()) {
        m_NeedCloseSession = m_Mode == CShtrihFR::InnerModes::SessionExpired;
    }

    if (success) {
        emit FRSessionClosed(outData);
    }

    toLog(success ? LogLevel::Normal : LogLevel::Error,
          success ? "ShtrihFR: Z-report is successfully processed"
                  : "ShtrihFR: error in processing Z-report");

    return success;
}

//--------------------------------------------------------------------------------
template <class T> TResult ProtoShtrihFR<T>::getLongStatus() {
    QByteArray data;

    return getLongStatus(data);
}

//--------------------------------------------------------------------------------
template <class T> TResult ProtoShtrihFR<T>::getLongStatus(QByteArray &aData) {
    TResult result = processCommand(CShtrihFR::Commands::GetLongStatus, &aData);

    if (CORRECT(result) && result && (aData.size() > 16)) {
        m_Mode = aData[15] & CShtrihFR::InnerModes::Mask;
        m_Submode = aData[16];
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> ESessionState::Enum ProtoShtrihFR<T>::getSessionState() {
    QByteArray data;

    if (!getLongStatus(data)) {
        return ESessionState::Error;
    }

    if (m_Mode == CShtrihFR::InnerModes::SessionClosed)
        return ESessionState::Closed;
    if (m_Mode == CShtrihFR::InnerModes::SessionExpired)
        return ESessionState::Expired;

    return ESessionState::Opened;
}

//--------------------------------------------------------------------------------
template <class T>
void ProtoShtrihFR<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                         const TStatusCollection &aOldStatusCollection) {
    // если нет ошибок и нужно продолжать печать - продолжаем
    if ((m_Submode == CShtrihFR::InnerSubmodes::NeedContinuePrinting) && isDeviceReady(false)) {
        toLog(LogLevel::Normal, "ShtrihFR: The paper is in the printer, continue printing...");
        processCommand(CShtrihFR::Commands::ExtentionPrinting);
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
bool ProtoShtrihFR<T>::getRegister(const CShtrihFR::TRegisterId &aRegister,
                                   QByteArray &aFRRegister) {
    CShtrihFR::Registers::SData data = CShtrihFR::Registers::Data.getInfo(aRegister);

    toLog(LogLevel::Normal,
          QString("ShtrihFR: Begin to get FR register %1, type %2 (%3)")
              .arg(aRegister.first)
              .arg(data.typeDescription)
              .arg(data.description));

    char command = (aRegister.second == CShtrihFR::ERegisterType::Money)
                       ? CShtrihFR::Commands::GetMoneyRegister
                       : CShtrihFR::Commands::GetOperationalRegister;
    QByteArray answer;
    bool processSuccess = processCommand(command, QByteArray(1, aRegister.first), &answer);

    aFRRegister = answer.mid(3);

    return processSuccess;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::processAnswer(const QByteArray &aCommand, char aError) {
    switch (aError) {
    case CShtrihFR::Errors::DocumentIsOpened: {
        m_ProcessingErrors.push_back(aError);

        return processCommand(CShtrihFR::Commands::CancelDocument);
    }
    //--------------------------------------------------------------------------------
    case CShtrihFR::Errors::BadModeForCommand:
    case CShtrihFR::Errors::BadModeForField: {
        m_ProcessingErrors.push_back(aError);

        if (getLongStatus()) {
            toLog(LogLevel::Normal,
                  m_DeviceName +
                      QString(": mode = %1, submode = %2").arg(int(m_Mode)).arg(int(m_Submode)));

            switch (m_Mode) {
            case CShtrihFR::InnerModes::SessionExpired: {
                return execZReport(true);
            }
            case CShtrihFR::InnerModes::SessionOpened: {
                bool trueCommand = (aCommand[0] == CShtrihFR::Commands::Encashment) ||
                                   (aCommand[0] == CShtrihFR::Commands::SetFRParameter);

                return trueCommand && execZReport(true);
            }
            case CShtrihFR::InnerModes::DocumentOpened: {
                return processCommand(CShtrihFR::Commands::CancelDocument);
            }
            }
        }

        return false;
    }
    //--------------------------------------------------------------------------------
    case CShtrihFR::Errors::NeedZReport: {
        m_ProcessingErrors.push_back(aError);

        return execZReport(true);
    }
    //--------------------------------------------------------------------------------
    case CShtrihFR::Errors::NeedExtentionPrinting: {
        m_ProcessingErrors.push_back(aError);

        return processCommand(CShtrihFR::Commands::ExtentionPrinting);
    }
    //--------------------------------------------------------------------------------
    case CShtrihFR::Errors::NeedWaitForPrinting: {
        m_ProcessingErrors.push_back(aError);

        return waitForPrintingEnd();
    }
    }

    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool ProtoShtrihFR<T>::waitForPrintingEnd(bool aCanBeOff, int aTimeout) {
    QTime clockTimer;
    clockTimer.start();

    do {
        QTime clock = QTime::currentTime();

        // 1. запрашиваем статус
        if (!getLongStatus()) {
            if (aCanBeOff) {
                continue;
            } else {
                return false;
            }
        }

        if ((m_Submode == CShtrihFR::InnerSubmodes::PaperEndPassive) ||
            (m_Submode == CShtrihFR::InnerSubmodes::PaperEndActive)) {
            // закончилась бумага
            return false;
        }
        // если режим или подрежим - печать или печать отчета или Z-отчета или режим - исходный, то
        else if ((m_Mode == CShtrihFR::InnerModes::Work) ||
                 (m_Submode == CShtrihFR::InnerSubmodes::Printing) ||
                 (m_Submode == CShtrihFR::InnerSubmodes::PrintingFullReports)) {
            // 2. подрежим - идет печать. Ждем
            toLog(LogLevel::Normal, "ShtrihFR: printing, wait...");
        } else {
            // 4. подрежим - документ допечатался, все хорошо
            return true;
        }

        // спим до периода опроса
        int sleepTime =
            CShtrihFR::Interval::WaitForPrintingEnd - abs(clock.msecsTo(QTime::currentTime()));

        if (sleepTime > 0) {
            SleepHelper::msleep(sleepTime);
        }
    } while (clockTimer.elapsed() < aTimeout);

    // 6. вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
    return false;
}

//--------------------------------------------------------------------------------
