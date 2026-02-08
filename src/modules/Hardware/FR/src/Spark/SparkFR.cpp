/* @file ФР СПАРК. */

#include <QtCore/qmath.h>

#include <Hardware/Protocols/FR/SparkFR.h>
#include <cmath>

#include "AdaptiveFiscalLogic.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
SparkFR::SparkFR() {
    // теги
    m_TagEngine = Tags::PEngine(new CSparkFR::TagEngine());

    // кодек
    m_Codec = CodecByName[CHardware::Codepages::SPARK];

    // данные устройства
    m_DeviceName = CSparkFR::Models::Default;
    m_DocumentState = CSparkFR::DocumentStates::Closed;
    m_LineFeed = false;
    m_SupportedModels = getModelList();
    m_SessionOpeningDT = CSparkFR::ClosedSession;
    m_ZReports = 0;
    m_CheckStatus = true;
    m_CanProcessZBuffer = true;
    // setConfigParameter(CHardware::CanOnline, true);    //TODO: раскомментить после поддержки
    // онлайновой реализации

    setConfigParameter(CHardware::Printer::RetractorEnable, true);

    // ошибки
    m_ErrorData = PErrorData(new CSparkFR::Errors::Data);

    using namespace SDK::Driver::IOPort::COM;

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600); // default
    m_PortParameters[EParameters::BaudRate].append(
        EBaudRate::BR4800); // default after resetting to zero
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

    m_PortParameters[EParameters::Parity].append(EParity::No);
    m_PortParameters[EParameters::Parity].append(EParity::Even);
}

//--------------------------------------------------------------------------------
QDateTime SparkFR::getDateTime() {
    TKKMInfoData data;

    if (getKKMData(data)) {
        return parseDateTime(data);
    }

    return QDateTime();
}

//--------------------------------------------------------------------------------
template <class T> T SparkFR::from_BCD(const QByteArray &aData) {
    T result = 0;
    int size = aData.size();

    for (int i = 0; i < qMin(int(sizeof(T)), qCeil(size / 2)); ++i) {
        int delta = size - 2 * (i + 1);
        result += (T(aData[delta + 0] - ASCII::Zero) << (4 * (2 * i + 1))) +
                  (T(aData[delta + 1] - ASCII::Zero) << (4 * (2 * i + 0)));
    }

    return qToBigEndian(result);
}

//--------------------------------------------------------------------------------
template <> char SparkFR::from_BCD(const QByteArray &aData) {
    char result = aData[0] - ASCII::Zero;

    if (aData.size() > 1) {
        result <<= 4;
        result += aData[1] - ASCII::Zero;
    }

    return result;
}

//--------------------------------------------------------------------------------
char SparkFR::from_BCD(char aData) {
    return from_BCD<char>(QByteArray(1, aData));
}

