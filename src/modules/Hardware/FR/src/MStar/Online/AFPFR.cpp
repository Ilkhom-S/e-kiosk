/* @file Онлайн ФР семейства MStar на протоколе AFP. */

#include "AFPFR.h"

#include "ModelData.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
AFPFR::AFPFR() {
    using namespace SDK::Driver::IOPort::COM;

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // USB
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // параметры семейства ФР
    m_DeviceName = CAFPFR::Models::Default;
    m_IsOnline = true;
    m_LineFeed = false;
    m_NextReceiptProcessing = false;

    setConfigParameter(CHardwareSDK::CanOnline, true);
    setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

    // налоги. Таблица налогов программно недоступна.
    m_TaxData.add(10, 1);
    m_TaxData.add(18, 0);
    m_TaxData.add(0, 5);

    // ошибки
    m_ErrorData = PErrorData(new CAFPFR::Errors::Data());
}

//--------------------------------------------------------------------------------
static QStringList AFPFR::getModelList() {
    QList<CAFPFR::Models::SData> modelData = CAFPFR::Models::CData().data().values();
    QStringList result;

    foreach (auto data, modelData) {
        result << data.name;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::getFRData(const CAFPFR::FRInfo::SData &aInfo, CAFPFR::TData &aData) {
    QString log = QString("data by index %1 (%2)").arg(aInfo.index).arg(aInfo.name);
    CAFPFR::TAnswerTypes answerTypes =
        CAFPFR::Requests::Data[CAFPFR::Commands::GetFRData].answerTypes;
    answerTypes.removeLast();

    if (!processCommand(
            CAFPFR::Commands::GetFRData, aInfo.index, &aData, answerTypes + aInfo.answerTypes)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get " + log);
        return false;
    }
    if (aInfo.index != aData[0].toInt()) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Invalid data index in answer = %1 at the request of %2")
                                 .arg(aData[0].toInt())
                                 .arg(log));
        return false;
    }

    aData = aData.mid(1);

    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::getLastFiscalizationData(int aField, QVariant &aData) {
    CAFPFR::TData data = 0;

    if (!processCommand(CAFPFR::Commands::GetLastFiscalizationData, aField, &data)) {
        return false;
    }

    aData = data[0];

    return aData.isValid();
}

//--------------------------------------------------------------------------------
bool AFPFR::updateParameters() {
    processDeviceData();

    if (!m_IsOnline) {
        return true;
    }

    setFRParameter(CAFPFR::FRParameters::PrintingOnClose, true);

    QVariant data;

    if (!getLastFiscalizationData(CFR::FiscalFields::TaxSystemsReg, data) ||
        !checkTaxSystems(char(data.toInt()))) {
        return false;
    }

    if (!getLastFiscalizationData(CFR::FiscalFields::AgentFlagsReg, data) ||
        !checkAgentFlags(char(data.toInt()))) {
        return false;
    }

    CFR::FiscalFields::TFields fields =
        CFR::FiscalFields::TFields()
        << CFR::FiscalFields::ModeFields << CFR::FiscalFields::FTSURL << CFR::FiscalFields::OFDURL
        << CFR::FiscalFields::OFDName << CFR::FiscalFields::LegalOwner
        << CFR::FiscalFields::PayOffAddress << CFR::FiscalFields::PayOffPlace;

    foreach (auto field, fields) {
        if (!getLastFiscalizationData(field, data)) {
            return false;
        }

        QString textKey = m_FFData[field].textKey;
        m_FFEngine.setConfigParameter(textKey, m_Codec->toUnicode(data.toByteArray()));
        QString value = m_FFEngine.getConfigParameter(textKey, data).toString();

        toLog(LogLevel::Normal,
              m_DeviceName + QString(": Add %1 = \"%2\" to config data")
                                 .arg(m_FFData.getTextLog(field))
                                 .arg(value));
    }

    return !m_OperatorPresence || loadSectionNames();
}

//--------------------------------------------------------------------------------
bool AFPFR::loadSectionNames() {
    TSectionNames sectionNames;

    for (int i = 1; i <= CAFPFR::SectionAmount; ++i) {
        QVariant data;

        if (!getFRParameter(CAFPFR::FRParameters::SectionName(i), data)) {
            toLog(LogLevel::Error,
                  m_DeviceName + QString(": Failed to get name for %1 section").arg(i));
            return false;
        }

        sectionNames.insert(i, data.toString());
    }

    if (sectionNames.isEmpty()) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Failed to get section names due to they are not exist");
        return false;
    }

    setConfigParameter(CHardwareSDK::FR::SectionNames,
                       QVariant::fromValue<TSectionNames>(sectionNames));

    return true;
}

