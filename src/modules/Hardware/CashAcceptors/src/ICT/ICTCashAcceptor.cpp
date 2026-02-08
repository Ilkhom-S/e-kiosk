/* @file Купюроприемник на протоколе ICT. */

#include "ICTCashAcceptor.h"

#include "ICTCashAcceptorConstants.h"
#include "ICTModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
ICTCashAcceptor::ICTCashAcceptor() {
    // параметры порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::Even);

    m_IOMessageLogging = ELoggingType::ReadWrite;

    // данные устройства
    m_DeviceName = "ICT cash acceptor";

    // параметры протокола
    m_DeviceCodeSpecification = PDeviceCodeSpecification(new CICTBase::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList ICTCashAcceptor::getModelList() {
    return CICT::ModelData().data().keys();
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::processReset() {
    if (m_OldFirmware) {
        toLog(LogLevel::Warning,
              m_DeviceName + ": protocol version ICT002, Reset is out of keeping");
        return true;
    }

    QByteArray answer;

    if (!m_IOPort->write(QByteArray(1, CICTBase::Commands::Reset))) {
        return false;
    }

    SleepHelper::msleep(CICTBase::ResetTimeout);

    if (!m_IOPort->read(answer, 100)) {
        return false;
    }

    if (answer.isEmpty()) {
        toLog(LogLevel::Warning,
              m_DeviceName +
                  ": There is no answer from peripheral device. Perhaps, this is protocol ICT002");
        m_OldFirmware = true;

        return true;
    }

    if (!answer.contains(CICTBase::States::PowerUp)) {
        toLog(LogLevel::Error, m_DeviceName + ": Invalid response");
        return false;
    }

    return answerToReset();
}

//---------------------------------------------------------------------------------
bool ICTCashAcceptor::checkStatus(QByteArray &aAnswer) {
    if (!m_IOPort->read(aAnswer, 100)) {
        return false;
    }

    auto poll = [&]() -> bool {
        return m_IOPort->write(QByteArray(1, CICTBase::Commands::Poll)) &&
               m_IOPort->read(aAnswer, 100) && !aAnswer.isEmpty();
    };

    if (aAnswer.isEmpty()) {
        if (!poll()) {
            return false;
        }

        if (aAnswer[0] == CICTBase::States::Disabled) {
            if (!enableMoneyAcceptingMode(true) || !poll()) {
                return false;
            }

            if ((aAnswer.size() > 1) || (aAnswer[0] != CICTBase::States::Idling)) {
                aAnswer = aAnswer.replace(CICTBase::States::Idling, "");
            }

            enableMoneyAcceptingMode(false);

            if (!poll()) {
                return false;
            }
        }
    }

    if (aAnswer.contains(CICTBase::States::PowerUp)) {
        answerToReset();
    }

    // Если купюрник вышел из ошибки, то чистим буфер ответа, спрашиваем еще раз статус и выходим
    if (aAnswer.contains(CICTBase::States::ErrorExlusion) && !poll()) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to poll after error exlusion");
        return false;
    }

    int escrowIndex = aAnswer.indexOf(CICTBase::States::Escrow);

    if (escrowIndex != -1) {
        m_EscrowPosition = escrowIndex + 1;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::isConnected() {
    if (!m_IOPort->write(CICTBase::Commands::Identification)) {
        return false;
    }

    SleepHelper::msleep(CICTBase::ResetTimeout);
    QByteArray answer;

    if (!m_IOPort->read(answer, 100)) {
        return false;
    }

    if (answer.size() < 3) {
        toLog(LogLevel::Error,
              m_DeviceName + ": Perhaps unknown device trying to impersonate any ICT device");
        return false;
    }

    CICTBase::DeviceCodeSpecification *specification =
        m_DeviceCodeSpecification.dynamicCast<CICTBase::DeviceCodeSpecification>().data();

    if ((answer.right(3) != CICTBase::Answers::Identification) &&
        !((answer.size() > 2) && (answer[0] == answer[1]) && (answer[0] == answer[2]) &&
          specification->contains(answer[0]))) {
        return false;
    }

    QString configModelName;

    if (!isAutoDetecting()) {
        configModelName = getConfigParameter(CHardwareSDK::ModelName).toString();
    }

    m_DeviceName = configModelName.isEmpty() ? "ICT U70" : configModelName;
    m_Verified = CICT::ModelData()[m_DeviceName];

    return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::answerToReset() {
    if (!m_IOPort->write(QByteArray(1, CICTBase::Commands::ACK))) {
        return false;
    }

    SleepHelper::msleep(CICTBase::ResetTimeout);
    enableMoneyAcceptingMode(false);
    SleepHelper::msleep(CICTBase::ResetTimeout);

    return true;
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::stack() {
    if (!checkConnectionAbility() || (m_Initialized != ERequestStatus::Success) || m_CheckDisable) {
        return false;
    }

    return m_IOPort->write(QByteArray(1, CICTBase::Commands::ACK));
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::reject() {
    if (!checkConnectionAbility() || (m_Initialized == ERequestStatus::Fail)) {
        return false;
    }

    return m_IOPort->write(QByteArray(1, CICTBase::Commands::NAK));
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::enableMoneyAcceptingMode(bool aEnabled) {
    char command = aEnabled ? CICTBase::Commands::Enable : CICTBase::Commands::Disable;

    return m_IOPort->write(QByteArray(1, command));
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::loadParTable() {
    m_EscrowParTable.add(0x40, SPar(10, Currency::RUB));
    m_EscrowParTable.add(0x41, SPar(50, Currency::RUB));
    m_EscrowParTable.add(0x42, SPar(100, Currency::RUB));
    m_EscrowParTable.add(0x43, SPar(500, Currency::RUB));
    m_EscrowParTable.add(0x44, SPar(1000, Currency::RUB));

    return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::isStatusesReplaceable(TStatusCodes &aStatusCodes) {
    TStatusCodes errors = m_StatusCollection.value(EWarningLevel::Error);
    auto check = [&errors, &aStatusCodes](int statusCode) -> bool {
        return aStatusCodes.contains(statusCode) && !errors.contains(statusCode);
    };

    foreach (int statusCode, aStatusCodes) {
        if ((m_StatusCodesSpecification->value(statusCode).warningLevel == EWarningLevel::Error) &&
            check(statusCode)) {
            return true;
        }
    }

    return TSerialCashAcceptor::isStatusesReplaceable(aStatusCodes);
}

//--------------------------------------------------------------------------------
void ICTCashAcceptor::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                        const TStatusCollection &aOldStatusCollection) {
    if (isPowerReboot() && m_OldFirmware) {
        TStatusCodes statusCodes;
        auto poll = [&]() -> bool { return getStatus(std::ref(statusCodes)); };

        CICTBase::DeviceCodeSpecification *specification =
            m_DeviceCodeSpecification.dynamicCast<CICTBase::DeviceCodeSpecification>().data();
        auto isPowerUp = [&]() -> bool {
            return std::find_if(m_DeviceCodeBuffers.begin(),
                                m_DeviceCodeBuffers.end(),
                                [&](const QByteArray &aBuffer) -> bool {
                                    return specification->isPowerUp(aBuffer);
                                }) != m_DeviceCodeBuffers.end();
        };

        PollingExpector().wait<bool>(poll, isPowerUp, CICTBase::PowerUpWaiting);
    }

    TSerialCashAcceptor::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