//--------------------------------------------------------------------------------
bool SparkFR::checkSystem_Flag(const QByteArray &aFlagBuffer, int aNumber) {
    auto dataIt = std::find_if(
        m_System_Flags.begin(),
        m_System_Flags.end(),
        [&](const CSparkFR::System_Flags::SData &aData) -> bool { return aData.number == aNumber; });

    if (dataIt == m_System_Flags.end()) {
        toLog(LogLevel::Error,
              QString("%1: Failed to find system flag %2 in device one")
                  .arg(m_DeviceName)
                  .arg(aNumber));
        return false;
    }

    int dataSize = aFlagBuffer.size();
    int size = aNumber * 2;

    if (dataSize < size) {
        toLog(LogLevel::Error,
              QString("%1: Failed to check system flag %2 (%3) due to size = %4, need %5")
                  .arg(m_DeviceName)
                  .arg(aNumber)
                  .arg(dataIt->name)
                  .arg(dataSize)
                  .arg(size));
        return false;
    }

    bool OK;
    QByteArray flagData = aFlagBuffer.mid((aNumber - 1) * 2, 2);
    char flag = char(flagData.toInt(&OK, 16));

    if (!OK) {
        toLog(LogLevel::Error,
              QString("%1: Failed to check system flag %2 (%3) due to wrong flag data = %4 = 0x%5")
                  .arg(m_DeviceName)
                  .arg(aNumber)
                  .arg(dataIt->name)
                  .arg(flagData.data())
                  .arg(flagData.toHex().data()));
        return false;
    }

    char newFlag = ProtocolUtils::mask(flag, dataIt->mask);

    if (flag != newFlag) {
        QByteArray commandData;
        commandData += QString("%1").arg(aNumber, 2, 10, QChar(ASCII::Zero)).right(2) +
                       QString("%1").arg(newFlag, 2, 16, QChar(ASCII::Zero)).right(2);
        QByteArray answer;

        if (!processCommand(CSparkFR::Commands::SetFlag, commandData, &answer)) {
            toLog(LogLevel::Error,
                  QString("%1: Failed to set system flag %2 (%3)")
                      .arg(m_DeviceName)
                      .arg(aNumber)
                      .arg(dataIt->name));
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::getSystem_Flags(QByteArray &aData, TTaxData *aTaxData) {
    QByteArray answer;

    if (!processCommand(CSparkFR::Commands::TaxesAndFlags, &answer)) {
        return false;
    }

    QList<QByteArray> data = answer.split(CSparkFR::Separator);

    if (data.size() < 5) {
        toLog(LogLevel::Error,
              QString("%1: Failed to check system flags due to size of data list = %2, need %3")
                  .arg(m_DeviceName)
                  .arg(data.size())
                  .arg(5));
        return false;
    }

    if (aTaxData) {
        *aTaxData = data.mid(0, 4);
    }

    aData = data[4];

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::updateParameters() {
    QByteArray flagData;
    TTaxData taxData;

    if (!getSystem_Flags(flagData, &taxData) || checkSystem_Flags(flagData)) {
        return false;
    }

    processDeviceData();

    if (!isFiscal()) {
        return true;
    }

    if (isAdaptiveFCCreation()) {
        foreach (int parameter, CSparkFR::TextProperties::Numbers) {
            QByteArray answer;
            QByteArray commandData = QByteArray(1, char(parameter) + ASCII::Zero);

            if (!processCommand(CSparkFR::Commands::GetTextPropertyName, commandData, &answer)) {
                return false;
            }

            QByteArray prefix =
                QByteArray(3, CSparkFR::TextProperties::Prefix) + QByteArray(17, ASCII::Space);
            commandData =
                QString("%1").arg(parameter, 2, 10, QChar(ASCII::Zero)).right(2).toLatin1() +
                prefix;

            if ((answer.mid(3) != prefix) &&
                !processCommand(CSparkFR::Commands::SetTextPropertyName, commandData)) {
                return false;
            }
        }
    }

    return checkTaxFlags(taxData);
}

//--------------------------------------------------------------------------------
bool SparkFR::checkSystem_Flags(QByteArray &aFlagData) {
    auto dataIt = std::find_if(m_System_Flags.begin(),
                               m_System_Flags.end(),
                               [&](const CSparkFR::System_Flags::SData &aData) -> bool {
                                   return aData.number == CSparkFR::System_Flags::ZReportsAndFiscal;
                               });

    if (dataIt == m_System_Flags.end()) {
        toLog(LogLevel::Error,
              QString("%1: Failed to find system flag %2 in device one")
                  .arg(m_DeviceName)
                  .arg(CSparkFR::System_Flags::ZReportsAndFiscal));
        return false;
    }

    for (int i = 0; i < 8; ++i) {
        if (CSparkFR::LongReportMask & (1 << i)) {
            dataIt->mask[7 - i] = m_OperatorPresence ? '1' : '0';
        }
    }

    foreach (auto data, m_System_Flags) {
        if (!checkSystem_Flag(aFlagData, data.number)) {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::checkTaxFlags(const TTaxData &aTaxData) {
    TTaxes taxes;

    for (int i = 0; i < CSparkFR::TaxRateCount; ++i) {
        bool OK;
        TVAT VAT = TVAT(aTaxData[i].toInt(&OK));
        taxes.append(OK ? VAT : -1);
    }

    m_Taxes = getActualVATs().toList();
    qSort(m_Taxes);

    TVATs absentVATs = getActualVATs() - taxes.toSet();

    if (!absentVATs.isEmpty()) {
        QByteArray commandData;

        for (int i = 0; i < CSparkFR::TaxRateCount; ++i) {
            commandData += (i < m_Taxes.size())
                               ? QString("%1").arg(m_Taxes[i], 2, 10, QChar(ASCII::Zero)).toLatin1()
                               : CSparkFR::NoTaxes;
        }

        if (!processCommand(CSparkFR::Commands::SetTaxes, commandData) ||
            !processCommand(CSparkFR::Commands::AcceptTaxes)) {
            toLog(LogLevel::Error, m_DeviceName + ": Failed to set taxes");
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::isAdaptiveFCCreation() {
    return getConfigParameter(CHardware::FR::FiscalChequeCreation).toString() ==
           CHardware::FR::Values::Adaptive;
}

//--------------------------------------------------------------------------------
bool SparkFR::isConnected() {
    QByteArray answer;
    TResult result = processCommand(CSparkFR::Commands::GetFWVersion, &answer);

    if (!CORRECT(result)) {
        return false;
    }

    if (answer.isEmpty() && isAutoDetecting()) {
        return false;
    }

    QString answerData = m_Codec->toUnicode(answer);
    QRegularExpression regExp(CSparkFR::Models::RegExpData);
    CSparkFR::Models::SData data;

    if (regExp.match(answerData).capturedStart() != -1) {
        QStringList capturedData = regExp.capturedTexts();
        data = CSparkFR::Models::Data[capturedData[1].toInt()];
    }

    m_DeviceName = data.name;
    m_Verified = data.verified;
    m_LineSize = data.lineSize;
    m_System_Flags = data.system_Flags;

    return true;
}

//--------------------------------------------------------------------------------
QStringList SparkFR::getModelList() {
    QStringList result;

    foreach (auto data, CSparkFR::Models::CData().data().values()) {
        result << data.name;
    }

    return result;
}

//--------------------------------------------------------------------------------
TResult SparkFR::execCommand(const QByteArray &aCommand,
                             const QByteArray &aCommandData,
                             QByteArray *aAnswer) {
    MutexLocker locker(&m_ExternalMutex);

    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QByteArray answerData;
    QByteArray &answer = aAnswer ? *aAnswer : answerData;

    if (aCommand == CSparkFR::Commands::ENQT) {
        return m_Protocol.processCommand(aCommand, answer, CSparkFR::Timeouts::Control);
    }

    CSparkFR::Commands::SData data = CSparkFR::Commands::Data[aCommand];

    if (data.password) {
        QByteArray commandData =
            QByteArray::number(CSparkFR::Password).rightJustified(6, ASCII::Zero, true);
        TResult result = processCommand(CSparkFR::Commands::EnterPassword, commandData);

        if (!result) {
            toLog(LogLevel::Error,
                  m_DeviceName + ": Failed to enter control password, unable to perform current "
                                "action therefore");
            return result;
        }
    }

    int timeout = CSparkFR::Timeouts::Default;

    if (aCommand == CSparkFR::Commands::PrintZBuffer) {
        timeout = qMax(1, m_ZReports) * CSparkFR::Timeouts::Report;
    } else if (aCommand == CSparkFR::Commands::Reports) {
        timeout = aCommandData.endsWith(CSparkFR::ZReport) ? CSparkFR::Timeouts::ZReport
                                                           : CSparkFR::Timeouts::Report;
    }

    TResult result = m_Protocol.processCommand(aCommand + aCommandData, answer, timeout);

    if (answer == QByteArray(1, ASCII::ENQ)) {
        if (m_CheckStatus) {
            simplePoll();
        }

        toLog(LogLevel::Error,
              m_DeviceName + ": Error: " + m_ErrorData->value(m_LastError).description);

        if (!m_ProcessingErrors.isEmpty() && (m_ProcessingErrors.last() == m_LastError)) {
            return CommandResult::Device;
        }

        char error = m_LastError;

        if (isErrorUnprocessed(aCommand, error) || !processAnswer(error)) {
            m_LastError = error;
            m_LastCommand = aCommand;

            return CommandResult::Device;
        }

        result = processCommand(aCommand, aCommandData, aAnswer);

        if (result) {
            m_ProcessingErrors.pop_back();
        }

        return result;
    } else if (result == CommandResult::NoAnswer) {
        if (m_CheckStatus) {
            TResult ENQTResult = processCommand(CSparkFR::Commands::ENQT);

            if (!ENQTResult) {
                return ENQTResult;
            }
        }

        return CommandResult::NoAnswer;

        // TODO: действия если нет ответа
    } else if (!result) {
        return result;
    } else if (!data.sending) {
        if (answer == QByteArray(1, ASCII::ACK)) {
            toLog(LogLevel::Error, m_DeviceName + ": ACK received for receiving command");
            return CommandResult::Answer;
        } else if (!data.answer.isEmpty()) {
            if (!answer.startsWith(data.answer)) {
                toLog(LogLevel::Error, m_DeviceName + ": Wrong command code in answer");
                return CommandResult::Answer;
            }

            answer = answer.mid(data.answer.size());
        }
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool SparkFR::processAnswer(char aError) {
    switch (aError) {
    case CSparkFR::Errors::NeedZReport:
    case CSparkFR::Errors::TimeOff: {
        m_ProcessingErrors.push_back(aError);

        if (!m_StatusCollection.contains(FRStatusCode::Error::ZBufferOverflow)) {
            if (m_OperatorPresence) {
                toLog(LogLevel::Error,
                      m_DeviceName +
                          ": Failed to process auto-Z-report due to presence of the operator.");
                m_NeedCloseSession =
                    m_NeedCloseSession || (getSessionState() == ESessionState::Expired);

                return false;
            }

            return execZReport(true);
        }
    }
    //--------------------------------------------------------------------------------
    case CSparkFR::Errors::NeedPayIOOnly:
    case CSparkFR::Errors::NeedSaleOnly: {
        m_ProcessingErrors.push_back(aError);

        return cancelDocument(true);
    }
    //--------------------------------------------------------------------------------
    case CSparkFR::Errors::KKMClosed: {
        m_ProcessingErrors.push_back(aError);

        QByteArray commandData =
            QByteArray::number(CSparkFR::CashierNumber).rightJustified(2, ASCII::Zero, true) +
            QByteArray::number(CSparkFR::CashierPassword).rightJustified(5, ASCII::Zero, true);

        return processCommand(CSparkFR::Commands::OpenKKM, commandData);
    }
    //--------------------------------------------------------------------------------
    case CSparkFR::Errors::KKMOpened: {
        m_ProcessingErrors.push_back(aError);

        return processCommand(CSparkFR::Commands::CloseKKM);
    }
    //--------------------------------------------------------------------------------
    case CSparkFR::Errors::CashierNotSet: {
        m_ProcessingErrors.push_back(aError);

        QByteArray commandData =
            QByteArray::number(CSparkFR::CashierPassword).rightJustified(5, ASCII::Zero, true);

        return processCommand(CSparkFR::Commands::SetCashier, commandData);
    }
    //--------------------------------------------------------------------------------
    case CSparkFR::Errors::WrongTextModeCommand: {
        m_ProcessingErrors.push_back(aError);

        return cut();
    }
    }

    return false;
}

//--------------------------------------------------------------------------------
bool SparkFR::printLine(const QByteArray &aString) {
    char tagModifier = ASCII::NUL;
    if (m_LineTags.contains(Tags::Type::UnderLine))
        tagModifier += CSparkFR::Tag::UnderLine;
    if (m_LineTags.contains(Tags::Type::DoubleHeight))
        tagModifier += CSparkFR::Tag::DoubleHeight;

    QByteArray commandData = char(tagModifier + ASCII::Zero) + aString;

    return processCommand(CSparkFR::Commands::PrintLine, commandData);
}

//--------------------------------------------------------------------------------
void SparkFR::execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine) {
    QByteArray data = m_Codec->from_Unicode(aTagLexeme.data);

    if (aTagLexeme.tags.contains(Tags::Type::DoubleWidth)) {
        Tags::TTypes types;
        types.insert(Tags::Type::DoubleWidth);

        for (int i = 0; i < data.size(); i = i + 2) {
            data.insert(i, m_TagEngine->getTag(types, Tags::Direction::Open));
        }
    }

    aLine = aLine.toByteArray() + data;
}

//--------------------------------------------------------------------------------
bool SparkFR::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray data = perform_Status(aStatusCodes, CSparkFR::Commands::ENQT, 21);

    if (data == CFR::Result::Fail) {
        return false;
    } else if (data != CFR::Result::Error) {
        QDate date = QDate(from_BCD(data[19]), from_BCD(data[18]), from_BCD(data[17])).addYears(2000);
        QTime time = QTime(from_BCD(data[20]), from_BCD(data[21]));
        m_SessionOpeningDT = QDateTime(date, time);
    }

    if (data.size() > 7) {
        m_DocumentState = data[7];
    } else if (data.size() > 6) {
        m_LastError = from_BCD<char>(data.mid(5, 2));
    } // TODO: m_LastCommand
    else if (data.size() > 2)
        CSparkFR::Status3.getSpecification(data[2], aStatusCodes);
    else if (data.size() > 1)
        CSparkFR::Status2.getSpecification(data[1], aStatusCodes);
    else if (data.size() > 0)
        CSparkFR::Status1.getSpecification(data[0], aStatusCodes);

    m_CheckStatus = false;

    if (m_StatusCollection.contains(FRStatusCode::Error::ZBuffer)) {
        getZBufferState();
    }

    data = perform_Status(aStatusCodes, CSparkFR::Commands::GetSensorState, 1);

    if (data == CFR::Result::Fail) {
        return false;
    } else if ((data != CFR::Result::Error) &&
               (from_BCD<char>(data.left(2)) & CSparkFR::PaperInPresenter)) {
        aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
    }

    m_CheckStatus = true;

    return true;
}

//--------------------------------------------------------------------------------
void SparkFR::getZBufferState() {
    // только для 110. отнаследовать!
    QByteArray ZBufferSpaceData;
    QByteArray ZBufferCountData;

    if (!processCommand(CSparkFR::Commands::ZBufferSpace, &ZBufferSpaceData) ||
        (ZBufferSpaceData.size() < 9) ||
        !processCommand(CSparkFR::Commands::ZBufferCount, &ZBufferCountData) ||
        (ZBufferCountData.size() < 2)) {
        m_ZBufferError = true;
    } else {
        m_ZBufferOverflow = m_ZBufferOverflow || bool(from_BCD(ZBufferSpaceData[8]));
        m_ZReports = from_BCD<uchar>(ZBufferCountData.left(2));
        /*
        TODO: для мониторинга
        int usedSpace  = from_BCD<ushort>(ZBufferSpaceData.left(4));
        int totalSpace = from_BCD<ushort>(ZBufferSpaceData.mid(4, 4));
        int totalCount = totalSpace * m_ZReports / usedSpace;
        */
    }
}

//--------------------------------------------------------------------------------
bool SparkFR::payIO(double aAmount, bool aIn) {
    QByteArray commandData = QByteArray::number(CSparkFR::CashPaymentType) +
                             QByteArray::number(int(aAmount * 100)).rightJustified(10, ASCII::Zero);
    QString commandLog = aIn ? "pay in" : "pay out";

    if (!processCommand(aIn ? CSparkFR::Commands::Payin : CSparkFR::Commands::Payout,
                        commandData)) {
        toLog(LogLevel::Error,
              QString("%1: Failed to %2 %3")
                  .arg(m_DeviceName)
                  .arg(commandLog)
                  .arg(aAmount, 0, 'f', 2));
        return false;
    }

    if (!processCommand(CSparkFR::Commands::ClosePayIO)) {
        toLog(LogLevel::Error, QString("%1: Failed to close %2").arg(m_DeviceName).arg(commandLog));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::setFiscalParameters(const QStringList &aReceipt) {
    QByteArray commandData;
    commandData += QString("%1").arg(0, 2, 10, QChar(ASCII::Zero)).right(2);

    if (!processCommand(CSparkFR::Commands::SetTextProperty, commandData)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to clear fiscal parameters");
        return false;
    }

    for (int i = 0; i < aReceipt.size(); ++i) {
        if (!aReceipt[i].isEmpty()) {
            commandData.clear();
            commandData += QString("%1")
                               .arg(CSparkFR::TextProperties::Numbers[i], 2, 10, QChar(ASCII::Zero))
                               .right(2)
                               .toLatin1() +
                           m_Codec->from_Unicode(aReceipt[i]);

            if (!processCommand(CSparkFR::Commands::SetTextProperty, commandData)) {
                toLog(LogLevel::Error,
                      QString("%1: Failed to set fiscal parameter %2 =\n%3")
                          .arg(m_DeviceName)
                          .arg(CSparkFR::TextProperties::Numbers[i])
                          .arg(aReceipt[i]));
                return false;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::perform_Fiscal(const QStringList &aReceipt,
                            const SPaymentData &aPaymentData,
                            quint32 * /*aFDNumber*/) {
    TSum totalAmount = getTotalAmount(aPaymentData);

    if (!payIO(totalAmount, true)) {
        return false;
    }

    AdaptiveFiscalLogic logic(getDeviceConfiguration());

    QStringList noTagReceipt;
    makeReceipt(aReceipt, noTagReceipt);

    bool result = true;

    if (isAdaptiveFCCreation() && logic.adjustReceipt(noTagReceipt)) {
        if (!setFiscalParameters(logic.getTextProperties()) && !processReceipt(aReceipt)) {
            result = false;
        }
    } else if (!processReceipt(aReceipt)) {
        result = false;
    }

    if (result) {
        foreach (auto unitData, aPaymentData.unitDataList) {
            result = result && sale(unitData);
        }

        result = result && processCommand(CSparkFR::Commands::CloseFiscal,
                                          QByteArray::number(CSparkFR::CashPaymentType));

        if (!result) {
            cancelDocument(true);
        }
    }

    if (!result) {
        payIO(totalAmount, false);
    }

    return result;
}

#define CHECK_SPARK_TAX(aNumber)                                                                   \
    if ((m_Taxes.size() >= aNumber) && (aUnitData.VAT == m_Taxes[aNumber - 1]))                      \
        command = CSparkFR::Commands::Sale##aNumber;

//--------------------------------------------------------------------------------
bool SparkFR::sale(const SUnitData &aUnitData) {
    QByteArray command = CSparkFR::Commands::Sale0;

    int index = m_Taxes.indexOf(aUnitData.VAT);

    if (index == 0)
        command = CSparkFR::Commands::Sale1;
    if (index == 1)
        command = CSparkFR::Commands::Sale2;
    if (index == 2)
        command = CSparkFR::Commands::Sale3;
    if (index == 3)
        command = CSparkFR::Commands::Sale4;

    QByteArray commandData =
        QByteArray::number(qRound64(aUnitData.sum * 100.0)).rightJustified(8, ASCII::Zero) +
        QByteArray::number(1 * 1000).rightJustified(8, ASCII::Zero) +
        QByteArray::number(1).rightJustified(2, ASCII::Zero) +
        m_Codec->from_Unicode(aUnitData.name.leftJustified(CSparkFR::LineSize, ASCII::Space, true));

    if (!processCommand(command, commandData)) {
        toLog(LogLevel::Error,
              QString("%1: Failed to sale for %2 (%3, VAT = %4)")
                  .arg(m_DeviceName)
                  .arg(aUnitData.sum, 0, 'f', 2)
                  .arg(aUnitData.name)
                  .arg(aUnitData.VAT));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
double SparkFR::getAmountInCash() {
    QByteArray answer;

    if (!processCommand(CSparkFR::Commands::GetCashAcceptorTotal, &answer) ||
        (answer.size() < 41)) {
        return -1;
    }

    return from_BCD<int>(answer.mid(29, 12)) / 100.0;
}

//--------------------------------------------------------------------------------
bool SparkFR::processPayout(double aAmount) {
    waitEjectorReady();
    int oldDocumentNumber = getLastDocumentNumber();

    bool result = payIO(aAmount, false);
    waitNextPrinting();

    if (!result && (oldDocumentNumber != -1)) {
        int newDocumentNumber = getLastDocumentNumber();

        if ((newDocumentNumber != -1) && (newDocumentNumber > oldDocumentNumber)) {
            return true;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::cut() {
    return processCommand(CSparkFR::Commands::Cut);
}

//--------------------------------------------------------------------------------
bool SparkFR::retract() {
    auto dataIt = std::find_if(m_System_Flags.begin(),
                               m_System_Flags.end(),
                               [&](const CSparkFR::System_Flags::SData &aData) -> bool {
                                   return aData.number == CSparkFR::System_Flags::System_Options2;
                               });

    if (dataIt == m_System_Flags.end()) {
        toLog(LogLevel::Error,
              QString("%1: Failed to find system flag %2 in device one")
                  .arg(m_DeviceName)
                  .arg(CSparkFR::System_Flags::System_Options2));
        return false;
    }

    QByteArray flagData;

    if (!getSystem_Flags(flagData)) {
        return false;
    }

    // отключаем альтернативный способ опроса датчиков презентера
    dataIt->mask[6] = '0';

    if (!checkSystem_Flag(flagData, CSparkFR::System_Flags::System_Options2)) {
        return false;
    }

    if (!processCommand(CSparkFR::Commands::Retract)) {
        return false;
    }

    // отключаем альтернативный способ опроса датчиков презентера
    dataIt->mask[6] = '1';
    checkSystem_Flag(flagData, CSparkFR::System_Flags::System_Options2);

    return true;
}

//--------------------------------------------------------------------------------
bool SparkFR::perform_ZReport(bool aPrintDeferredReports) {
    bool printZBufferOK = !aPrintDeferredReports;

    if (aPrintDeferredReports && m_ZReports) {
        toLog(LogLevel::Normal, m_DeviceName + ": Printing deferred Z-reports");

        printZBufferOK = processCommand(CSparkFR::Commands::PrintZBuffer, CSparkFR::PushZReport);
        getZBufferState();
    }

    bool printZReport = execZReport(false) &&
                        processCommand(CSparkFR::Commands::PrintZBuffer, CSparkFR::PushZReport);

    return (printZBufferOK && aPrintDeferredReports) || printZReport;
}

//--------------------------------------------------------------------------------
bool SparkFR::cancelDocument(bool aDocumentIsOpened) {
    if (!aDocumentIsOpened) {
        return cut();
    }

    if (!processCommand(CSparkFR::Commands::CancelFiscal)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to cancel fiscal document");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
QDateTime SparkFR::parseDateTime(TKKMInfoData &aData) {
    QTime time = QTime::from_String(aData[9], CSparkFR::TimeFormat);
    QDate date = QDate::from_String(aData[10].insert(4, "20"), CFR::DateFormat);

    return QDateTime(date, time);
}

//--------------------------------------------------------------------------------
void SparkFR::processDeviceData() {
    QByteArray answer;

    if (processCommand(CSparkFR::Commands::GetFWVersion, &answer)) {
        QString answerData = m_Codec->toUnicode(answer);
        QRegularExpression regExp(CSparkFR::Models::RegExpData);

        if (regExp.match(answerData).capturedStart() != -1) {
            QStringList capturedData = regExp.capturedTexts();
            setDeviceParameter(CDeviceData::Firmware,
                               QString("%1").arg(capturedData[2].toDouble(), 0, 'f', 2));
        }
    }

    TKKMInfoData data;

    if (getKKMData(data)) {
        setDeviceParameter(CDeviceData::FR::TotalPaySum, data[1].toInt());
        setDeviceParameter(CDeviceData::FR::FiscalDocuments, data[2].toInt());
        setDeviceParameter(CDeviceData::Count, data[3].toInt(), CDeviceData::FR::Session);
        setDeviceParameter(CDeviceData::FR::NonFiscalDocuments, data[4].toInt());
        setDeviceParameter(CDeviceData::FR::OwnerId, data[6].toInt());

        m_RNM = CFR::RNMToString(data[7]);
        m_Serial = CFR::serialToString(data[8]);
    }

    if (processCommand(CSparkFR::Commands::EKLZInfo, &answer) && (answer.size() >= 42)) {
        qulonglong EKLZSerial = from_BCD<qulonglong>(answer.mid(32, 10));
        setDeviceParameter(CDeviceData::EKLZ::Serial, EKLZSerial);
    }

    removeDeviceParameter(CDeviceData::FR::EKLZ);

    if (processCommand(CSparkFR::Commands::GetEKLZError, &answer) && !answer.isEmpty()) {
        char reregistrationData = answer[0];
        m_Fiscalized = reregistrationData != CSparkFR::NoReregistrationNumber;
        setDeviceParameter(CDeviceData::FR::Activated, m_Fiscalized, CDeviceData::FR::EKLZ);

        if (m_Fiscalized) {
            int reregistrationNumber = from_BCD(reregistrationData);

            if (reregistrationNumber != -1) {
                setDeviceParameter(CDeviceData::FR::ReregistrationNumber, reregistrationNumber);
            }
        }
    }

    // пока как инфо, чтобы посмотреть состояние Z-буфера на загрузке. Потом пойдет в мониторинг.
    getZBufferState();
    checkDateTime();
}

//--------------------------------------------------------------------------------
bool SparkFR::processXReport() {
    waitEjectorReady();
    int oldDocumentNumber = getLastDocumentNumber();

    QByteArray commandData = QByteArray(1, CSparkFR::SessionReport) + CSparkFR::XReport;
    TResult result = processCommand(CSparkFR::Commands::Reports, commandData);
    waitNextPrinting();

    if (!result && (oldDocumentNumber != -1)) {
        int newDocumentNumber = getLastDocumentNumber();

        if ((newDocumentNumber != -1) && (newDocumentNumber > oldDocumentNumber)) {
            return true;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::getKKMData(TKKMInfoData &aData) {
    QByteArray answer;

    if (!processCommand(CSparkFR::Commands::KKMInfo, &answer)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get KKM info");
        return false;
    }

    aData = answer.split(CSparkFR::Separator);

    if (aData.size() < 11) {
        toLog(LogLevel::Error,
              QString("%1: Too small sections in KKM info answer = %2, need 11 min")
                  .arg(m_DeviceName)
                  .arg(aData.size()));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
int SparkFR::getLastDocumentNumber() {
    TKKMInfoData data;

    return getKKMData(data) ? data[5].toInt() : -1;
}

//--------------------------------------------------------------------------------
ESessionState::Enum SparkFR::getSessionState() {
    QByteArray data;

    if (!processCommand(CSparkFR::Commands::ENQT, &data) || (data.size() <= 2)) {
        return ESessionState::Error;
    }

    if (data[2] & CSparkFR::SessionExpired) {
        return ESessionState::Expired;
    }

    /*
    isSessionExpired() - удалено
    {
            TKKMInfoData data;

            return (getSessionState() == ESessionState::Opened) && getKKMData(data) &&
    m_SessionOpeningDT.daysTo(parseDateTime(data));
    }
    */

    // Т.е. если дата и время начала смены валидны, то она открыта. Возможно - особенность
    // нефискализированного аппарата.
    bool result = m_SessionOpeningDT != CSparkFR::ClosedSession;

    return result ? ESessionState::Opened : ESessionState::Closed;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum SparkFR::getDocumentState() {
    QByteArray data;

    if (!processCommand(CSparkFR::Commands::ENQT, &data) || (data.size() <= 7)) {
        return EDocumentState::Error;
    }

    return data[7] ? EDocumentState::Opened : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
bool SparkFR::execZReport(bool /*aAuto*/) {
    toLog(LogLevel::Normal,
          m_DeviceName + QString(": Begin processing %1-report").arg(CSparkFR::ZReport));
    ESessionState::Enum sessionState = getSessionState();

    if (sessionState == ESessionState::Error)
        return false;
    else if (sessionState == ESessionState::Closed)
        return true;

    m_NeedCloseSession = false;
    QByteArray commandData = QByteArray(1, CSparkFR::SessionReport) + CSparkFR::ZReport;
    bool result = processCommand(CSparkFR::Commands::Reports, commandData);

    m_NeedCloseSession = getSessionState() == ESessionState::Expired;
    m_ZReports += int(result);

    return result;
}

//--------------------------------------------------------------------------------
bool SparkFR::waitEjectorReady() {
    TStatusCodes statusCodes;
    bool pollled = false;
    auto poll = [&]() -> bool {
        pollled = true;
        statusCodes.clear();
        return getStatus(statusCodes);
    };
    auto condition = [&]() -> bool {
        return pollled && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable) &&
               !statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter);
    };

    return PollingExpector().wait<bool>(poll, condition, CSparkFR::EjectorWaiting); // || retract();
}

//--------------------------------------------------------------------------------
bool SparkFR::waitNextPrinting() {
    TStatusCodes statusCodes;
    bool pollled = false;
    auto poll = [&]() -> bool {
        pollled = true;
        statusCodes.clear();
        return getStatus(statusCodes);
    };
    auto condition = [&]() -> bool {
        return pollled && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable) &&
               !statusCodes.contains(DeviceStatusCode::Error::Unknown);
    };

    return PollingExpector().wait<bool>(poll, condition, CSparkFR::PrintingWaiting);
}

//--------------------------------------------------------------------------------
