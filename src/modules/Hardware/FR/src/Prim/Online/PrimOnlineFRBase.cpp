/* @file Онлайн ФР семейства ПРИМ. */

#include <QtCore/qmath.h>

#include "Prim_OnlineFRBase.h"
#include "Prim_OnlineFRConstants.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrim_FR {
inline TModels OnlineModels() {
    return TModels();
}
} // namespace CPrim_FR

//--------------------------------------------------------------------------------
Prim_OnlineFRBase::Prim_OnlineFRBase() {
    // данные устройства
    m_IsOnline = true;
    m_AFDFont = CPrim_FR::FiscalFont::Default;

    // данные моделей
    m_DeviceName = CPrim_FR::DefaultOnlineModelName;
    m_Models = CPrim_FR::OnlineModels();
    m_Model = CPrim_FR::Models::OnlineUnknown;

    // типы оплаты
    m_PayTypeData.data().clear();

    // команды
    m_CommandTimouts.append(CPrim_OnlineFR::Commands::GetFSStatus, 2 * 1000);
    m_CommandTimouts.append(CPrim_OnlineFR::Commands::GetFiscalTLVData, 5 * 1000);

    setConfigParameter(CHardwareSDK::CanOnline, true);

    // ошибки
    m_ErrorData = PErrorData(new CPrim_OnlineFR::Errors::Data());
    m_ExtraErrorData = PExtraErrorData(new CPrim_OnlineFR::Errors::ExtraData());
}

