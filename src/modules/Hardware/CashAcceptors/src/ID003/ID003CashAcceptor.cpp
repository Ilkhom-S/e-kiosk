/* @file Купюроприемник на протоколе ID003. */

#include "ID003CashAcceptor.h"

#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>

#include "CurrencySpecification.h"
#include "Hardware/CashAcceptors/CashAcceptorData.h"
#include "ID003CashAcceptorConstants.h"
#include "ID003ModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
ID003CashAcceptor::ID003CashAcceptor() {
    // параметры порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

    m_PortParameters[EParameters::Parity].append(EParity::Even);

    m_PortParameters[EParameters::RTS].clear();
    m_PortParameters[EParameters::RTS].append(ERTSControl::Disable);

    // данные устройства
    m_DeviceName = "ID003 cash acceptor";
    m_EscrowPosition = 1;

    // параметры протокола
    m_DeviceCodeSpecification = PDeviceCodeSpecification(new CID003::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList ID003CashAcceptor::getModelList() {
    QStringList result;

    foreach (CID003::SModelData aData, CID003::ModelData().data().values()) {
        result << aData.name;

        foreach (const SBaseModelData &aModelData, aData.models.data().values()) {
            result << aModelData.name;
        }
    }

    result << CID003::Models::GPTAurora << CID003::Models::JCMIPRO << CID003::Models::JCMVega;
    result.removeAll("");
    result.removeDuplicates();
    result.sort();

    return result;
}

//---------------------------------------------------------------------------------
bool ID003CashAcceptor::checkStatus(QByteArray &aAnswer) {
    if (!processCommand(CID003::Commands::StatusRequest, &aAnswer)) {
        return false;
    }

    CID003::DeviceCodeSpecification *specification =
        m_DeviceCodeSpecification.dynamicCast<CID003::DeviceCodeSpecification>().data();

    if (specification->isNeedACK(aAnswer) && !m_Protocol.sendACK()) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to send ACK message");
    }

    return true;
}

//---------------------------------------------------------------------------------
TResult ID003CashAcceptor::execCommand(const QByteArray &aCommand,
                                       const QByteArray &aCommandData,
                                       QByteArray *aAnswer) {
    MutexLocker locker(&m_ExternalMutex);

    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QByteArray answer;
    QByteArray request = aCommand + aCommandData;
    TResult result = m_Protocol.processCommand(request, answer);

    if (!result) {
        return result;
    }

    using namespace CID003::Commands;

    EAnswerType::Enum answerType = Data[aCommand[0]];

    if (answerType == EAnswerType::ACK) {
        if (answer[0] != CID003::ACK) {
            toLog(LogLevel::Error, m_DeviceName + ": Answer doesn't contain ACK  message");
            return CommandResult::Answer;
        }
    } else if (answerType == EAnswerType::Echo) {
        if (!answer.endsWith(request)) {
            toLog(LogLevel::Error,
                  m_DeviceName + ": Echo answer data doesn't match with command request");
            return CommandResult::Answer;
        }
    } else if (answerType == EAnswerType::Answer) {
        if (aCommand[0] != CID003::Commands::StatusRequest) {
            answer.remove(0, 1);
        }

        if (aAnswer) {
            *aAnswer = answer;
        }
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool ID003CashAcceptor::processReset() {
    return processCommand(CID003::Commands::Reset);
}

//--------------------------------------------------------------------------------
bool ID003CashAcceptor::isConnected() {
    if (isAutoDetecting()) {
        SleepHelper::msleep(CID003::IdentificationPause);
    }

    removeDeviceParameter(CDeviceData::Firmware);

    if (!waitReady(CID003::AvailableWaiting)) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Failed to wait any available status from the cash acceptor.");
        return false;
    }

    QByteArray answerData;

    if (!processCommand(CID003::Commands::VersionRequest, &answerData)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to reset the cash acceptor.");
        return false;
    }

    if (answerData.isEmpty() && isAutoDetecting()) {
        toLog(LogLevel::Error, m_DeviceName + ": Unknown device trying to impersonate this device");
        return false;
    }

    QString answer = QString(answerData).trimmed() + ASCII::Space;
    m_Verified = false;

    if (answer.contains(CID003::ProtocolData::GPTAurora)) {
        m_DeviceName = CID003::Models::GPTAurora;

        return true;
    }

    CID003::ProtocolData::CIdentification identificationData;

    foreach (const QString &regExpData, identificationData.data().values()) {
        QRegularExpression regExp(regExpData);
        QRegularExpressionMatch match = regExp.match(answer);

        if (match.capturedStart() != -1) {
            QString alias = identificationData.key(regExpData);

            if (alias != CID003::ProtocolData::Alias::ID003) {
                setDeviceParameter(CDeviceData::CashAcceptors::Alias, alias);
            }

            QStringList modelParameters = match.capturedTexts();

            char modelCode = modelParameters[0][0].toLatin1();
            QString countryCode = modelParameters[2];
            QString modelNumber = modelParameters[3];
            QString stackerType = modelParameters[4];
            QString interfaceType = modelParameters[5];
            QString firmware = modelParameters[8];
            QString firmwareDate = modelParameters[11];
            // QString CRCCode       = modelParameters[12];

            if (alias == CID003::ProtocolData::Alias::OP003) {
                modelNumber += " " + stackerType;
                stackerType.clear();
            }

            if (alias == CID003::ProtocolData::Alias::ID003Ext) {
                firmware.insert(1, ASCII::Dot);
                firmware.insert(4, ASCII::Dash);
                firmware.insert(8, ASCII::Dash);
            }

            setDeviceParameter(CDeviceData::Version, firmware, CDeviceData::Firmware);
            setDeviceParameter(CDeviceData::Date, firmwareDate, CDeviceData::Firmware);

            setDeviceParameter(CDeviceData::Identity, answer);
            setDeviceParameter(CDeviceData::CashAcceptors::Interface, interfaceType);
            setDeviceParameter(CDeviceData::CashAcceptors::CountryCode, countryCode);
            setDeviceParameter(
                CDeviceData::ModelKey,
                QString("'%1' (%2)").arg(modelCode).arg(ProtocolUtils::toHexLog(modelCode)));

            if (!modelNumber.isEmpty()) {
                setDeviceParameter(CDeviceData::ModelNumber,
                                   QString("%1").arg(modelNumber, 2, QChar(ASCII::Zero)));
            }

            setDeviceParameter(CDeviceData::CashAcceptors::StackerType, stackerType);

            SBaseModelData modelData =
                CID003::ModelData().getData(modelCode, modelNumber, stackerType);

            if (modelNumber.size() == CID003::NewJCMModelDataCount) {
                if (modelData.name == CID003::Models::JCMUBA) {
                    modelData = CID003::SModelData(CID003::Models::JCMIPRO, true);
                } else if (modelData.name == CID003::Models::CashcodeMVU) {
                    modelData = CID003::SModelData(CID003::Models::JCMVega, true);
                }
            }

            m_DeviceName = modelData.name;
            m_Verified = modelData.verified;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool ID003CashAcceptor::setDefaultParameters() {
    // устанавливаем режим связи с устройством
    if (!processCommand(CID003::Commands::SetComm_Mode, QByteArray(1, CID003::CommunicationMode))) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to set communication mode");
        return false;
    }

    // разрешаем принимать номиналы во всех направлениях
    if (!processCommand(CID003::Commands::SetDirections,
                        QByteArray(1, CID003::AllNoteDirections))) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to set nominals directions");
        return false;
    }
    /*
    // разрешаем принимать все номиналы с высоким уровнем контроля подлинности
    if (!processCommand(CID003::Commands::SetSecurities, CID003::HighSecurityLevel))
    {
            toLog(LogLevel::Error, m_DeviceName + ": Failed to set high nominals security");
            return false;
    }
    */

    return true;
}

//---------------------------------------------------------------------------
bool ID003CashAcceptor::stack() {
    if (!checkConnectionAbility() || (m_Initialized != ERequestStatus::Success) || m_CheckDisable) {
        return false;
    }

    return processCommand(CID003::Commands::Stack2);
}

//---------------------------------------------------------------------------
bool ID003CashAcceptor::reject() {
    if (!checkConnectionAbility() || (m_Initialized == ERequestStatus::Fail)) {
        return false;
    }

    return processCommand(CID003::Commands::Return);
}

//---------------------------------------------------------------------------
bool ID003CashAcceptor::enableMoneyAcceptingMode(bool aEnabled) {
    return processCommand(CID003::Commands::SetInhibits, QByteArray(1, char(!aEnabled)));
}

//---------------------------------------------------------------------------
bool ID003CashAcceptor::applyParTable() {
    int initEscrowPosition = m_EscrowParTable.data().begin().key();
    QByteArray commandData(2, ASCII::NUL);

    for (auto it = m_EscrowParTable.data().begin(); it != m_EscrowParTable.data().end(); ++it) {
        if (!it->enabled || it->inhibit) {
            int id = (it.key() - initEscrowPosition);
            int index = id / 8;
            commandData[index] = commandData[index] | (1 << id % 8);
        }
    }

    if (!processCommand(CID003::Commands::SetEnables, commandData)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to set nominals availability.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool ID003CashAcceptor::loadParTable() {
    QByteArray answer;

    if (!processCommand(CID003::Commands::GetBillTable, &answer)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to get par table");
        return false;
    }

    int nominalCount = answer.size() / CID003::NominalSize;

    for (int i = 0; i < nominalCount; ++i) {
        QByteArray parData = answer.mid(i * CID003::NominalSize, CID003::NominalSize);
        int nominal = int(uchar(parData[2]) * qPow(10, uchar(parData[3])));
        int currencyCode = CID003::CurrencyCodes[parData[1]];

        MutexLocker locker(&m_ResourceMutex);

        m_EscrowParTable.data().insert(uchar(parData[0]), SPar(nominal, currencyCode));
    }

    return true;
}

//--------------------------------------------------------------------------------
