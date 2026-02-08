/* @file Принтеры семейства ПРИМ. */

#include "PrimFRBase.h"

#include <Hardware/FR/PrimFR.h>

#include "Hardware/Printers/POSPrinterData.h"

using namespace PrinterStatusCode;
using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
template TResult PrimFRBase::processCommand<uchar>(char, int, const QString &, uchar &);
template TResult PrimFRBase::processCommand<ushort>(char, int, const QString &, ushort &);
template TResult PrimFRBase::processCommand<uint>(char, int, const QString &, uint &);
template TResult PrimFRBase::processCommand<int>(char, int, const QString &, int &);

template void PrimFRBase::loadDeviceData<uchar>(
    const CPrimFR::TData &, const QString &, const QString &, int, const QString &);
template void PrimFRBase::loadDeviceData<ushort>(
    const CPrimFR::TData &, const QString &, const QString &, int, const QString &);
template void PrimFRBase::loadDeviceData<uint>(
    const CPrimFR::TData &, const QString &, const QString &, int, const QString &);

//--------------------------------------------------------------------------------
PrimFRBase::PrimFRBase() : m_Mode(EFRMode::Fiscal) {
    // теги
    m_TagEngine = Tags::PEngine(new CPrimFR::TagEngine());

    // данные моделей
    m_DeviceName = CPrimFR::DefaultModelName;
    m_Models = CPrimFR::CommonModels();
    m_Model = CPrimFR::Models::Unknown;
    m_Offline = true;

    setConfigParameter(CHardware::Printer::Commands::Cutting, CPOSPrinter::Command::Cut);

    // типы оплаты
    m_PayTypeData.add(EPayTypes::Cash, 0);
    m_PayTypeData.add(EPayTypes::EMoney, 1);
    m_PayTypeData.add(EPayTypes::PrePayment, 2);

    // команды
    m_CommandTimouts.append(CPrimFR::Commands::SetFRParameters, 3 * 1000);
    m_CommandTimouts.append(CPrimFR::Commands::SetEjectorAction, 3 * 1000);
    m_CommandTimouts.append(CPrimFR::Commands::ZReport, 10 * 1000);
    m_CommandTimouts.append(CPrimFR::Commands::AFD, 10 * 1000);

    // ошибки
    m_ErrorData = PErrorData(new CPrimFR::Errors::Data());
    m_ExtraErrorData = PExtraErrorData(new CPrimFR::Errors::ExtraData());

    // налоги
    m_TaxData.add(0, 0);
    m_TaxData.add(10, 4);
    m_TaxData.add(18, 5);

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // preferable for work
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);   // default
    m_PortParameters[EParameters::BaudRate].append(
        EBaudRate::BR4800); // default after resetting to zero
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

    m_PortParameters[EParameters::RTS].clear();
    m_PortParameters[EParameters::RTS].append(ERTSControl::Disable);

    m_PortParameters[EParameters::DTR].clear();
    m_PortParameters[EParameters::DTR].append(EDTRControl::Handshake);

    m_PortParameters[EParameters::Parity].append(EParity::No);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::updateParameters() {
    if ((!m_OperatorPresence && !checkParameters()) || !checkControlSettings()) {
        return false;
    }

    if (!isFiscal()) {
        return true;
    }

    if (!checkTaxes()) {
        return false;
    }

    CPrimFR::TData commandData;
    QString payment = getConfigParameter(CHardware::FR::Strings::Payment).toString();
    commandData << m_Codec->fromUnicode(payment) << " " << " ";

    // устанавливаем названия строк фискального чека
    if (!processCommand(CPrimFR::Commands::SetFDTypeNames, commandData)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to set fiscal receipt type name");
        return false;
    }

    // GetLastCVCNumber -> EDocument, для получения предыдущего чека

    return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkParameters() {
    CPrimFR::TData answer;

    if (!processCommand(CPrimFR::Commands::GetFRParameters, &answer) || (answer.size() < 8)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to get FR parameters");
        return false;
    }

    ushort FRParameter1 = QString(answer[5]).toUShort(0, 16);
    ushort FRParameter2 = QString(answer[6]).toUShort(0, 16);
    ushort FRParameter3 = QString(answer[7]).toUShort(0, 16);

    ushort parameter1 = CPrimFR::Parameter1;
    ushort parameter2 = CPrimFR::Parameter2;
    ushort parameter3 = qToBigEndian(getParameter3());

    using namespace CHardware::Printer;

    if (m_IsOnline) {
        parameter1 &= CPrimFR::Parameter1Mask;
        parameter1 |= ~CPrimFR::Parameter1Mask & FRParameter1;
    }

    QString nullingSumInCash = getConfigParameter(CHardwareSDK::FR::NullingSumInCash).toString();

    if (nullingSumInCash == CHardwareSDK::Values::Auto) {
        parameter2 &= ~CPrimFR::NullingSumInCashMask;
        parameter2 |= FRParameter2 & CPrimFR::NullingSumInCashMask;
    } else if (nullingSumInCash == CHardwareSDK::Values::Use) {
        parameter2 |= CPrimFR::NullingSumInCashMask;
    } else if (nullingSumInCash == CHardwareSDK::Values::NotUse) {
        parameter2 &= ~CPrimFR::NullingSumInCashMask;
    }

    if (m_OperatorPresence) {
        parameter2 |= CPrimFR::LongReportMask2;
    }

    QString printDocumentCap = getConfigParameter(Settings::DocumentCap).toString();
    parameter2 &= ~CPrimFR::NeedPrintFiscalCapMask;

    if ((printDocumentCap == CHardwareSDK::Values::Use) ||
        ((printDocumentCap == CHardwareSDK::Values::Auto) &&
         (FRParameter2 & CPrimFR::NeedPrintFiscalCapMask))) {
        parameter2 |= CPrimFR::NeedPrintFiscalCapMask;
    }

    if ((FRParameter1 == parameter1) && (FRParameter2 == parameter2) &&
        (FRParameter3 == parameter3)) {
        return true;
    }

    auto getCommandData = [](ushort aData) -> QByteArray {
        return QString("%1").arg(aData, 4, 16, QChar(ASCII::Zero)).toUpper().toLatin1();
    };
    CPrimFR::TData commandData = CPrimFR::TData()
                                 << getCommandData(
                                        CPrimFR::Parameter1) // т.к. старший байт - только 0-й бит
                                 << getCommandData(parameter2) << getCommandData(parameter3);

    if (!processCommand(CPrimFR::Commands::SetFRParameters, commandData)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to set FR parameters");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
ushort PrimFRBase::getParameter3() {
    return ushort(getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt());
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkControlSettings() {
    CPrimFR::TData answer;

    if (!processCommand(CPrimFR::Commands::GetFRControlSettings, &answer) || (answer.size() < 11)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to get control settings");
        return false;
    }

    if (bool(answer[10].toInt()) == CPrimFR::DateTimeInCommand) {
        return true;
    }

    CPrimFR::TData commandData = answer.mid(6);
    commandData[4] = int2ByteArray(CPrimFR::DateTimeInCommand);

    if (!processCommand(CPrimFR::Commands::FRControl, commandData)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to set control settings");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
QDateTime PrimFRBase::getDateTime() {
    QDateTime result;
    processCommand(CPrimFR::Commands::GetDateTime, 5, "FR date and time", result);

    if (result.isValid()) {
        QTime currentTime = QTime::currentTime();
        result = result.addSecs(currentTime.second()).addMSecs(currentTime.msec());
    }

    return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getTaxData(int aGroup, CPrimFR::Taxes::SData &aData) {
    CPrimFR::TData answer;

    if (!processCommand(
            CPrimFR::Commands::GetTaxRate, CPrimFR::TData() << int2ByteArray(aGroup), &answer) ||
        (answer.size() < 8)) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to get data for %1 tax group").arg(aGroup));
        return false;
    }

    bool groupOK;
    bool valueOK;
    int group = answer[5].toInt(&groupOK);
    aData.description = m_Codec->toUnicode(answer[6]);
    aData.value = TVAT(answer[7].toDouble(&valueOK));
    aData.extraData = answer.mid(8);

    if (!groupOK || !valueOK) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to parse data for %1 tax group").arg(aGroup));
        return false;
    }

    if (aData.extraData.isEmpty()) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to parse data for %1 tax group due to extra data is empty")
                      .arg(aGroup));
        return false;
    }

    if (group != aGroup) {
        toLog(LogLevel::Error,
              m_DeviceName + QString("tax group = %1, need %2").arg(group).arg(aGroup));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setTaxData(int aGroup, const CPrimFR::Taxes::SData &aData) {
    CPrimFR::TData commandData =
        CPrimFR::TData()
        << int2ByteArray(aGroup) << m_Codec->fromUnicode(aData.description)
        << QString("%1").arg(aData.value, 5, 'f', 2, QLatin1Char(ASCII::Zero)).toLatin1()
        << aData.extraData;

    if (!processCommand(CPrimFR::Commands::SetTaxRate, commandData)) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to set data for %1 tax group").arg(aGroup));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkTax(TVAT aVAT, CFR::Taxes::SData &aData) {
    CPrimFR::Taxes::SData taxData;

    if (!getTaxData(aData.group, taxData)) {
        return false;
    }

    QStringList log;
    LogLevel::Enum logLevel = LogLevel::Warning;

    if (taxData.value != aVAT) {
        log << QString("tax value = %1%, need %2%")
                   .arg(taxData.value, 5, 'f', 2, ASCII::Zero)
                   .arg(aVAT, 5, 'f', 2, ASCII::Zero);
        logLevel = LogLevel::Error;
    }

    if (taxData.description != aData.description) {
        log << QString("tax description = %1, need %2")
                   .arg(taxData.description)
                   .arg(aData.description);
    }

    if (log.isEmpty()) {
        return true;
    }

    aData.deviceVAT = taxData.value;

    toLog(logLevel,
          m_DeviceName +
              QString(": Wrong %1 for %2 tax group").arg(log.join("; ")).arg(aData.group));

    if (m_IsOnline && (taxData.value != aVAT)) {
        return false;
    }

    taxData.value = aVAT;
    taxData.description = aData.description;

    bool result = setTaxData(aData.group, taxData);

    return result || (logLevel == LogLevel::Warning);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::isConnected() {
    QByteArray answer;

    while (m_IOPort->read(answer, 5) && !answer.isEmpty()) {
    }

    TStatusCodes statusCodes;
    CPrimFR::TData answerData;

    if (!getStatusInfo(statusCodes, answerData) || (answerData.size() < 8)) {
        return false;
    }

    QByteArray softVersion = answerData[6];
    m_Model = CPrimFR::ModelNames[answerData[5].simplified()];

    if (softVersion.indexOf(CPrimFR::FirmarePRIM21_03) != -1) {
        m_Model = CPrimFR::Models::PRIM_21K_03;
    }

    setDeviceParameter(CDeviceData::Firmware, softVersion);

    CPrimFR::SModelParameters modelData = CPrimFR::ModelData[m_Model];
    m_Verified = modelData.verified;
    m_ModelCompatibility = m_Models.contains(m_Model);
    m_CanProcessZBuffer = !m_IsOnline && modelData.hasBuffer;
    m_DeviceName = modelData.name;

    setConfigParameter(CHardware::Printer::FeedingAmount, modelData.feed);

    return true;
}

//--------------------------------------------------------------------------------
QStringList PrimFRBase::getModelList() {
    return CPrimFR::getModelList(CPrimFR::CommonModels());
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::processCommand(char aCommand, CPrimFR::TData *aAnswer) {
    CPrimFR::TData commandData;

    return processCommand(aCommand, commandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::processCommand(char aCommand,
                                   const CPrimFR::TData &aCommandData,
                                   CPrimFR::TData *aAnswer) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QVariantMap configuration;
    configuration.insert(CHardware::Port::DeviceModelName, "PRIM");
    m_IOPort->setDeviceConfiguration(configuration);

    QByteArray commandData = QByteArray(1, aCommand).toHex().toUpper();
    CPrimFR::TData data;

    if (CPrimFR::Commands::DateTimeIn.contains(aCommand)) {
        QTime currentTime = QTime::currentTime();
        QString time = QString("%1%2")
                           .arg(currentTime.hour(), 2, 10, QLatin1Char(ASCII::Zero))
                           .arg(currentTime.minute(), 2, 10, QLatin1Char(ASCII::Zero));

        data << QDate::currentDate().toString("ddMMyy").toLatin1() << time.toLatin1();
    }

    data << aCommandData;

    foreach (auto dataItem, data) {
        commandData += CPrimFR::Separator + dataItem;
    }

    m_LastError = 0;
    QByteArray answer;
    TResult result = m_Protocol.processCommand(commandData, answer, m_CommandTimouts[aCommand]);

    if (CORRECT(result) || (result == CommandResult::Id)) {
        m_Mode = EFRMode::Fiscal;
    }

    if (m_Connected && (result == CommandResult::NoAnswer) && (answer.size() == 1) &&
        (~answer[0] & CPrimFR::CommandResultMask::PrinterMode)) {
        m_Mode = EFRMode::Printer;
        m_IOPort->write(QByteArray(2, ASCII::LF));
        performReceipt(QStringList() << CPrimFR::EndPrinterModeText, true);

        if (setMode(EFRMode::Fiscal)) {
            result = m_Protocol.processCommand(commandData, answer, m_CommandTimouts[aCommand]);
        }
    }

    if (aCommand == CPrimFR::Commands::SetFDTypeNames) {
        SleepHelper::msleep(CPrimFR::Pause::Programming);
    }

    CPrimFR::TData answerData;
    result = checkAnswer(result, answer, answerData);

    if (!result) {
        return result;
    }

    if (aAnswer) {
        *aAnswer = answerData;
    }

    char error = char(qToBigEndian(QString(answerData[3]).toUShort(0, 16)));
    m_LastError = error;
    m_LastCommand = QByteArray(1, aCommand);

    if (!error) {
        if (aCommand == CPrimFR::Commands::ZReport) {
            m_NeedCloseSession = false;
        }

        return CommandResult::OK;
    }

    if (!m_ProcessingErrors.isEmpty() && (m_ProcessingErrors.last() == error)) {
        return CommandResult::Device;
    }

    QString canAutoCloseSession = getConfigParameter(CHardware::FR::CanAutoCloseSession).toString();

    if ((error == CPrimFR::Errors::NeedZReport) && (m_Initialized == ERequestStatus::InProcess)) {
        if (canAutoCloseSession == CHardwareSDK::Values::Auto) {
            setConfigParameter(CHardware::FR::CanAutoCloseSession, CHardwareSDK::Values::NotUse);

            emit configurationChanged();
        } else if (canAutoCloseSession == CHardwareSDK::Values::NotUse) {
            return CommandResult::OK;
        }
    }

    if (isErrorUnprocessed(aCommand, error) || !processAnswer(error)) {
        m_LastError = error;
        m_LastCommand = QByteArray(1, aCommand);

        return CommandResult::Device;
    }

    result = processCommand(aCommand, aCommandData, aAnswer);

    if (result) {
        m_ProcessingErrors.pop_back();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T>
TResult PrimFRBase::processCommand(char aCommand, int aIndex, const QString &aLog, T &aResult) {
    CPrimFR::TData commandData;

    return processCommand(aCommand, commandData, aIndex, aLog, aResult);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PrimFRBase::processCommand(char aCommand,
                                   const CPrimFR::TData &aCommandData,
                                   int aIndex,
                                   const QString &aLog,
                                   T &aResult) {
    CPrimFR::TData data;
    TResult result = processCommand(aCommand, aCommandData, &data);

    if (!result) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to process command to get " + aLog);
        return result;
    }

    return parseAnswerData<T>(data, aIndex, aLog, aResult) ? CommandResult::OK
                                                           : CommandResult::Answer;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimFRBase::parseAnswerData(const CPrimFR::TData &aData,
                                 int aIndex,
                                 const QString &aLog,
                                 T &aResult) {
    if (aData.size() <= aIndex) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to parse %1 data due to answer size = %2, need %3 minimum")
                      .arg(aLog)
                      .arg(aData.size())
                      .arg(aIndex + 1));
        return false;
    }

    bool OK;
    QByteArray data = aData[aIndex];
    aResult = qToBigEndian(T(data.toLongLong(&OK, 16)));

    if ((data.size() != (sizeof(T) * 2)) || !OK) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to parse %1 data, answer = %2 (%3)")
                                .arg(aLog)
                                .arg(m_Codec->toUnicode(data))
                                .arg(data.toHex().data()));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <>
bool PrimFRBase::parseAnswerData<QDateTime>(const CPrimFR::TData &aData,
                                            int aIndex,
                                            const QString & /*aLog*/,
                                            QDateTime &aResult) {
    if (aData.size() <= ++aIndex) {
        toLog(
            LogLevel::Error,
            m_DeviceName +
                QString(
                    ": Failed to parse date and time data due to answer size = %1, need %2 minimum")
                    .arg(aData.size())
                    .arg(aIndex + 1));
        return false;
    }

    QByteArray data = aData[aIndex - 1] + aData[aIndex];
    aResult = QDateTime::fromString(data.insert(4, "20"), CPrimFR::FRDateTimeFormat);

    if (!aResult.isValid()) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to parse date and time data, answer = %1 (%2)")
                                .arg(m_Codec->toUnicode(data))
                                .arg(data.toHex().data()));
        return false;
    }

    return true;
};

//--------------------------------------------------------------------------------
ESessionState::Enum PrimFRBase::getSessionState() {
    ushort state;

    if (!processCommand(CPrimFR::Commands::GetDateTime, 2, "session state", state)) {
        return ESessionState::Error;
    }

    if (~state & CPrimFR::SessionOpened)
        return ESessionState::Closed;
    if (state & CPrimFR::SessionExpired)
        return ESessionState::Expired;

    return ESessionState::Opened;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum PrimFRBase::getDocumentState() {
    ushort state;

    if (!processCommand(CPrimFR::Commands::GetDateTime, 2, "document state", state)) {
        return EDocumentState::Error;
    }

    return (state & CPrimFR::DocumentMask) ? EDocumentState::Opened : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
TResult
PrimFRBase::checkAnswer(TResult aResult, const QByteArray &aAnswer, CPrimFR::TData &aAnswerData) {
    if (aResult == CommandResult::NoAnswer) {
        char commandResultAnswer;
        TResult result = m_Protocol.getCommandResult(commandResultAnswer);

        if (result == CommandResult::Port) {
            return CommandResult::Port;
        } else if (result == CommandResult::OK) {
            TStatusCodes statusCodes = parseRTStatus(0, commandResultAnswer);

            if (statusCodes.contains(PrinterStatusCode::Error::PrinterFR)) {
                toLog(LogLevel::Error, "FR printer is in error, going to offline mode and exiting");
                m_Offline = true;
            }

            if (statusCodes.contains(DeviceStatusCode::Warning::OperationError)) {
                toLog(LogLevel::Error, "Last command not identify");
            }
        }

        return CommandResult::NoAnswer;
    } else if (!aResult) {
        return aResult;
    }

    aAnswerData = aAnswer.split(CPrimFR::Separator);
    int size = aAnswerData.size();

    if (size < 5) {
        toLog(
            LogLevel::Error,
            QString(
                "Failed to process command because too few sections in answer = %1, need 5 minimum")
                .arg(size));
        return CommandResult::Answer;
    }

    ushort error = qToBigEndian(QString(aAnswerData[3]).toUShort(0, 16));
    char errorCode = char(error >> 0);
    char errorReason = char(error >> 8);

    if (errorCode) {
        FRError::SData errorData = m_ErrorData->value(errorCode);
        QString log = m_DeviceName + ": Error: " + errorData.description;

        if (errorData.extraData) {
            log += m_ExtraErrorData->value(errorCode, errorReason);
        }

        toLog(LogLevel::Error, log);
    } else if (errorReason == 1) {
        toLog(LogLevel::Warning, "PRIMFR: OK, but the document was not printed");
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::printLine(const QByteArray &aString) {
    if (!TSerialFRBase::printLine(aString)) {
        return false;
    }

    if (m_Model == CPrimFR::Models::PRIM_07K) {
        SleepHelper::msleep(CPrimFR::Pause::LinePrinting);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performReceipt(const QStringList &aReceipt, bool aProcessing) {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
    m_IOPort->setDeviceConfiguration(configuration);

    bool result = TSerialFRBase::processReceipt(aReceipt, aProcessing);

    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(m_IOMessageLogging));
    m_IOPort->setDeviceConfiguration(configuration);

    return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processReceipt(const QStringList &aReceipt, bool aProcessing) {
    if (!isPrintingNeed(aReceipt)) {
        return true;
    }

    if (!setMode(EFRMode::Printer)) {
        return false;
    }

    // TODO: лишняя команда для наследников, которые печатают через драйвер принтера
    m_IOPort->write(CPOSPrinter::Command::AlignLeft);
    bool result = performReceipt(aReceipt, aProcessing);

    setMode(EFRMode::Fiscal);

    return result;
}

//--------------------------------------------------------------------------------
void PrimFRBase::getRTStatuses(TStatusCodes &aStatusCodes) {
    aStatusCodes.clear();

    foreach (auto command, CPrimFR::ModelData[m_Model].statusData.keys()) {
        if (aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable)) {
            break;
        }

        aStatusCodes.unite(getRTStatus(command));
    }

    bool notAvailabled = aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);
    bool oldNotAvailabled = m_StatusCollection.isEmpty() ||
                            m_StatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
    bool error =
        std::find_if(aStatusCodes.begin(), aStatusCodes.end(), [&](int aCode) -> bool {
            return m_StatusCodesSpecification->value(aCode).warningLevel == EWarningLevel::Error;
        }) != aStatusCodes.end();

    m_Offline = notAvailabled || (!oldNotAvailabled && error);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getStatusInfo(TStatusCodes &aStatusCodes, CPrimFR::TData &aAnswer) {
    bool printerMode = m_Mode == EFRMode::Printer;

    if (m_Offline || printerMode) {
        getRTStatuses(aStatusCodes);
    }

    TResult result = processCommand(CPrimFR::Commands::GetKKMInfo, &aAnswer);
    printerMode = m_Mode == EFRMode::Printer;

    if (!m_Offline && !printerMode) {
        if (!CORRECT(result)) {
            if (!aAnswer.isEmpty() && !aAnswer[0].isEmpty()) {
                getRTStatuses(aStatusCodes);
            }

            return false;
        } else if (result == CommandResult::Device) {
            int statusCode = getErrorStatusCode(m_ErrorData->value(m_LastError).type);
            aStatusCodes.insert(statusCode);
        } else if (result == CommandResult::Answer) {
            aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
        }

        TStatusCodes commandStatus = getRTStatus(0);
        aStatusCodes.unite(commandStatus);
    }

    return !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getStatus(TStatusCodes &aStatusCodes) {
    if (m_NeedCloseSession) {
        aStatusCodes.insert(FRStatusCode::Error::NeedCloseSession);
    }

    if (m_Mode == EFRMode::Printer) {
        return true;
    }

    CPrimFR::TData answerData;

    if (!getStatusInfo(aStatusCodes, answerData)) {
        return false;
    }

    char fixedStatus = char(answerData[1].toUShort(0, 16));

    foreach (auto bit, CPrimFR::StatusInfo.data().keys()) {
        if (fixedStatus & (1 << bit)) {
            // aStatusCodes.insert(CPrimFR::StatusInfo[bit]);
        }
    }

    QByteArray printerStatuses = answerData[4];
    int byte = 0;

    while (byte < (printerStatuses.size() / 2)) {
        char status = char(printerStatuses.mid(byte * 2, 2).toUShort(0, 16));
        aStatusCodes += parseRTStatus(++byte, status);
    }

    return true;
}

//--------------------------------------------------------------------------------
int PrimFRBase::getVerificationCode() {
    int result;

    if (!processCommand(CPrimFR::Commands::GetLastCVCNumber, 5, "last CVC number", result)) {
        return 0;
    }

    return result;
}

//--------------------------------------------------------------------------------
void PrimFRBase::makeAFDReceipt(QStringList &aReceipt) {
    aReceipt = simplifyReceipt(aReceipt);

    Tags::TLexemeReceipt lexemeReceipt;
    makeLexemeReceipt(aReceipt, lexemeReceipt);
    aReceipt.clear();

    for (int i = 0; i < lexemeReceipt.size(); ++i) {
        QString line;

        for (int j = 0; j < lexemeReceipt[i].size(); ++j) {
            for (int k = 0; k < lexemeReceipt[i][j].size(); ++k) {
                line += lexemeReceipt[i][j][k].data;
            }
        }

        aReceipt << line;
    }

    if (!aReceipt.isEmpty()) {
        toLog(LogLevel::Normal, "Printing fiscal document, receipt:\n" + aReceipt.join("\n"));
    }

    for (int i = 0; i < aReceipt.size(); ++i) {
        QString line = aReceipt.takeAt(i--);

        while (!line.isEmpty()) {
            aReceipt.insert(++i, line.left(CPrimFR::MaxLengthGField));
            line = line.mid(CPrimFR::MaxLengthGField);
        }
    }
}

//--------------------------------------------------------------------------------
void PrimFRBase::setFiscalData(CPrimFR::TData &aCommandData,
                               CPrimFR::TDataList &aAdditionalAFDData,
                               const SPaymentData &aPaymentData,
                               int aReceiptSize) {
    QString depositing = getConfigParameter(CHardware::FR::Strings::Depositing).toString();
    QString serialNumber = getConfigParameter(CHardware::FR::Strings::SerialNumber).toString();
    QString documentNumber = getConfigParameter(CHardware::FR::Strings::DocumentNumber).toString();
    QString INN = getConfigParameter(CHardware::FR::Strings::INN).toString();
    QString date = getConfigParameter(CHardware::FR::Strings::Date).toString();
    QString time = getConfigParameter(CHardware::FR::Strings::Time).toString();
    QString amount = getConfigParameter(CHardware::FR::Strings::Amount).toString();

    QByteArray sum =
        QString::number(qRound64(getTotalAmount(aPaymentData) * 100.0) / 100.0, '0', 2).toLatin1();
    QString cashier = m_FFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
    QString operatorId = cashier.isEmpty() ? CPrimFR::OperatorID : cashier;

    // обязательные G-поля
    aCommandData << addGFieldToBuffer(aReceiptSize + 1, serialNumber.size() + 2) // серийный номер
                 << addGFieldToBuffer(aReceiptSize + 1,
                                      serialNumber.size() + 14 +
                                          documentNumber.size())         // номер документа
                 << addGFieldToBuffer(aReceiptSize + 3, date.size() + 2) // дата
                 << addGFieldToBuffer(aReceiptSize + 3, date.size() + 16 + time.size()) // время
                 << addGFieldToBuffer(aReceiptSize + 2, INN.size() + 2)                 // ИНН
                 << addGFieldToBuffer(aReceiptSize + 3, 7)
                 << m_Codec->fromUnicode(operatorId)                                // ID оператора
                 << addGFieldToBuffer(aReceiptSize + 4, amount.size() + 2) << sum; // сумма

    // произворльные G-поля
    aAdditionalAFDData << addArbitraryFieldToBuffer(aReceiptSize + 1, 1, serialNumber)
                       << addArbitraryFieldToBuffer(
                              aReceiptSize + 1, serialNumber.size() + 12, documentNumber)
                       << addArbitraryFieldToBuffer(aReceiptSize + 2, 1, INN)
                       << addArbitraryFieldToBuffer(aReceiptSize + 3, 1, date)
                       << addArbitraryFieldToBuffer(aReceiptSize + 3, date.size() + 15, time)
                       << addArbitraryFieldToBuffer(aReceiptSize + 4, 1, amount);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performFiscal(const QStringList &aReceipt,
                               const SPaymentData &aPaymentData,
                               quint32 * /*aFDNumber*/) {
    QStringList receipt(aReceipt);
    makeAFDReceipt(receipt);
    int receiptSize = receipt.size();

    int verificationCode = getVerificationCode();
    QByteArray FDType = CPrimFR::PayOffTypeData[aPaymentData.payOffType];
    QByteArray payTypeData = int2ByteArray(m_PayTypeData[aPaymentData.payType].value);

    CPrimFR::TData commandData = CPrimFR::TData()
                                 << FDType             // тип чека
                                 << payTypeData        // тип оплаты
                                 << CPrimFR::Font      // шрифт - не используется, по умолчанию
                                 << CPrimFR::Copies    // количество копий
                                 << CPrimFR::Copies    // количество копий по горизонтали
                                 << CPrimFR::Copies    // количество копий по вертикали
                                 << CPrimFR::CopуGrid  // шаг сетки копий по горизонтали
                                 << CPrimFR::CopуGrid  // шаг сетки копий по вертикали
                                 << CPrimFR::LineGrid; // шаг строк

    CPrimFR::TDataList additionalAFDData;
    setFiscalData(commandData, additionalAFDData, aPaymentData, receiptSize);

    for (int i = 0; i < additionalAFDData.size(); ++i) {
        commandData << additionalAFDData[i];
    }

    // терминальный чек
    for (int i = 0; i < receiptSize; ++i) {
        char font = m_IsOnline ? CPrimFR::FiscalFont::Narrow : CPrimFR::FiscalFont::Default;
        commandData << addArbitraryFieldToBuffer(i + 1, 1, receipt[i], font);
    }

    // количество полей
    commandData.insert(32, int2ByteArray(receiptSize + additionalAFDData.size()));

    bool result = processCommand(CPrimFR::Commands::AFD, commandData);

    m_PaperInPresenter = QDateTime::currentDateTime();

    int newVerificationCode = getVerificationCode();

    if (verificationCode && newVerificationCode) {
        return newVerificationCode > verificationCode;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processXReport() {
    return processCommand(CPrimFR::Commands::XReport);
}

//--------------------------------------------------------------------------------
double PrimFRBase::getAmountInCash() {
    CPrimFR::TData answer;
    // индексы для парсинга, меняться не будут. Зависят от мажорной версии прошивки, которая
    // привязана к версии ФФД
    int index = (m_FFDFR < EFFD::F105) ? 23 : 31;

    if (!processCommand(CPrimFR::Commands::EReport, &answer) || (answer.size() <= index)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to process E-Report");
        return -1;
    }

    bool OK;
    double result = answer[index].toDouble(&OK);

    return OK ? result : -1;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processPayout(double aAmount) {
    QByteArray data = QString::number(floor(aAmount * 100) / 100, '0', 2).toLatin1();
    CPrimFR::TData commandData = CPrimFR::TData() << data << CPrimFR::OperatorID;

    return processCommand(CPrimFR::Commands::Encashment, commandData);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setStartZReportNumber(int aNumber, const CPrimFR::TData &aExtraData) {
    toLog(LogLevel::Normal, "PrimPrinters: Set start Z-report number: " + QString::number(aNumber));

    CPrimFR::Taxes::SData taxData(0, CPrimFR::LastTaxRateName, aExtraData);
    taxData.extraData[0] =
        QString("%1").arg(aNumber, 5, 'f', 2, QLatin1Char(ASCII::Zero)).toLatin1();

    return setTaxData(CPrimFR::LastTaxRate, taxData);
}

//--------------------------------------------------------------------------------
int PrimFRBase::getStartZReportNumber(CPrimFR::TData &aExtraData) {
    toLog(LogLevel::Normal, "PrimPrinters: Get start Z-report number");
    CPrimFR::Taxes::SData taxData;

    if (!getTaxData(CPrimFR::LastTaxRate, taxData)) {
        return -1;
    }

    QString strName = taxData.description.simplified();
    QString strNumber = taxData.extraData[0];
    aExtraData = taxData.extraData;

    if (strName.toLower() != QString(CPrimFR::LastTaxRateName).toLower()) {
        toLog(LogLevel::Error,
              QString("PRIM: Tax rate name not valid: %1, need %2")
                  .arg(strName)
                  .arg(CPrimFR::LastTaxRateName));
        return -1;
    }

    bool OK;
    int startZReport = int(strNumber.toDouble(&OK));

    if (!OK) {
        toLog(LogLevel::Error,
              QString("PRIM: Failed to convert begin Z-report number: %1").arg(strNumber));
        return -1;
    }

    toLog(LogLevel::Normal, "Start Z-report number: " + QString::number(startZReport));

    return startZReport;
}

//--------------------------------------------------------------------------------
int PrimFRBase::getEndZReportNumber() {
    ushort result;

    if (!processCommand(CPrimFR::Commands::GetStatus, 7, "end Z-report number", result)) {
        return -1;
    }

    toLog(LogLevel::Normal, "End Z-report number: " + QString::number(result));

    return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::printDeferredZReport(int aNumber) {
    toLog(LogLevel::Normal,
          "PrimPrinters: try to print deferred Z-report #" + QString::number(aNumber));

    CPrimFR::TData commandData =
        CPrimFR::TData() << QString("%1")
                                .arg(qToBigEndian(ushort(aNumber)), 4, 16, QChar(ASCII::Zero))
                                .toUpper()
                                .toLatin1();

    if (!processCommand(CPrimFR::Commands::PrintDeferredZReports, commandData)) {
        toLog(LogLevel::Error,
              "PrimPrinters: Failed to print deferred Z-report #" + QString::number(aNumber));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::doZReport(bool aAuto) {
    CPrimFR::TData commandData;

    if (aAuto) {
        commandData << CPrimFR::DontPrintFD;
    }

    return processCommand(CPrimFR::Commands::ZReport, commandData);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::execZReport(bool aAuto) {
    toLog(LogLevel::Normal,
          m_DeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
    ESessionState::Enum sessionState = getSessionState();

    if (sessionState == ESessionState::Error)
        return false;
    else if (sessionState == ESessionState::Closed)
        return true;

    bool cannotAutoZReport =
        !m_CanProcessZBuffer ||
        (m_OperatorPresence && !getConfigParameter(CHardware::FR::ForcePerformZReport).toBool());

    if (aAuto && cannotAutoZReport && !m_IsOnline) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  (m_OperatorPresence
                       ? ": Failed to process auto-Z-report due to presence of the operator."
                       : ": has no Z-buffer, so it is impossible to perform auto-Z-report."));
        return false;
    }

    if (!doZReport(aAuto)) {
        toLog(LogLevel::Error, "PrimPrinters: Failed to process Z-report");
        return false;
    }

    m_NeedCloseSession = false;
    m_ProcessingErrors.removeAll(CPrimFR::Errors::NeedZReport);

    return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performZReport(bool aPrintDeferredReports) {
    bool ZReportOK = execZReport(false);

    int needPrintDeferred = aPrintDeferredReports && m_CanProcessZBuffer;
    int endZReport = needPrintDeferred ? getEndZReportNumber() : 0;

    if (!needPrintDeferred || (endZReport <= 0)) {
        return ZReportOK;
    }

    CPrimFR::TData extraData;
    int startZReport = getStartZReportNumber(extraData);

    if ((startZReport <= 0) || (startZReport > endZReport)) {
        startZReport = 1;
        setStartZReportNumber(startZReport, extraData);
    }

    toLog(LogLevel::Normal,
          QString("PrimPrinters: Begin printing deferred Z-reports from %1 to %2")
              .arg(startZReport)
              .arg(endZReport));
    bool deferred = true;

    for (int i = startZReport; i <= endZReport; ++i) {
        if (!printDeferredZReport(i) || !setStartZReportNumber(i, extraData)) {
            deferred = false;
        }
    }

    if (deferred) {
        toLog(LogLevel::Normal, "PrimPrinters: clear SKL.");

        if (!processCommand(CPrimFR::Commands::ClearZBuffer)) {
            toLog(LogLevel::Error, "PrimPrinters: Failed to clear SKL.");
            return false;
        }
    }

    return ZReportOK || (aPrintDeferredReports && deferred);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setMode(EFRMode::Enum aMode) {
    if (m_Mode == aMode) {
        return true;
    }

    if (aMode == EFRMode::Printer) {
        if (!processCommand(CPrimFR::Commands::SetPrinterMode)) {
            toLog(LogLevel::Error, "PrimPrinters: Failed to set printer mode");
            return false;
        }
    } else if (aMode == EFRMode::Fiscal) {
        QByteArray answer;
        TResult result = m_Protocol.execCommand(CPrimFR::Commands::SetFiscalMode,
                                               answer,
                                               CPrimFR::SetFiscalModeTimeout,
                                               EPrimFRCommandConditions::PrinterMode);

        CPrimFR::TData answerData;
        result = checkAnswer(result, answer, answerData);

        if (!result) {
            toLog(LogLevel::Error, "PrimPrinters: Failed to set fiscal mode");
            return false;
        }
    }

    m_Mode = aMode;

    return true;
}

//--------------------------------------------------------------------------------
CPrimFR::TData PrimFRBase::addGFieldToBuffer(int aX, int aY, int aFont) {
    return CPrimFR::TData()
           << QString("%1")
                  .arg(qToBigEndian(unsigned short(aX)), 4, 16, QLatin1Char(ASCII::Zero))
                  .toLatin1() // позиция реквизита по X
           << QString("%1")
                  .arg(qToBigEndian(unsigned short(aY)), 4, 16, QLatin1Char(ASCII::Zero))
                  .toLatin1()               // позиция реквизита по Y
           << int2String(aFont).toLatin1(); // шрифт, см. ESC !
}

//--------------------------------------------------------------------------------
CPrimFR::TData
PrimFRBase::addArbitraryFieldToBuffer(int aX, int aY, const QString &aData, int aFont) {
    return CPrimFR::TData()
           << QString("%1")
                  .arg(qToBigEndian(unsigned short(aX)), 4, 16, QLatin1Char(ASCII::Zero))
                  .toLatin1() // позиция реквизита по X
           << QString("%1")
                  .arg(qToBigEndian(unsigned short(aY)), 4, 16, QLatin1Char(ASCII::Zero))
                  .toLatin1()              // позиция реквизита по Y
           << int2String(aFont).toLatin1() // шрифт, см. ESC !
           << "01"                         // Печать произвольного реквизита
           << "00"                         // N вывода на контрольную ленту
           << m_Codec->fromUnicode(aData);  // данные
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processAnswer(char aError) {
    switch (aError) {
    case CPrimFR::Errors::InvalidStateForCommand: {
        m_ProcessingErrors.push_back(aError);

        return processCommand(CPrimFR::Commands::CancelDocument);
    }
    //--------------------------------------------------------------------------------
    case CPrimFR::Errors::NeedBeginSession: {
        m_ProcessingErrors.push_back(aError);

        return processCommand(CPrimFR::Commands::OpenSession);
    }
    //--------------------------------------------------------------------------------
    case CPrimFR::Errors::NeedBeginFRSession: {
        m_ProcessingErrors.push_back(aError);

        return openFRSession();
    }
    //--------------------------------------------------------------------------------
    case CPrimFR::Errors::NeedZReport: {
        m_ProcessingErrors.push_back(aError);

        m_NeedCloseSession = true;

        return execZReport(true);
    }
    }

    return false;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::openSession() {
    CPrimFR::TData commandData = CPrimFR::TData() << ""; // реквизиты смены

    return processCommand(CPrimFR::Commands::OpenFRSession, commandData);
}

//--------------------------------------------------------------------------------
TStatusCodes PrimFRBase::getRTStatus(int aCommand) {
    m_RTProtocol.setPort(m_IOPort);
    m_RTProtocol.setLog(m_Log);

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
    m_IOPort->setDeviceConfiguration(configuration);

    char answer;
    TStatusCodes result;

    if (m_RTProtocol.processCommand(aCommand, answer)) {
        result = parseRTStatus(aCommand, answer);
    } else {
        result.insert(DeviceStatusCode::Error::NotAvailable);
        m_Offline = true;
    }

    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::None));
    m_IOPort->setDeviceConfiguration(configuration);

    return result;
}

//--------------------------------------------------------------------------------
TStatusCodes PrimFRBase::parseRTStatus(int aCommand, char aAnswer) {
    TStatusCodes result;
    CPrimFR::TStatusBitShifts shifts = CPrimFR::ModelData[m_Model].statusData[aCommand];

    foreach (auto bit, shifts.keys()) {
        if (bool(aAnswer & (1 << bit)) == bool(aCommand)) {
            result.insert(shifts[bit]);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
QString PrimFRBase::int2String(int aValue) {
    return QString("%1").arg(int(uchar(aValue)), 2, 16, QLatin1Char(ASCII::Zero));
}

//--------------------------------------------------------------------------------
QByteArray PrimFRBase::int2ByteArray(int aValue) {
    return int2String(aValue).toLatin1();
}

//--------------------------------------------------------------------------------
template <class T>
void PrimFRBase::loadDeviceData(const CPrimFR::TData &aData,
                                const QString &aName,
                                const QString &aLog,
                                int aIndex,
                                const QString &aExtensibleName) {
    T answerData;

    if (parseAnswerData(aData, aIndex, aLog, answerData)) {
        setDeviceParameter(aName, answerData, aExtensibleName);
    }
}

//--------------------------------------------------------------------------------
