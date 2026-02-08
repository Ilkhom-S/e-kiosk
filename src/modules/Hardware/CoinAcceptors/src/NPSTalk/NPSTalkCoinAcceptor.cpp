/* @file Монетоприемник на протоколе NPSTalk. */

#include "NPSTalkCoinAcceptor.h"

#include <QtCore/qmath.h>

#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"
#include "NPSTalkCoinAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
NPSTalkCoinAcceptor::NPSTalkCoinAcceptor() {
    // параметры порта
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // данные устройства
    m_DeviceName = "NPSTalk Comestero coin acceptor";
    m_MaxBadAnswers = 5;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::enableMoneyAcceptingMode(bool aEnabled) {
    return processCommand(aEnabled ? CNPSTalk::Command::Enable : CNPSTalk::Command::Disable);
}

//--------------------------------------------------------------------------------
TResult NPSTalkCoinAcceptor::execCommand(const QByteArray &aCommand,
                                         const QByteArray &aCommandData,
                                         QByteArray *aAnswer) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    QByteArray answer;
    TResult result = m_Protocol.processCommand(aCommand + aCommandData, answer);

    if (!result) {
        return result;
    }

    if (aAnswer) {
        *aAnswer = answer;
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
QStringList NPSTalkCoinAcceptor::getModelList() {
    return QStringList() << "Comestero Group RM5";
}

//---------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::processReset() {
    toLog(LogLevel::Normal, m_DeviceName + ": processing command reset");

    if (!processCommand(CNPSTalk::Command::Reset)) {
        toLog(LogLevel::Error, m_DeviceName + ": Failed to reset");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::getStatus(TStatusCodes &aStatusCodes) {
    // TDeviceCodes lastCodes(m_Codes); // ?
    m_Codes.clear();
    m_EscrowPars.clear();

    QByteArray answer;

    if (!processCommand(CNPSTalk::Command::GetStatus, answer)) {
        return false;
    }

    aStatusCodes.insert((!answer.isEmpty() && answer[0])
                            ? BillAcceptorStatusCode::Normal::Enabled
                            : BillAcceptorStatusCode::Normal::Disabled);

    for (auto it = m_EscrowParTable.data().begin(); it != m_EscrowParTable.data().end(); ++it) {
        uchar coinPosition = uchar(it.key());

        if (processCommand(
                CNPSTalk::Command::GetAcceptedCoins, QByteArray(1, coinPosition), &answer) &&
            !answer.isEmpty()) {
            uchar coinAmountChange = uchar(answer[0]) - m_CoinsByChannel[coinPosition];
            m_CoinsByChannel[coinPosition] = uchar(answer[0]);

            for (uchar i = 0; i < coinAmountChange; ++i) {
                m_EscrowPars << it.value();
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::loadParTable() {
    QByteArray answer;

    if (!processCommand(CNPSTalk::Command::GetNominalChannels, answer) || answer.isEmpty()) {
        return false;
    }

    uchar channelCount = uchar(answer[0]);

    for (uchar i = 1; i <= channelCount; ++i) {
        QByteArray nominalData;

        if (processCommand(CNPSTalk::Command::GetNominals, QByteArray(1, i), &nominalData)) {
            // TODO: сейчас только вариант для России и НЕ планируется доделывать для другой валюты.
            // Зарефакторить если таковой вариант появится.
            QByteArray countryCode = nominalData.left(2);

            if (countryCode == "RU") {
                MutexLocker locker(&m_ResourceMutex);

                SPar par(nominalData.mid(2, 3).toInt(), Currency::RUB, ECashReceiver::CoinAcceptor);
                m_EscrowParTable.data().insert(i, par);
            } else {
                toLog(LogLevel::Error,
                      m_DeviceName + QString(": Unknown currency code %1)").arg(countryCode.data()));
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::setDefaultParameters() {
    QByteArray answer;

    if (!processCommand(CNPSTalk::Command::GetNominalChannels, &answer) || answer.isEmpty()) {
        return false;
    }

    uchar channelCount = uchar(answer[0]);

    for (uchar i = 1; i < channelCount; ++i) {
        if (!processCommand(CNPSTalk::Command::GetAcceptedCoins, QByteArray(1, i), &answer) ||
            answer.isEmpty()) {
            return false;
        }

        m_CoinsByChannel[i] = uchar(answer[0]);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool NPSTalkCoinAcceptor::isConnected() {
    if (!processCommand(CNPSTalk::Command::TestConnection)) {
        return false;
    }

    QByteArray data;

    if (processCommand(CNPSTalk::Command::GetModelVersion, &data)) {
        m_DeviceName = "Comestero RM5";
    }

    if (processCommand(CNPSTalk::Command::GetFirmwareVersion, &data)) {
        setDeviceParameter(CDeviceData::Firmware, data.simplified());
    }

    return true;
}

//--------------------------------------------------------------------------------