//--------------------------------------------------------------------------------
QStringList Prim_OnlineFRBase::getModelList() {
    return CPrim_FR::getModelList(CPrim_FR::OnlineModels());
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::updateParameters() {
    TStatusCodes statusCodes;
    CPrim_FR::TData data;

    if (!getStatusInfo(statusCodes, data) || (data.size() < 12)) {
        return false;
    }

    m_Serial = CFR::serialToString(data[7]);
    m_RNM = CFR::RNMToString(data[8]);
    m_INN = CFR::INNToString(data[9]);

    m_FSSerialNumber = CFR::FSSerialToString(data[11]);

    QString firmware = data[10].simplified();
    setDeviceParameter(CDeviceData::Firmware, firmware);

    if (firmware.size() == 7) {
        QString textBuild = QString("%1.%2").arg(firmware.right(3)).arg(firmware[2]);
        double build = textBuild.toDouble();

        if (build) {
            m_FFDFR = CPrim_OnlineFR::getFFD(build);
            double actualFirmware = CPrim_OnlineFR::getActualFirmware(m_FFDFR);
            m_OldFirmware = actualFirmware && (actualFirmware > build);

            // TODO: TLV-запрос для получения версии ФФД ФН -> m_FFDFS (тег 1190), будет после 1.05
            m_FFDFS = EFFD::F10;
        }

        setDeviceParameter(CDeviceData::ControllerBuild, textBuild, CDeviceData::Firmware);
        int DTDBuild = CPrim_OnlineFR::getDTD(firmware, m_FFDFR);

        if (DTDBuild) {
            setDeviceParameter(CDeviceData::FR::DTDBuild, DTDBuild, CDeviceData::Firmware);
        }

        QString codes = CPrim_OnlineFR::getCodes(firmware);

        if (!codes.isEmpty()) {
            setDeviceParameter(CDeviceData::Printers::Codes, codes, CDeviceData::Firmware);
        }

        setDeviceParameter(
            CDeviceData::Printers::PNESensor, firmware[3] != ASCII::Zero, CDeviceData::Firmware);
    }

    processDeviceData();

    if (m_FFDFR > EFFD::F10) {
        getRegTLVData(CFR::FiscalFields::LotteryMode);
        getRegTLVData(CFR::FiscalFields::GamblingMode);
        getRegTLVData(CFR::FiscalFields::ExcisableUnitMode);
        getRegTLVData(CFR::FiscalFields::InAutomateMode);
    }

    if ((!m_OperatorPresence && !checkParameters()) || !checkControlSettings()) {
        return false;
    }

    if (!isFiscal()) {
        return true;
    }

    CPrim_FR::TData commandData = CPrim_FR::TData()
                                  << CPrim_FR::DontPrintFD << CPrim_OnlineFR::LastRegistration;

    if (!processCommand(CPrim_OnlineFR::Commands::GetRegistrationTotal, commandData, &data)) {
        return false;
    }

    uchar taxSystem_Data;
    uchar operationModeData;

    if (!parseAnswerData(data, 8, "tax systems", taxSystem_Data) ||
        !checkTaxSystems(char(taxSystem_Data)) ||
        !parseAnswerData(data, 9, "operation modes", operationModeData) ||
        !checkOperationModes(char(operationModeData))) {
        return false;
    }

    if (m_FFDFR > EFFD::F10) {
        QString FSVersion = getDeviceParameter(CDeviceData::FS::Version).toString();
        bool canCheckAgentFlags = (m_LastError != CPrim_OnlineFR::Errors::NoRequiedData) &&
                                  !FSVersion.contains(CPrim_OnlineFR::FSNoAgentFlags);
        uchar FFData;

        if (canCheckAgentFlags && getRegTLVData(CFR::FiscalFields::AgentFlagsReg, FFData) &&
            !checkAgentFlags(char(FFData))) {
            return false;
        }
    }

    return checkTaxes() && checkPayTypes();
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::checkPayTypes() {
    m_PayTypeData.data().clear();

    for (int i = 0; i < CPrim_OnlineFR::PayTypeAmount; ++i) {
        CPrim_FR::TData data;

        if (!processCommand(CPrim_OnlineFR::Commands::GetPayTypeData,
                            CPrim_FR::TData() << int2String(i).toLatin1(),
                            &data)) {
            return false;
        }

        ushort field;

        if (parseAnswerData(data, 12, "pay type", field)) {
            if (!m_FFData.data().contains(field)) {
                toLog(LogLevel::Warning,
                      m_DeviceName + ": Unknown fiscal field of pay type " +
                          QString::number(field));
            } else if (!CPrim_OnlineFR::PayTypeData.data().contains(field)) {
                toLog(LogLevel::Warning,
                      m_DeviceName + ": Unknown fiscal field of pay type" + QString::number(field));
            } else {
                m_PayTypeData.add(CPrim_OnlineFR::PayTypeData[field], i);
            }
        }
    }

    if (m_PayTypeData.data().isEmpty()) {
        toLog(LogLevel::Error, m_DeviceName + ": Pay type data is empty");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::getRegTLVData(int aField, uchar &aData) {
    CPrim_FR::TData commandData = CPrim_FR::TData()
                                  << QByteArray::number(qToBigEndian(ushort(aField)), 16);

    return processCommand(CPrim_OnlineFR::Commands::GetRegTLVData,
                          commandData,
                          5,
                          m_FFData.getTextLog(aField),
                          aData);
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::getRegTLVData(int aField) {
    uchar FFData;

    if (!getRegTLVData(aField, FFData)) {
        return false;
    }

    m_FFEngine.setConfigParameter(m_FFData[aField].textKey, FFData);
    toLog(LogLevel::Normal,
          m_DeviceName +
              QString(": Add %1 = %2 to config data").arg(m_FFData.getTextLog(aField)).arg(FFData));

    return true;
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::getStatus(TStatusCodes &aStatusCodes) {
    CPrim_FR::TData data;

    if (!Prim_FRBase::getStatus(aStatusCodes)) {
        return false;
    }

    ushort OFDNotSentCount;
    TResult result = processCommand(CPrim_OnlineFR::Commands::GetOFDNotSentCount,
                                    5,
                                    "OFD not sent fiscal documents count",
                                    OFDNotSentCount);

    if (!CORRECT(result)) {
        return false;
    } else if (result == CommandResult::Device) {
        int statusCode = getErrorStatusCode(m_ErrorData->value(m_LastError).type);
        aStatusCodes.insert(statusCode);
    } else if (result == CommandResult::Answer) {
        aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
    } else {
        checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);
    }

    return true;
}

//--------------------------------------------------------------------------------
void Prim_OnlineFRBase::processDeviceData() {
    CPrim_FR::TData data;

    removeDeviceParameter(CDeviceData::FR::Session);

    if (processCommand(CPrim_OnlineFR::Commands::GetFSStatus, &data)) {
        uchar sessionStateData;

        if (parseAnswerData(data, 8, "session state", sessionStateData)) {
            QString sessionState =
                sessionStateData ? CDeviceData::Values::Opened : CDeviceData::Values::Closed;
            setDeviceParameter(CDeviceData::FR::Session, sessionState);
        }

        loadDeviceData<uint>(data, CDeviceData::FR::FiscalDocuments, "fiscal documents count", 12);

        if (data.size() > 13) {
            if (data[13].size() == 6) {
                data[13] = data[13].insert(4, "20");
            }

            QDate date = QDate::fromString(data[13], CFR::DateFormat);

            if (date.isValid()) {
                setDeviceParameter(CDeviceData::FS::ValidityData, CFR::FSValidityDateOff(date));
            }
        }

        if (data.size() > 15) {
            setDeviceParameter(CDeviceData::FS::Version, clean(data[14]).data());
            setDeviceParameter(CDeviceData::Type,
                               data[15].toUInt() ? "serial" : "debug",
                               CDeviceData::FS::Version);
        }

        loadDeviceData<ushort>(data, CDeviceData::Count, "count", 16, CDeviceData::FR::Session);
    }

    if (processCommand(CPrim_FR::Commands::GetStatus, &data)) {
        loadDeviceData<uchar>(
            data, CDeviceData::FR::FreeReregistrations, "free reregistrations", 5);

        if (data.size() > 8) {
            QString dateTimedata = data[8] + data[7];
            QDateTime dateTime =
                QDateTime::from_String(dateTimedata.insert(4, "20"), CPrim_FR::FRDateTimeFormat);

            if (dateTime.isValid()) {
                setDeviceParameter(CDeviceData::FR::OpeningDate,
                                   dateTime.toString(CFR::DateTimeLogFormat),
                                   CDeviceData::FR::Session);
            }
        }
    }

    checkDateTime();

    m_OFDDataError =
        !processCommand(CPrim_OnlineFR::Commands::GetOFDData, &data) || (data.size() <= 9) ||
        !checkOFDData(data[9], getBufferFrom_String(data[5].right(2) + data[5].left(2)));
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::perform_Fiscal(const QStringList &aReceipt,
                                       const SPaymentData &aPaymentData,
                                       quint32 *aFDNumber) {
    if (!Prim_FRBase::perform_Fiscal(aReceipt, aPaymentData)) {
        return false;
    }

    if (aFDNumber &&
        !processCommand(
            CPrim_OnlineFR::Commands::GetFSStatus, 12, "last document number in FS", *aFDNumber)) {
        aFDNumber = 0;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::getFiscalFields(quint32 aFDNumber,
                                        TFiscalPaymentData &aFPData,
                                        TComplexFiscalPaymentData &aPSData) {
    CPrim_FR::TData commandData =
        CPrim_FR::TData()
        << QString("%1").arg(qToBigEndian(aFDNumber), 8, 16, QLatin1Char(ASCII::Zero)).toLatin1()
        << int2ByteArray(CPrim_OnlineFR::FiscalTLVDataFlags::Start);

    if (!processCommand(CPrim_OnlineFR::Commands::GetFiscalTLVData, commandData)) {
        return false;
    }

    commandData[1] = int2ByteArray(CPrim_OnlineFR::FiscalTLVDataFlags::Get);
    CPrim_FR::TData answer;

    if (!processCommand(CPrim_OnlineFR::Commands::GetFiscalTLVData, commandData, &answer)) {
        return false;
    }

    QRegularExpression regExp(CPrim_OnlineFR::RegExpTLVData);

    for (int i = 5; i < answer.size(); ++i) {
        QString TLVData = m_Codec->toUnicode(answer[i]);

        if (regExp.match(TLVData).capturedStart() == -1) {
            toLog(LogLevel::Error,
                  m_DeviceName + QString(": Failed to parse TLV data %1 (%2)")
                                     .arg(TLVData)
                                     .arg(answer[i].toHex().data()));
            return false;
        }

        QStringList capturedData = regExp.capturedTexts();
        CFR::STLV TLV;

        bool OK;
        TLV.field = capturedData[1].toInt(&OK);

        if (!OK) {
            toLog(LogLevel::Error,
                  m_DeviceName + ": Failed to parse TLV field number " + capturedData[1]);
            return false;
        }

        TLV.data = m_Codec->fromUnicode(capturedData[2]);

        if (TLV.field == CFR::FiscalFields::PayOffSubject) {
            aPSData << TFiscalPaymentData();
        } else if (m_FFData.data().contains(TLV.field)) {
            if (!CFR::FiscalFields::PayOffSubjectFields.contains(TLV.field)) {
                setFPData(aFPData, TLV);
            } else if (!aPSData.isEmpty()) {
                setFPData(aPSData.last(), TLV);
            }
        } else {
            toLog(LogLevel::Warning,
                  QString("%1: Failed to add fiscal field %2 due to it is unknown")
                      .arg(m_DeviceName)
                      .arg(TLV.field));
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
void Prim_OnlineFRBase::setFPData(TFiscalPaymentData &aFPData, const CFR::STLV &aTLV) {
    QString data = m_Codec->toUnicode(aTLV.data);
    CFR::FiscalFields::SData FFData = m_FFData[aTLV.field];

    if (FFData.isTime()) {
        QDateTime dateTime =
            QDateTime::from_String(data.insert(6, "20"), CPrim_OnlineFR::FFDateTimeFormat);
        m_FFEngine.setFPData(aFPData, aTLV.field, dateTime);
    } else if (!FFData.isMoney()) {
        m_FFEngine.setFPData(aFPData, aTLV.field, data);
    } else {
        bool OK;
        double value = data.toDouble(&OK);

        if (OK) {
            m_FFEngine.setFPData(aFPData, aTLV.field, qulonglong(100 * value));
        } else {
            toLog(LogLevel::Warning,
                  m_DeviceName + QString(": Failed to parse money data %1 for field %2")
                                     .arg(data)
                                     .arg(aTLV.field));
        }
    }
}

//--------------------------------------------------------------------------------
void Prim_OnlineFRBase::setFiscalData(CPrim_FR::TData &aCommandData,
                                      CPrim_FR::TDataList &aAdditionalAFDData,
                                      const SPaymentData &aPaymentData,
                                      int aReceiptSize) {
    QString serialNumber = getConfigParameter(CHardware::FR::Strings::SerialNumber).toString();
    QString documentNumber = getConfigParameter(CHardware::FR::Strings::DocumentNumber).toString();
    QString INN = getConfigParameter(CHardware::FR::Strings::INN).toString();
    QString cashier = getConfigParameter(CHardware::FR::Strings::Cashier).toString();
    QString session = getConfigParameter(CHardware::FR::Strings::Session).toString();
    QString receiptNumber = getConfigParameter(CHardware::FR::Strings::ReceiptNumber).toString();
    QString total = getConfigParameter(CHardware::FR::Strings::Total).toString();

    auto getAmountData = [&](const TSum &aSum) -> QByteArray {
        return QString::number(qRound64(aSum * 100.0) / 100.0, ASCII::Zero, 2).toLatin1();
    };
    auto getDataY = [&](int aSize) -> int {
        return CPrim_OnlineFR::AFD::LineSize::GField - aSize - 1;
    };
    auto getAmountY = [&](const TSum &aSum) -> int { return getDataY(getAmountData(aSum).size()); };

    QByteArray sum_Data = getAmountData(getTotalAmount(aPaymentData));
    QByteArray FDType = CPrim_FR::PayOffTypeData[aPaymentData.payOffType];

    QString cashierValue = m_FFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
    QString cashierINN =
        CFR::INNToString(m_FFEngine.getConfigParameter(CFiscalSDK::CashierINN).toByteArray());
    QString operatorId = CPrim_FR::OperatorID;

    if (!cashierValue.isEmpty()) {
        operatorId = cashierValue;

        if (!cashierINN.isEmpty() && (m_FFDFR > EFFD::F10)) {
            operatorId += "|" + cashierINN;
        }
    }

    int serialNumberY = serialNumber.size() + 2;
    int documentNumberY = getDataY(5);
    int timeY = getDataY(5);
    bool cashierExist = operatorId != CPrim_FR::OperatorID;
    int cashierX = cashierExist ? 3 : 2;

    int startX = aReceiptSize;
    int lastX = startX + cashierX;

    int INNY = INN.size() + 2;
    int cashierY = cashierExist ? (cashier.size() + 2) : (INNY + 16);
    int totalY = getAmountY(getTotalAmount(aPaymentData));
    int receiptNumberY = getDataY(5);

    aCommandData << addGFieldToBuffer(startX + 1, serialNumberY, m_AFDFont)   // серийный номер
                 << addGFieldToBuffer(startX + 1, documentNumberY, m_AFDFont) // номер документа
                 << addGFieldToBuffer(startX + 2, 1, m_AFDFont)               // дата
                 << addGFieldToBuffer(startX + 2, timeY, m_AFDFont)           // время
                 << addGFieldToBuffer(lastX + 2, INNY, m_AFDFont)             // ИНН
                 << addGFieldToBuffer(lastX + 0, cashierY, m_AFDFont)
                 << m_Codec->fromUnicode(operatorId)                            // ID оператора
                 << addGFieldToBuffer(lastX + 2, totalY, m_AFDFont) << sum_Data; // сумма

    aAdditionalAFDData
        << addArbitraryFieldToBuffer(startX + 1, 1, serialNumber, m_AFDFont)
        << addArbitraryFieldToBuffer(
               startX + 1, documentNumberY - (documentNumber.size() + 1), documentNumber, m_AFDFont)
        << addArbitraryFieldToBuffer(lastX + 2, 1, INN, m_AFDFont)
        << addArbitraryFieldToBuffer(lastX + 2, totalY - total.size() - 1, total, m_AFDFont)

        // 1038 (номер смены)
        << addArbitraryFieldToBuffer(lastX + 1, 1, session, m_AFDFont)
        << addFiscalField(
               lastX + 1, 2 + session.size(), m_AFDFont, CFR::FiscalFields::SessionNumber)

        // 1042 (номер чека)
        << addArbitraryFieldToBuffer(
               lastX + 1, receiptNumberY - receiptNumber.size() - 1, receiptNumber, m_AFDFont)
        << addFiscalField(lastX + 1, receiptNumberY, m_AFDFont, CFR::FiscalFields::DocumentNumber);

    if (cashierExist) {
        aAdditionalAFDData << addArbitraryFieldToBuffer(
            aReceiptSize + cashierX, 1, cashier, m_AFDFont);
    }

    // продажи
    lastX += 2;

    for (int i = 0; i < aPaymentData.unitDataList.size(); ++i) {
        SUnitData unitData = aPaymentData.unitDataList.value(i);
        int section = (unitData.section != -1) ? unitData.section : 1;
        QStringList data =
            QStringList()
            << unitData.name                                      // 1059 (товар)
            << getAmountData(unitData.sum)                        // 1079 (цена)
            << "1"                                                // 1023 (количество)
            << int2String(section) +                              // отдел
                   int2String(m_TaxData[unitData.VAT].group) +    // налоговая группа
                   int2String(unitData.payOffSubjectMethodType) + // 1214 (признак способа расчета)
                   int2String(unitData.payOffSubjectType)         // 1212 (признак предмета расчета)
            << "";

        int addAFDDataIndex = aAdditionalAFDData.size();

#define ADD_AFD_TAG(aX, aY, aField, ...)                                                           \
    aAdditionalAFDData << addFiscalField(                                                          \
        lastX + aX, aY, m_AFDFont, CFR::FiscalFields::aField * 100 + i + 1, __VA_ARGS__)
#define ADD_AFD_TAG_MULTI(aX, aY, aField, aData)                                                   \
    ADD_AFD_TAG(aX, aY, aField, aData);                                                            \
    lastX += int(newLine)

        if (m_FFDFR > EFFD::F10) {
            // координаты

            TSum sum = unitData.sum;
            int amountY = getAmountY(sum);
            int VATY = getAmountY(sum * unitData.VAT / 100.0);

            int unitNameSize = unitData.name.size();
            int unitRest = (unitNameSize % CPrim_OnlineFR::AFD::LineSize::GField) +
                           int(unitNameSize > CPrim_OnlineFR::AFD::LineSize::Unit);
            bool newLine = (amountY - unitRest) < 4;

            ADD_AFD_TAG_MULTI(1, 1, PayOffSubject, data.join("|")); // 1059 (товар)
            ADD_AFD_TAG(1, amountY, PayOffSubjectUnitPrice);        // 1079 (цена)
            ADD_AFD_TAG(2, 1, PayOffSubjectQuantity);               // 1023 (количество)
            ADD_AFD_TAG(2, amountY, PayOffSubjectAmount);           // 1043 (стоимость)
            ADD_AFD_TAG(3, 1, VATRate);                             // 1199 (НДС, %)
            ADD_AFD_TAG(3, VATY, PayOffSubjectTaxAmount);           // 1200 (НДС, сумма)

            if (unitData.payOffSubjectMethodType != EPayOffSubjectMethodTypes::Full) {
                ADD_AFD_TAG(4, 1, PayOffSubjectMethodType); // 1214 (признак способа расчета)
            }

            // не печатается на основании ФФД
            // ADD_AFD_TAG(5, 1, PayOffSubjectType);    // 1212 (признак предмета расчета)
        } else {
            aAdditionalAFDData << addFiscalField(lastX + 1,
                                                 1,
                                                 m_AFDFont,
                                                 CFR::FiscalFields::UnitName,
                                                 data.join("|")); // 1030 (наименование товара)
        }

        foreach (auto AFDData, aAdditionalAFDData.mid(addAFDDataIndex)) {
            int newLastX = qToBigEndian(AFDData[0].toUShort(0, 16));
            lastX = qMax(lastX, newLastX);
        }
    }

    bool agentFlagExists = (m_FFDFR > EFFD::F10) && (aPaymentData.agentFlag != EAgentFlags::None);

    // 1055 (СНО)
    if (aPaymentData.taxSystem != ETaxSystems::None) {
        int index = 0;

        while (~aPaymentData.taxSystem & (1 << index++)) {
        }

        int taxSystem_Y = agentFlagExists ? 25 : 1;
        aAdditionalAFDData << addFiscalField(
            lastX + 1, taxSystem_Y, m_AFDFont, CFR::FiscalFields::TaxSystem, int2String(index));
    }

#define ADD_PRIM_FF_DATA(aField, aData)                                                            \
    aAdditionalAFDData.append(                                                                     \
        addFiscalField(++lastX, 1, m_AFDFont, CFR::FiscalFields::aField, aData))
#define ADD_PRIM_FF(aField)                                                                        \
    ADD_PRIM_FF_DATA(aField, m_FFEngine.getConfigParameter(CFiscalSDK::aField).toString())

    // 1057 (флаг агента)
    if (agentFlagExists) {
        ADD_PRIM_FF_DATA(AgentFlagsReg, int2String(uchar(aPaymentData.agentFlag)));

        bool isBankAgent = CFR::isBankAgent(aPaymentData.agentFlag);
        bool isPaymentAgent = CFR::isPaymentAgent(aPaymentData.agentFlag);

        if (isBankAgent) {
            ADD_PRIM_FF(AgentOperation);
            ADD_PRIM_FF(TransferOperatorName);
            ADD_PRIM_FF(TransferOperatorINN);
            ADD_PRIM_FF(TransferOperatorAddress);
            ADD_PRIM_FF(TransferOperatorPhone);
        } else if (isPaymentAgent) {
            ADD_PRIM_FF(ProcessingPhone);
        }

        if (isBankAgent || isPaymentAgent) {
            ADD_PRIM_FF(AgentPhone);
            ADD_PRIM_FF(ProviderPhone);
        }
    }

    QString userContact = m_FFEngine.getConfigParameter(CFiscalSDK::UserContact).toString();

    if (!userContact.isEmpty()) {
        if (m_FFDFR >= EFFD::F105) {
            ADD_PRIM_FF(UserContact);
        } else {
            toLog(LogLevel::Warning,
                  m_DeviceName +
                      QString(": Failed to transmit %1 due to FFD version = %2, need %3 min")
                          .arg(m_FFData.getTextLog(CFR::FiscalFields::UserContact))
                          .arg(CFR::FFD[m_FFDFR].description)
                          .arg(CFR::FFD[EFFD::F105].description));
        }
    }
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::perform_ZReport(bool /*aPrintDeferredReports*/) {
    return execZReport(false);
}

//--------------------------------------------------------------------------------
TResult Prim_OnlineFRBase::doZReport(bool aAuto) {
    CPrim_FR::TData commandData = CPrim_FR::TData() << CPrim_FR::OperatorID << ""
                                                    << ""; // + сообщение для ОФД и доп. реквизит
    commandData << (aAuto ? CPrim_OnlineFR::ZReportInBuffer : CPrim_OnlineFR::ZReportOut);

    return processCommand(CPrim_FR::Commands::ZReport, commandData);
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::openSession() {
    CPrim_FR::TData commandData = CPrim_FR::TData()
                                  << CPrim_FR::OperatorID << "" << ""
                                  << ""; // + сообщение оператору, доп. реквизит и реквизиты смены

    return processCommand(CPrim_FR::Commands::OpenFRSession, commandData);
}

//--------------------------------------------------------------------------------
bool Prim_OnlineFRBase::processAnswer(char aError) {
    if (aError == CPrim_OnlineFR::Errors::FSOfflineEnd) {
        m_ProcessingErrors.push_back(m_LastError);

        m_FSOfflineEnd = true;
    }

    return Prim_FRBase::processAnswer(aError);
}

//--------------------------------------------------------------------------------
int Prim_OnlineFRBase::getVerificationCode() {
    int result;

    if (!processCommand(
            CPrim_OnlineFR::Commands::GetFSStatus, 12, "last document number in FS", result)) {
        return 0;
    }

    return result;
}

//--------------------------------------------------------------------------------
CPrim_FR::TData Prim_OnlineFRBase::addFiscalField(
    int aX, int aY, int aFont, int aFiscalField, const QString &aData) {
    return addArbitraryFieldToBuffer(aX, aY, QString("<%1>").arg(aFiscalField) + aData, aFont);
}

//--------------------------------------------------------------------------------