//--------------------------------------------------------------------------------
void AFPFR::processDeviceData() {
    using namespace CAFPFR::EAnswerTypes;

    CAFPFR::TData frData = 0;

    if (getFRData(CAFPFR::FRInfo::SerialNumber, FRData))
        m_Serial = CFR::serialToString(FRData[0].toByteArray());
    if (getFRData(CAFPFR::FRInfo::INN, FRData))
        m_INN = CFR::INNToString(FRData[0].toByteArray());
    if (getFRData(CAFPFR::FRInfo::RNM, FRData))
        m_RNM = CFR::RNMToString(FRData[0].toByteArray());
    if (getFRData(CAFPFR::FRInfo::FFDFR, FRData))
        m_FFDFR = EFFD::Enum(FRData[0].toInt());
    if (getFRData(CAFPFR::FRInfo::FFDFS, FRData))
        m_FFDFS = EFFD::Enum(FRData[1].toInt());

    if (getFRData(CAFPFR::FRInfo::Firmware, frData)) {
        QString data = FRData[0].toString();

        if (data.size() >= 4) {
            data = data.insert(1, ASCII::Dot).insert(3, ASCII::Dot).insert(5, ASCII::Dot);
        }

        setDeviceParameter(CDeviceData::Firmware, data);

        m_OldFirmware = DeviceUtils::isComplexFirmwareOld(data, m_ModelData.firmware);
    }

    if (getFRData(CAFPFR::FRInfo::FirmwareDate, frData)) {
        setDeviceParameter(CDeviceData::Date,
                           FRData[0].toDate().toString(CFR::DateLogFormat),
                           CDeviceData::Firmware);
    }

    if (getFRData(CAFPFR::FRInfo::TotalPaySum, frData)) {
        setDeviceParameter(CDeviceData::FR::TotalPaySum, FRData[0].toDouble());
    }

    if (getFRData(CAFPFR::FRInfo::LastRegDate, frData)) {
        setDeviceParameter(CDeviceData::FR::LastRegistrationDate,
                           FRData[0].toDate().toString(CFR::DateLogFormat));
    }

    CAFPFR::TData answerData = 0;

    if (processCommand(CAFPFR::Commands::GetFSStatus, &answerData)) {
        m_Fiscalized = answerData[0].toInt() == CAFPFR::FSData::FiscalMode;
        setDeviceParameter(CDeviceData::FR::Session,
                           answerData[0].toInt() ? CDeviceData::Values::Opened
                                                 : CDeviceData::Values::Closed);
        setDeviceParameter(CDeviceData::FR::FiscalDocuments, answerData[8].toInt());

        m_FSSerialNumber = CFR::FSSerialToString(answerData[7].toByteArray());

        setDeviceParameter(CDeviceData::FS::Version,
                           QString("%1, type %2")
                               .arg(answerData[9].toString())
                               .arg(answerData[10].toInt() ? "serial" : "debug"));
        setDeviceParameter(CDeviceData::FR::FreeReregistrations, answerData[12].toInt());
        setDeviceParameter(CDeviceData::FR::ReregistrationNumber, answerData[13].toInt());

        QDate date = answerData[11].toDate();
        setDeviceParameter(CDeviceData::FS::ValidityData, CFR::FSValidityDateOff(date));
    }

    QVariant addressData;
    QVariant portData;
    m_OFDDataError = !getFRParameter(CAFPFR::FRParameters::OFDAddress, addressData) ||
                     !getFRParameter(CAFPFR::FRParameters::OFDPort, portData);
    portData = getBufferFrom_String(QByteArray::number(portData.toInt(), 16));
    m_OFDDataError =
        m_OFDDataError || !checkOFDData(addressData.toByteArray(), portData.toByteArray());

    checkDateTime();
}

