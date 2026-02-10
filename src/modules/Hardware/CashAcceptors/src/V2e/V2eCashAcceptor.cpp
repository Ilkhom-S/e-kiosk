/* @file Купюроприемник на протоколе V2e. */

#include "V2eCashAcceptor.h"

#include <QtCore/qmath.h>

#include "V2eCashAcceptorConstants.h"
#include "V2eModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
V2eCashAcceptor::V2eCashAcceptor() {
    // параметры порта
    this->m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    this->m_PortParameters[EParameters::Parity].append(EParity::Even);

    this->m_PortParameters[EParameters::DTR].clear();
    this->m_PortParameters[EParameters::DTR].append(EDTRControl::Enable);

    // данные устройства
    this->m_DeviceName = "V2e cash acceptor";
    this->m_EscrowPosition = 3;
    this->m_ParInStacked = true;
    this->m_MaxBadAnswers = 7;
    this->m_ResetOnIdentification = true;
    this->m_PollingIntervalEnabled = CV2e::PollingIntervals::Enabled;

    // параметры протокола
    this->m_DeviceCodeSpecification = PDeviceCodeSpecification(new CV2e::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList V2eCashAcceptor::getModelList() {
    QSet<QString> result;

    foreach (SBaseModelData aData, CV2e::ModelData().data().values()) {
        result << aData.name;
    }

    return {result.begin(), result.end()};
}

//---------------------------------------------------------------------------------
bool V2eCashAcceptor::checkStatus(QByteArray &aAnswer) {
    return processCommand(CV2e::Commands::Poll, &aAnswer);
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::processReset() {
    return processCommand(CV2e::Commands::Reset);
}

//---------------------------------------------------------------------------------
TResult V2eCashAcceptor::execCommand(const QByteArray &aCommand,
                                     const QByteArray &aCommandData,
                                     QByteArray *aAnswer) {
    MutexLocker locker(&m_ExternalMutex);

    this->m_Protocol.setPort(m_IOPort);
    this->m_Protocol.setLog(m_Log);

    QByteArray answer;

    int repeat = 0;
    bool correct = true;

    do {
        if (!correct) {
            toLog(LogLevel::Normal,
                  this->m_DeviceName +
                      QString(": process status due to IRQ in the answer, iteration #%1")
                          .arg(repeat + 1));
            TResult result = checkStatus(answer);

            if (!result) {
                return result;
            }
        }

        TResult result = this->m_Protocol.processCommand(aCommand + aCommandData, answer);

        if (!result) {
            return result;
        }

        correct = (answer.size() != 1) || (answer[0] != CV2e::IRQ);
    } while (!correct && (++repeat < CV2e::MaxRepeat) && (aCommand[0] != CV2e::Commands::Poll));

    if (!correct) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to handle IRQ in answer");
        return CommandResult::Transport;
    }

    if (aAnswer) {
        *aAnswer = answer.mid(1);
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::isConnected() {
    this->m_ResetOnIdentification = false;

    if (waitForBusy(true)) {
        waitForBusy(false);
    } else {
        this->m_ResetOnIdentification = true;

        if (!processCommand(CV2e::Commands::Reset)) {
            return false;
        }
    }

    QByteArray answer;

    if (!processCommand(CV2e::Commands::Identification, &answer)) {
        return false;
    }

    if (answer.isEmpty() && isAutoDetecting()) {
        toLog(LogLevel::Error,
              this->m_DeviceName + ": Unknown device trying to impersonate this device");
        return false;
    }

    QByteArray dbVersion = answer.mid(CV2e::FirmwareBytesAmount);
    QString hexDBVersion = "0x" + dbVersion.toHex().toUpper();
    QString logDBVersion = ProtocolUtils::clean(dbVersion);
    logDBVersion = logDBVersion.isEmpty() ? hexDBVersion
                                          : QString("%1 (%2)").arg(logDBVersion).arg(hexDBVersion);

    setDeviceParameter(CDeviceData::Firmware,
                       ProtocolUtils::clean(answer.left(CV2e::FirmwareBytesAmount)));
    setDeviceParameter(CDeviceData::CashAcceptors::Database, logDBVersion);

    SBaseModelData data = CV2e::ModelData().getData(answer.mid(4, 2));
    this->m_Verified = data.verified;
    this->m_DeviceName = data.name;

    return true;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::setDefaultParameters() {
    if (!waitForBusy(false) || (waitForBusy(true) && !waitForBusy(false))) {
        toLog(LogLevel::Error,
              this->m_DeviceName + ": Failed to wait not busy status from the cash acceptor.");
        return false;
    }

    // устанавливаем режим связи с устройством
    if (!processCommand(CV2e::Commands::SetComm_Mode, QByteArray(1, CV2e::CommunicationMode))) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to set communication mode");
        return false;
    }

    // разрешаем принимать все номиналы во всех направлениях
    if (!processCommand(CV2e::Commands::SetOrientation, QByteArray(1, CV2e::AllNoteDirections))) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to set nominals directions");
        return false;
    }

    // Inhibit mode нам не нужен
    if (!processCommand(CV2e::Commands::Uninhibited)) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to exit from Inhibit mode");
        return false;
    }

    // сохраняем настройки
    if (!processCommand(CV2e::Commands::ChangeDefault)) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to save settings");
        return false;
    }

    // включение высокого уровня контроля подлинности: 42h - 1 уровень, 4Bh - 2 уровень

    return true;
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::stack() {
    if (!checkConnectionAbility() || (m_Initialized != ERequestStatus::Success) ||
        this->m_CheckDisable) {
        return false;
    }

    return processCommand(CV2e::Commands::Stack);
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::reject() {
    if (!checkConnectionAbility() || (m_Initialized == ERequestStatus::Fail)) {
        return false;
    }

    return processCommand(CV2e::Commands::Return);
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::enableMoneyAcceptingMode(bool aEnabled) {
    CCashAcceptor::TStatuses lastStatuses = this->m_StatusHistory.lastValue().statuses;

    if (aEnabled && !lastStatuses.isEmpty(ECashAcceptorStatus::Inhibit) &&
        !processCommand(CV2e::Commands::Uninhibited)) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to exit from Inhibit mode");
        return false;
    }

    QByteArray commandData(8, ASCII::NUL);

    for (auto it = this->m_EscrowParTable.data().begin(); it != this->m_EscrowParTable.data().end();
         ++it) {
        if (aEnabled && it->enabled && !it->inhibit) {
            int id = it.key() - 1;
            int index = id / 8;
            commandData[index] = commandData[index] | (1 << id % 8);
        }
    }

    if (!processCommand(CV2e::Commands::SetBillEnables, commandData)) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to set nominals availability.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::loadParTable() {
    QByteArray answer;

    if (!processCommand(CV2e::Commands::GetParTable, QByteArray(1, CV2e::ProtocolID), &answer)) {
        toLog(LogLevel::Error, this->m_DeviceName + ": Failed to get par table");
        return false;
    }

    int nominalCount = answer.size() / CV2e::NominalSize;

    for (int i = 0; i < nominalCount; ++i) {
        QByteArray parData = answer.mid(1 + i * CV2e::NominalSize, CV2e::NominalSize);
        int nominal = uchar(parData[4]) * int(qPow(10, double(uchar(parData[5]))));
        QString currency = QString(parData.mid(1, 3));

        MutexLocker locker(&m_ResourceMutex);

        this->m_EscrowParTable.data().insert(uchar(parData[0]), SPar(nominal, currency));
    }

    return true;
}

//--------------------------------------------------------------------------------
void V2eCashAcceptor::cleanSpecificStatusCodes(TStatusCodes &aStatusCodes) {
    if (m_DeviceName == CV2e::Models::Aurora) {
        // при рефакторинге сделать как восстановимую ошибку для Авроры - неизвестную ошибку
        TStatusCodes beforeLastErrors = getStatusCodes(m_StatusCollection);
        bool lastStatusCodesOK =
            aStatusCodes.contains(DeviceStatusCode::Error::Unknown) &&
            !aStatusCodes.contains(BillAcceptorStatusCode::MechanicFailure::StackerOpen);
        bool beforeLastStatusCodesOK =
            !beforeLastErrors.contains(DeviceStatusCode::Error::Unknown) &&
            beforeLastErrors.contains(BillAcceptorStatusCode::MechanicFailure::StackerOpen);

        if (m_DeviceName.contains(CV2e::Models::Aurora) && lastStatusCodesOK &&
            beforeLastStatusCodesOK) {
            aStatusCodes.remove(DeviceStatusCode::Error::Unknown);

            // в статусе не будет инициализации
            SleepHelper::msleep(3000);
        }
    }
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::waitForBusy(bool aBusy) {
    CV2e::DeviceCodeSpecification *specification =
        this->m_DeviceCodeSpecification.dynamicCast<CV2e::DeviceCodeSpecification>().data();
    auto isBusy = [&]() -> bool {
        return !m_DeviceCodeBuffers.isEmpty() &&
               std::find_if(m_DeviceCodeBuffers.begin(),
                            this->m_DeviceCodeBuffers.end(),
                            [&](const QByteArray &aBuffer) -> bool {
                                return specification->isBusy(aBuffer) == aBusy;
                            }) != this->m_DeviceCodeBuffers.end();
    };

    auto poll = [&]() -> bool {
        TStatusCodes statusCodes;
        return getStatus(std::ref(statusCodes));
    };
    int timeout = aBusy ? CV2e::Timeouts::ReliableNonBusy : CV2e::Timeouts::Busy;

    return PollingExpector().wait<bool>(poll, isBusy, this->m_PollingIntervalEnabled, timeout);
}

//--------------------------------------------------------------------------------
