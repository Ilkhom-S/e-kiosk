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
    this->mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    this->mPortParameters[EParameters::Parity].append(EParity::Even);

    this->mPortParameters[EParameters::DTR].clear();
    this->mPortParameters[EParameters::DTR].append(EDTRControl::Enable);

    // данные устройства
    this->mDeviceName = "V2e cash acceptor";
    this->mEscrowPosition = 3;
    this->mParInStacked = true;
    this->mMaxBadAnswers = 7;
    this->mResetOnIdentification = true;
    this->mPollingIntervalEnabled = CV2e::PollingIntervals::Enabled;

    // параметры протокола
    this->mDeviceCodeSpecification = PDeviceCodeSpecification(new CV2e::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList V2eCashAcceptor::getModelList() {
    QSet<QString> result;

    foreach (SBaseModelData aData, CV2e::ModelData().data().values()) {
        result << aData.name;
    }

    return QList<QString>(result.begin(), result.end());
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
    MutexLocker locker(&mExternalMutex);

    this->mProtocol.setPort(mIOPort);
    this->mProtocol.setLog(mLog);

    QByteArray answer;

    int repeat = 0;
    bool correct = true;

    do {
        if (!correct) {
            toLog(LogLevel::Normal,
                  this->mDeviceName +
                      QString(": process status due to IRQ in the answer, iteration #%1")
                          .arg(repeat + 1));
            TResult result = checkStatus(answer);

            if (!result) {
                return result;
            }
        }

        TResult result = this->mProtocol.processCommand(aCommand + aCommandData, answer);

        if (!result) {
            return result;
        }

        correct = (answer.size() != 1) || (answer[0] != CV2e::IRQ);
    } while (!correct && (++repeat < CV2e::MaxRepeat) && (aCommand[0] != CV2e::Commands::Poll));

    if (!correct) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to handle IRQ in answer");
        return CommandResult::Transport;
    }

    if (aAnswer) {
        *aAnswer = answer.mid(1);
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::isConnected() {
    this->mResetOnIdentification = false;

    if (waitForBusy(true)) {
        waitForBusy(false);
    } else {
        this->mResetOnIdentification = true;

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
              this->mDeviceName + ": Unknown device trying to impersonate this device");
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
    this->mVerified = data.verified;
    this->mDeviceName = data.name;

    return true;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::setDefaultParameters() {
    if (!waitForBusy(false) || (waitForBusy(true) && !waitForBusy(false))) {
        toLog(LogLevel::Error,
              this->mDeviceName + ": Failed to wait not busy status from the cash acceptor.");
        return false;
    }

    // устанавливаем режим связи с устройством
    if (!processCommand(CV2e::Commands::SetCommMode, QByteArray(1, CV2e::CommunicationMode))) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to set communication mode");
        return false;
    }

    // разрешаем принимать все номиналы во всех направлениях
    if (!processCommand(CV2e::Commands::SetOrientation, QByteArray(1, CV2e::AllNoteDirections))) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to set nominals directions");
        return false;
    }

    // Inhibit mode нам не нужен
    if (!processCommand(CV2e::Commands::Uninhibited)) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to exit from Inhibit mode");
        return false;
    }

    // сохраняем настройки
    if (!processCommand(CV2e::Commands::ChangeDefault)) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to save settings");
        return false;
    }

    // включение высокого уровня контроля подлинности: 42h - 1 уровень, 4Bh - 2 уровень

    return true;
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::stack() {
    if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) ||
        this->mCheckDisable) {
        return false;
    }

    return processCommand(CV2e::Commands::Stack);
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::reject() {
    if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail)) {
        return false;
    }

    return processCommand(CV2e::Commands::Return);
}

//---------------------------------------------------------------------------
bool V2eCashAcceptor::enableMoneyAcceptingMode(bool aEnabled) {
    CCashAcceptor::TStatuses lastStatuses = this->mStatusHistory.lastValue().statuses;

    if (aEnabled && !lastStatuses.isEmpty(ECashAcceptorStatus::Inhibit) &&
        !processCommand(CV2e::Commands::Uninhibited)) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to exit from Inhibit mode");
        return false;
    }

    QByteArray commandData(8, ASCII::NUL);

    for (auto it = this->mEscrowParTable.data().begin(); it != this->mEscrowParTable.data().end();
         ++it) {
        if (aEnabled && it->enabled && !it->inhibit) {
            int id = it.key() - 1;
            int index = id / 8;
            commandData[index] = commandData[index] | (1 << id % 8);
        }
    }

    if (!processCommand(CV2e::Commands::SetBillEnables, commandData)) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to set nominals availability.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool V2eCashAcceptor::loadParTable() {
    QByteArray answer;

    if (!processCommand(CV2e::Commands::GetParTable, QByteArray(1, CV2e::ProtocolID), &answer)) {
        toLog(LogLevel::Error, this->mDeviceName + ": Failed to get par table");
        return false;
    }

    int nominalCount = answer.size() / CV2e::NominalSize;

    for (int i = 0; i < nominalCount; ++i) {
        QByteArray parData = answer.mid(1 + i * CV2e::NominalSize, CV2e::NominalSize);
        int nominal = uchar(parData[4]) * int(qPow(10, double(uchar(parData[5]))));
        QString currency = QString(parData.mid(1, 3));

        MutexLocker locker(&mResourceMutex);

        this->mEscrowParTable.data().insert(uchar(parData[0]), SPar(nominal, currency));
    }

    return true;
}

//--------------------------------------------------------------------------------
void V2eCashAcceptor::cleanSpecificStatusCodes(TStatusCodes &aStatusCodes) {
    if (mDeviceName == CV2e::Models::Aurora) {
        // при рефакторинге сделать как восстановимую ошибку для Авроры - неизвестную ошибку
        TStatusCodes beforeLastErrors = getStatusCodes(mStatusCollection);
        bool lastStatusCodesOK =
            aStatusCodes.contains(DeviceStatusCode::Error::Unknown) &&
            !aStatusCodes.contains(BillAcceptorStatusCode::MechanicFailure::StackerOpen);
        bool beforeLastStatusCodesOK =
            !beforeLastErrors.contains(DeviceStatusCode::Error::Unknown) &&
            beforeLastErrors.contains(BillAcceptorStatusCode::MechanicFailure::StackerOpen);

        if (mDeviceName.contains(CV2e::Models::Aurora) && lastStatusCodesOK &&
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
        this->mDeviceCodeSpecification.dynamicCast<CV2e::DeviceCodeSpecification>().data();
    auto isBusy = [&]() -> bool {
        return !mDeviceCodeBuffers.isEmpty() &&
               std::find_if(mDeviceCodeBuffers.begin(),
                            this->mDeviceCodeBuffers.end(),
                            [&](const QByteArray &aBuffer) -> bool {
                                return specification->isBusy(aBuffer) == aBusy;
                            }) != this->mDeviceCodeBuffers.end();
    };

    auto poll = [&]() -> bool {
        TStatusCodes statusCodes;
        return getStatus(std::ref(statusCodes));
    };
    int timeout = aBusy ? CV2e::Timeouts::ReliableNonBusy : CV2e::Timeouts::Busy;

    return PollingExpector().wait<bool>(poll, isBusy, this->mPollingIntervalEnabled, timeout);
}

//--------------------------------------------------------------------------------