//--------------------------------------------------------------------------------
QDateTime AFPFR::getDateTime() {
    CAFPFR::TData answerData = 0;

    if (processCommand(CAFPFR::Commands::GetFRDateTime, &answerData)) {
        return QDateTime(answerData[0].toDate(), answerData[1].toTime());
    }

    return QDateTime();
}

//--------------------------------------------------------------------------------
bool AFPFR::getFRParameter(const CAFPFR::FRParameters::SData &aData, QVariant &aValue) {
    CAFPFR::TData commandData = CAFPFR::TData() << aData.number << aData.index;
    CAFPFR::TData answerData = 0;

    if (!processCommand(
            CAFPFR::Commands::GetFRParameter, commandData, &answerData, aData.answerType)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get " + aData.log());
        return false;
    }

    aValue = answerData[0];

    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::setFRParameter(const CAFPFR::FRParameters::SData &aData, QVariant aValue) {
    if ((aData.bit != CAFPFR::FRParameters::NoBit) &&
        ((aData.answerType == CAFPFR::EAnswerTypes::Int) ||
         (aData.answerType == CAFPFR::EAnswerTypes::FInt))) {
        QVariant value;

        if (!getFRParameter(aData, value)) {
            return false;
        }

        int data = value.toInt();
        int mask = 1 << aData.bit;
        int newData = int(aValue.toBool()) << aData.bit;

        data &= ~mask;
        data |= newData;

        aValue = data;

        if (aValue == value) {
            return true;
        }
    }

    CAFPFR::TData commandData = CAFPFR::TData() << aData.number << aData.index << aValue;

    if (!processCommand(CAFPFR::Commands::SetFRParameter, commandData)) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to set %1 = %2").arg(aData.log()).arg(aValue.toString()));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::isConnected() {
    CAFPFR::TData frData = 0;

    if (!getFRData(CAFPFR::FRInfo::ModelId, frData)) {
        return false;
    }

    QString modelId = FRData[0].toString();
    m_ModelData = CAFPFR::Models::Data[modelId];
    m_Verified = m_ModelData.verified;
    m_DeviceName = m_ModelData.name;

    if (m_DeviceName == CAFPFR::Models::Default) {
        toLog(LogLevel::Error, m_DeviceName + ": Unknown model Id = " + modelId);
    }

    m_ModelCompatibility = true;

    return true;
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, CAFPFR::TData *aAnswer) {
    return processCommand(aCommand, CAFPFR::TData(), aAnswer);
}

//--------------------------------------------------------------------------------
static TResult
AFPFR::processCommand(char aCommand, const QVariant &aCommandData, CAFPFR::TData *aAnswer) {
    return processCommand(aCommand, CAFPFR::TData() << aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult
AFPFR::processCommand(char aCommand, const CAFPFR::TData &aCommandData, CAFPFR::TData *aAnswer) {
    return processCommand(aCommand, aCommandData, aAnswer, CAFPFR::TAnswerTypes());
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand,
                              const CAFPFR::TData &aCommandData,
                              CAFPFR::TData *aAnswer,
                              CAFPFR::EAnswerTypes::Enum aAnswerType) {
    return processCommand(aCommand, aCommandData, aAnswer, CAFPFR::TAnswerTypes() << aAnswerType);
}

//--------------------------------------------------------------------------------
static TResult AFPFR::processCommand(char aCommand,
                                     const QVariant &aCommandData,
                                     CAFPFR::TData *aAnswer,
                                     CAFPFR::EAnswerTypes::Enum aAnswerType) {
    return processCommand(
        aCommand, CAFPFR::TData() << aCommandData, aAnswer, CAFPFR::TAnswerTypes() << aAnswerType);
}

//--------------------------------------------------------------------------------
static TResult AFPFR::processCommand(char aCommand,
                                     const QVariant &aCommandData,
                                     CAFPFR::TData *aAnswer,
                                     const CAFPFR::TAnswerTypes &aAnswerTypes) {
    return processCommand(aCommand, CAFPFR::TData() << aCommandData, aAnswer, aAnswerTypes);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand,
                              const CAFPFR::TData &aCommandData,
                              CAFPFR::TData *aAnswer,
                              const CAFPFR::TAnswerTypes &aAnswerTypes) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QByteArray commandData;

    foreach (auto dataItem, aCommandData) {
        int type = dataItem.typeId();
        QByteArray data;

        if (type == QMetaType::QString)
            data = m_Codec->fromUnicode(dataItem.toString());
        else if ((type == QMetaType::QByteArray) || (type == QMetaType::Double))
            data = dataItem.toByteArray();
        else
            data = QByteArray::number(dataItem.toULongLong());

        commandData += CAFPFR::Separator + data;
    }

    QByteArray command =
        QByteArray::number(uchar(aCommand), 16).rightJustified(2, ASCII::Zero).right(2).toUpper();
    commandData.replace(0, 1, command);

    QByteArray answer;
    m_LastError = 0;

    m_LastCommandResult =
        m_Protocol.processCommand(commandData, answer, CAFPFR::Requests::Data[aCommand].timeout);

    if (!m_LastCommandResult) {
        return m_LastCommandResult;
    }

    bool ok = false;
    QByteArray errorData = answer.left(2);
    char error = char(errorData.toUShort(&ok, 16));

    if (!ok) {
        toLog(LogLevel::Error,
              m_DeviceName +
                  QString(": Failed to parse error = 0x%1").arg(errorData.toHex().data()));
        return false;
    }

    m_LastError = error;
    m_LastCommand = QByteArray(1, aCommand);
    QList<QByteArray> answerData = answer.mid(2).split(CAFPFR::Separator);

    if (answer.endsWith(CAFPFR::Separator)) {
        answerData.removeLast();
    }

    if (!m_LastError) {
        if (aAnswer) {
            aAnswer->clear();

            int answerSize = answerData.size();
            CAFPFR::Requests::SData requestData = CAFPFR::Requests::Data[aCommand];
            CAFPFR::TAnswerTypes answerTypes = requestData.answerTypes;
            int size = requestData.answerTypes.size();

            if (!aAnswerTypes.isEmpty()) {
                answerTypes = aAnswerTypes;
                size = aAnswerTypes.size();
            }

            if (answerSize < size) {
                toLog(LogLevel::Error,
                      m_DeviceName + QString(": Wrong answer quantity of parts = %1, need = %2")
                                         .arg(size)
                                         .arg(size));
                return CommandResult::Answer;
            }

            for (int i = 0; i < size; ++i) {
                QByteArray part = answerData[i].simplified();
                int answerType = answerTypes[i];

                switch (answerType) {
                case CAFPFR::EAnswerTypes::Unknown:
                    *aAnswer << part;
                    break;
                case CAFPFR::EAnswerTypes::String:
                    *aAnswer << m_Codec->toUnicode(part);
                    break;
                case CAFPFR::EAnswerTypes::FString: {
                    if (part.isEmpty()) {
                        toLog(LogLevel::Error,
                              m_DeviceName + QString(": Failed to parse string of answer part [%1] "
                                                     "due to it is empty")
                                                 .arg(i));
                        return CommandResult::Answer;
                    }

                    *aAnswer << m_Codec->toUnicode(part);

                    break;
                }
                case CAFPFR::EAnswerTypes::Date: {
                    QDate result = QDate::fromString(part.insert(4, "20"), CAFPFR::DateFormat);

                    if (!result.isValid()) {
                        toLog(LogLevel::Error,
                              m_DeviceName +
                                  QString(": Failed to parse date of answer part [%1] = %2")
                                      .arg(i)
                                      .arg(part.data()));
                        return CommandResult::Answer;
                    }

                    *aAnswer << result;

                    break;
                }
                case CAFPFR::EAnswerTypes::Time: {
                    QTime result = QTime::fromString(part, CAFPFR::TimeFormat);

                    if (!result.isValid()) {
                        toLog(LogLevel::Error,
                              m_DeviceName +
                                  QString(": Failed to parse time of answer part [%1] = %2")
                                      .arg(i)
                                      .arg(part.data()));
                        return CommandResult::Answer;
                    }

                    *aAnswer << result;

                    break;
                }
                case CAFPFR::EAnswerTypes::Int:
                case CAFPFR::EAnswerTypes::FInt: {
                    if (part.isEmpty()) {
                        if (answerType == CAFPFR::EAnswerTypes::FInt) {
                            toLog(LogLevel::Error,
                                  m_DeviceName +
                                      QString(": Failed to parse int of answer part [%1] "
                                              "due to it is empty")
                                          .arg(i));
                            return CommandResult::Answer;
                        }

                        *aAnswer << 0;

                        break;
                    }

                    QRegularExpression regExp("^[0-9]+$");

                    if (regExp.match(part).capturedStart() == -1) {
                        toLog(LogLevel::Error,
                              m_DeviceName +
                                  QString(": Failed to parse int of answer part [%1] = %2")
                                      .arg(i)
                                      .arg(part.data()));
                        return CommandResult::Answer;
                    }

                    *aAnswer << part.toULongLong();

                    break;
                }
                case CAFPFR::EAnswerTypes::Double: {
                    QRegularExpression regExp("^[0-9\\.]+$");

                    if (!part.isEmpty() && (regExp.match(part).capturedStart() == -1)) {
                        toLog(LogLevel::Error,
                              m_DeviceName +
                                  QString(": Failed to parse double of answer part [%1] = %2")
                                      .arg(i)
                                      .arg(part.data()));
                        return CommandResult::Answer;
                    }

                    *aAnswer << (part.isEmpty() ? 0 : part.toDouble());

                    break;
                }
                default: {
                    toLog(LogLevel::Error,
                          m_DeviceName +
                              QString(
                                  ": Failed to parse answer part [%1] = %2 due to unknown type %3")
                                  .arg(i)
                                  .arg(part.data())
                                  .arg(requestData.answerTypes[i]));
                    return CommandResult::Driver;
                }
                }
            }
        }

        return CommandResult::OK;
    }

    toLog(LogLevel::Error,
          m_DeviceName + ": Error: " + m_ErrorData->value(m_LastError).description);

    if (!answerData.isEmpty()) {
        QStringList logData;

        for (int i = 0; i < answerData.size(); ++i) {
            logData << m_Codec->toUnicode(answerData[i]);
        }

        logData.removeAll("");

        if (!logData.isEmpty()) {
            QString log = logData.join("\n1C\n").remove("\r");
            logData = log.split("\n");
            logData.removeAll("");
            toLog(LogLevel::Error, m_DeviceName + ": Stack:\n" + logData.join("\n"));
        }
    }

    if (!isErrorUnprocessed(aCommand, error)) {
        setErrorFlags();
    }

    if (!m_ProcessingErrors.isEmpty() && (m_ProcessingErrors.last() == m_LastError)) {
        return CommandResult::Device;
    }

    if (isErrorUnprocessed(aCommand, error) || !processAnswer(aCommand, error)) {
        m_LastError = error;
        m_LastCommand = QByteArray(1, aCommand);

        return CommandResult::Device;
    }

    TResult result = processCommand(aCommand, aCommandData, aAnswer, aAnswerTypes);

    if (result) {
        m_ProcessingErrors.pop_back();
    }

    return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::processAnswer(char aCommand, char aError) {
    switch (aError) {
    case CAFPFR::Errors::NeedZReport: {
        m_ProcessingErrors.append(aError);

        return execZReport(true) && openFRSession();
    }
    case CAFPFR::Errors::WrongState: {
        m_ProcessingErrors.append(aError);

        if (aCommand == CAFPFR::Commands::CancelDocument) {
            return false;
        }

        if (getDocumentState() == EDocumentState::Opened) {
            return processCommand(CAFPFR::Commands::CancelDocument);
        }
    }
    case CAFPFR::Errors::UnknownCommand: {
        m_OldFirmware = m_OldFirmware || (aCommand == CAFPFR::Commands::GetFRData);

        break;
    }
    }

    return false;
}

//--------------------------------------------------------------------------------
bool AFPFR::performFiscal(const QStringList &aReceipt,
                          const SPaymentData &aPaymentData,
                          const quint32 *aFDNumber) {
    if ((getDocumentState() == EDocumentState::Opened) &&
        !processCommand(CAFPFR::Commands::CancelDocument)) {
        return false;
    }

    // СНО
    char taxSystem = char(aPaymentData.taxSystem);

    if ((taxSystem != ETaxSystems::None) && (m_TaxSystems.size() != 1) &&
        !processCommand(CAFPFR::Commands::SetTaxSystem, taxSystem)) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to set taxation system %1 (%2)")
                                 .arg(toHexLog(taxSystem))
                                 .arg(CFR::TaxSystems[taxSystem]));
        return false;
    }

    // флаг агента
    char agentFlag = char(aPaymentData.agentFlag);

    if (!processCommand(CAFPFR::Commands::SetAgentFlag, agentFlag)) {
        toLog(LogLevel::Error,
              m_DeviceName + QString(": Failed to set agent flag %1 (%2)")
                                 .arg(toHexLog(agentFlag))
                                 .arg(CFR::AgentFlags[agentFlag]));
        return false;
    }

    // фискальные теги, ассоциированные с флагом агента
    bool isBankAgent = CFR::isBankAgent(aPaymentData.agentFlag);
    bool isPaymentAgent = CFR::isPaymentAgent(aPaymentData.agentFlag);

#define CHECK_AFP_FF(aField)                                                                       \
    setFRParameter(CAFPFR::FRParameters::aField, m_FFEngine.getConfigParameter(CFiscalSDK::aField))

    if (isNotPrinting() && !CHECK_AFP_FF(SenderMail)) {
        return false;
    }

    if (isBankAgent) {
        if (!CHECK_AFP_FF(TransferOperatorAddress) || !CHECK_AFP_FF(TransferOperatorINN) ||
            !CHECK_AFP_FF(TransferOperatorName) || !CHECK_AFP_FF(AgentOperation) ||
            !CHECK_AFP_FF(TransferOperatorPhone)) {
            return false;
        }
    } else if (isPaymentAgent) {
        if (!CHECK_AFP_FF(ProcessingPhone)) {
            return false;
        }
    }

    if (isBankAgent || isPaymentAgent) {
        if (!CHECK_AFP_FF(AgentPhone) || !CHECK_AFP_FF(ProviderPhone)) {
            return false;
        }
    }

    char fdType = CAFPFR::PayOffTypeData[aPaymentData.payOffType];

    if (!processReceipt(aReceipt, false) || !openDocument(FDType)) {
        return false;
    }

    bool result = true;
    ExitAction exitAction([&]() { processCommand(CAFPFR::Commands::CancelDocument); });

    foreach (auto unitData, aPaymentData.unitDataList) {
        result = result && sale(unitData);
    }

    bool needSetUserContact =
        isNotPrinting() || m_OperationModes.contains(EOperationModes::Internet);
    QVariant userContact = getConfigParameter(CFiscalSDK::UserContact);

    if (needSetUserContact && !processCommand(CAFPFR::Commands::SetUserContact, userContact)) {
        return false;
    }

    CAFPFR::TData data = CAFPFR::TData()
                         << (aPaymentData.payType - 1) << getTotalAmount(aPaymentData) << "";

    if (!result || !processCommand(CAFPFR::Commands::Total, data) || !closeDocument(true)) {
        return false;
    }

    if (aFDNumber && processCommand(CAFPFR::Commands::GetFSStatus, &data)) {
        *aFDNumber = data[8].toUInt();
    }

    return exitAction.reset();
}

//--------------------------------------------------------------------------------
static bool AFPFR::getFiscalFields(quint32 aFDNumber,
                                   TFiscalPaymentData &aFPData,
                                   TComplexFiscalPaymentData &aPSData) {
    CAFPFR::TData answer = 0;

    if (!processCommand(CAFPFR::Commands::GetFiscalTLVData, QVariant(aFDNumber), &answer)) {
        return false;
    }

    QString rawData = answer[0].toString();
    QString log;

    if (!checkBufferString(rawData, &log)) {
        toLog(LogLevel::Error, m_DeviceName + ": " + log);
        return false;
    }

    QByteArray data = getBufferFrom_String(rawData);
    int i = 0;

    while ((i + 4) < data.size()) {
        CFR::STLV tlv;

        auto getInt = [&data, &i](int aIndex, int aShift) -> int {
            int result = uchar(data[i + aIndex]);
            return result << (8 * aShift);
        };
        int field = getInt(0, 0) | getInt(1, 1);
        int size = getInt(2, 0) | getInt(3, 1);

        if (size == 0) {
            toLog(LogLevel::Warning, m_DeviceName + ": Mo data for " + m_FFData.getTextLog(field));
        }

        if (!m_FFEngine.parseTLV(data.mid(i, size + 4), TLV)) {
            return false;
        }

        m_FFEngine.parseSTLVData(TLV, aPSData);
        m_FFEngine.parseTLVData(TLV, aFPData);

        i += size + 4;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::sale(const SUnitData &aUnitData) {
    QVariant section;

    if (aUnitData.section != -1) {
        section = aUnitData.section;
    }

    CAFPFR::TData commandData =
        CAFPFR::TData() << aUnitData.name // название товара
                        << ""             // артикул или штриховой код товара/номер ТРК
                        << 1              // количество
                        << aUnitData.sum  // цена
                        << m_TaxData[aUnitData.VAT].group    // номер ставки налога
                        << ""                                // номер товарной позиции
                        << section                           // номер секции
                        << ""                                // код товара
                        << ""                                // ИНН поставщика
                        << aUnitData.payOffSubjectMethodType // признак способа расчёта (1214)
                        << aUnitData.payOffSubjectType       // признак предмета расчёта (1212)
                        << ""                                // доп. реквизит предмета расчёта
                        << ""                                // код страны происхождения товара
                        << ""                                // номер таможенной декларации
                        << 0.00                              // акциз
                        << 0                                 // признак агента по предмету расчёта
                        << ""                                // единица измерения
                        << 0.00;                             // сумма НДС за предмет расчёта

    return processCommand(CAFPFR::Commands::Sale, commandData);
}

//--------------------------------------------------------------------------------
EResult::Enum
AFPFR::perform_Command(TStatusCodes &aStatusCodes, char aCommand, CAFPFR::TData &aAnswerData) {
    TResult result = processCommand(aCommand, &aAnswerData);

    if (result == CommandResult::Device) {
        int statusCode = getErrorStatusCode(m_ErrorData->value(m_LastError).type);
        aStatusCodes.insert(statusCode);
    } else if (result == CommandResult::Answer) {
        aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
    } else if (result) {
        return EResult::OK;
    }

    return !CORRECT(result) ? EResult::Fail : EResult::Error;
}

//--------------------------------------------------------------------------------
bool AFPFR::getStatus(TStatusCodes &aStatusCodes) {
    CAFPFR::TData answerData = 0;
    EResult::Enum result =
        perform_Command(aStatusCodes, CAFPFR::Commands::GetPrinterStatus, answerData);

    if (result == EResult::Fail) {
        return false;
    }
    if (result != EResult::Error) {
        CAFPFR::Statuses::Printer.getSpecification(char(answerData[0].toInt()), aStatusCodes);
    }

    result = perform_Command(aStatusCodes, CAFPFR::Commands::GetFRStatus, answerData);

    if (result == EResult::Fail) {
        return false;
    }
    if (result != EResult::Error) {
        CAFPFR::Statuses::FR.getSpecification(char(answerData[0].toInt()), aStatusCodes);
    }

    result = perform_Command(aStatusCodes, CAFPFR::Commands::GetOFDStatus, answerData);

    if (result == EResult::Fail) {
        return false;
    }
    if (result != EResult::Error) {
        checkOFDNotSentCount(answerData[2].toInt(), aStatusCodes);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::processReceipt(const QStringList &aReceipt, bool aProcessing) {
    if (!isPrintingNeed(aReceipt)) {
        return true;
    }

    if (!aProcessing) {
        return TSerialFRBase::processReceipt(aReceipt, false);
    }

    if (!openDocument(CAFPFR::DocumentTypes::NonFiscal)) {
        return false;
    }

    bool result = TSerialFRBase::processReceipt(aReceipt, false);
    bool processing = closeDocument(true);

    return result && processing;
}

//--------------------------------------------------------------------------------
static bool AFPFR::printLine(const QByteArray &aString) {
    uint tags = ASCII::NUL;

    for (auto it = CAFPFR::Tags.data().begin(); it != CAFPFR::Tags.data().end(); ++it) {
        if (m_LineTags.contains(it.key())) {
            tags |= uchar(it.value());
        }
    }

    return processCommand(CAFPFR::Commands::PrintLine, CAFPFR::TData() << aString << tags);
}

//--------------------------------------------------------------------------------
bool AFPFR::cut() {
    return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::openDocument(char aType) {
    CAFPFR::TData commandData = CAFPFR::TData() << aType << "" << "" << "";

    if (aType != CAFPFR::DocumentTypes::NonFiscal) {
        QString cashier = m_FFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
        commandData[2] = m_Codec->fromUnicode(cashier);
    }

    return processCommand(CAFPFR::Commands::OpenDocument, commandData);
}

//--------------------------------------------------------------------------------
bool AFPFR::closeDocument(bool aProcessing) {
    return processCommand(CAFPFR::Commands::CloseDocument,
                          CAFPFR::TData() << static_cast<int>(!aProcessing));
}

//--------------------------------------------------------------------------------
ESessionState::Enum AFPFR::getSessionState() {
    CAFPFR::TData answerData = 0;

    if (!processCommand(CAFPFR::Commands::GetFRStatus, &answerData)) {
        return ESessionState::Error;
    }

    char flags = char(answerData[0].toInt());

    if ((~flags & CAFPFR::SessionOpened) != 0) {
        return ESessionState::Closed;
    }
    if ((flags & CAFPFR::SessionExpired) != 0) {
        return ESessionState::Expired;
    }

    return ESessionState::Opened;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum AFPFR::getDocumentState() {
    CAFPFR::TData answerData = 0;

    if (!processCommand(CAFPFR::Commands::GetFRStatus, &answerData)) {
        return EDocumentState::Error;
    }

    return answerData[1].toInt() ? EDocumentState::Opened : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
bool AFPFR::setNotPrintDocument(bool aEnabled, bool /*aZReport*/) {
    return setFRParameter(CAFPFR::FRParameters::NotPrintDocument, aEnabled);
}

//--------------------------------------------------------------------------------
bool AFPFR::openSession() {
    QString cashier;

    if (m_OperatorPresence && !m_FFEngine.checkCashier(cashier)) {
        return false;
    }

    checkNotPrinting(true);
    bool result = processCommand(CAFPFR::Commands::OpenSession, CAFPFR::TData() << cashier);
    checkNotPrinting();

    return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::processXReport() {
    return processCommand(CAFPFR::Commands::XReport);
}

//--------------------------------------------------------------------------------
bool AFPFR::perform_ZReport(bool /*aPrintDeferredReports*/) {
    return execZReport(false);
}

//--------------------------------------------------------------------------------
bool AFPFR::execZReport(bool aAuto) {
    toLog(LogLevel::Normal,
          m_DeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
    ESessionState::Enum sessionState = getSessionState();

    if (sessionState == ESessionState::Error) {
        return false;
    }
    if (sessionState == ESessionState::Closed) {
        return true;
    }

    bool needCloseSession = sessionState == ESessionState::Expired;

    if (aAuto) {
        if (m_OperatorPresence) {
            toLog(LogLevel::Error,
                  m_DeviceName +
                      ": Failed to process auto-Z-report due to presence of the operator.");
            m_NeedCloseSession = m_NeedCloseSession || needCloseSession;

            return false;
        }
        if (!checkNotPrinting(aAuto, true)) {
            m_NeedCloseSession = m_NeedCloseSession || needCloseSession;

            return false;
        }
    }

    QVariantMap outData = getSessionOutData();

    bool result = processCommand(CAFPFR::Commands::ZReport);
    m_NeedCloseSession = getSessionState() == ESessionState::Expired;

    if (!result) {
        toLog(LogLevel::Error, m_DeviceName + ": error in processing Z-report");
        return false;
    }

    emit frSessionClosed(outData);

    toLog(LogLevel::Normal, m_DeviceName + ": Z-report is successfully processed");

    return true;
}

//--------------------------------------------------------------------------------
double AFPFR::getAmountInCash() {
    CAFPFR::TData data = 0;

    return getFRData(CAFPFR::FRInfo::TotalCash, data) ? data[0].toDouble() : -1;
}

//--------------------------------------------------------------------------------
bool AFPFR::processPayout(double aAmount) {
    if (!openDocument(CAFPFR::DocumentTypes::PayOut)) {
        return false;
    }

    ExitAction exitAction([&]() { processCommand(CAFPFR::Commands::CancelDocument); });

    return processCommand(CAFPFR::Commands::PayIO, CAFPFR::TData() << "" << aAmount) &&
           closeDocument(true) && exitAction.reset();
}

//--------------------------------------------------------------------------------
static QVariantMap AFPFR::getSessionOutData() {
    return QVariantMap();
}

//--------------------------------------------------------------------------------
void AFPFR::setErrorFlags() {}

//--------------------------------------------------------------------------------
