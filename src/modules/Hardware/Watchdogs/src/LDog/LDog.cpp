/* @file Сторожевой таймер LDog. */

#include "LDog.h"

#include "LDogData.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
LDog::LDog() {
    // Данные порта.
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // Данные устройства.
    m_DeviceName = "LDog";
    m_PingTimer.setInterval(1000 * CLDog::Timeouts::PCReset / 2);
    m_NextRequestTime = QDateTime::currentDateTime();
    m_ioMessageLogging = ELoggingType::Write;
}

//----------------------------------------------------------------------------
bool LDog::isConnected() {
    return processCommand(CLDog::Commands::GetDeviceID) ||
           processCommand(CLDog::Commands::GetDeviceID);
}

//----------------------------------------------------------------------------
bool LDog::reset(const QString & /*aLine*/) {
    if (!checkConnectionAbility()) {
        return false;
    }

    return setTimeouts(true) && processCommand(CLDog::Commands::ResetModem);
}

//----------------------------------------------------------------------------
bool LDog::setTimeouts(bool aEnabled) {
    QByteArray commandData;
    auto appendTimeout = [&](ushort aTimeout) {
        commandData.append(reinterpret_cast<char *>(&aTimeout), sizeof(aTimeout));
    };

    appendTimeout(aEnabled ? CLDog::Timeouts::Start : 0);
    appendTimeout(aEnabled ? CLDog::Timeouts::PCReset : 0);
    appendTimeout(CLDog::Timeouts::PowerOff);
    appendTimeout(CLDog::Timeouts::PowerOff);

    return processCommand(CLDog::Commands::SetTimeouts, commandData);
}

//----------------------------------------------------------------------------
void LDog::setPingEnable(bool aEnabled) {
    WatchdogBase::setPingEnable(aEnabled);

    setTimeouts(aEnabled);
}

//--------------------------------------------------------------------------------
TResult LDog::processCommand(char aCommand, QByteArray *aAnswer) {
    QByteArray commandData;

    return processCommand(aCommand, commandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult LDog::processCommand(char aCommand, const QByteArray &aCommandData, QByteArray *aAnswer) {
    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    makePause(aCommand);

    QByteArray answerData;
    QByteArray &answer = aAnswer ? *aAnswer : answerData;
    TResult result = m_Protocol.processCommand(aCommand + aCommandData, answer);

    if (!result) {
        return result;
    }

    if (!CLDog::Commands::Data[aCommand] && (aAnswer != QByteArray(1, CLDog::ACK))) {
        if (aAnswer == QByteArray(1, CLDog::NAK)) {
            toLog(LogLevel::Error, m_DeviceName + ": Answer contains NAK");
        } else {
            toLog(LogLevel::Error, m_DeviceName + ": Wrong answer, need ACK");
        }

        return CommandResult::Transport;
    }

    return CommandResult::OK;
}

//---------------------------------------------------------------------------
void LDog::onPing() {
    processCommand(CLDog::Commands::PCEnable);
}

//--------------------------------------------------------------------------------
void LDog::makePause(char aCommand) {
    qint64 pause = QDateTime::currentDateTime().msecsTo(m_NextRequestTime);

    if (pause > 0) {
        SleepHelper::msleep(ulong(pause));
    }

    qlonglong interval = CLDog::Intervals::Write;

    if (CLDog::Commands::Data[aCommand]) {
        interval = CLDog::Intervals::Read;
    } else if ((aCommand == CLDog::Commands::ResetModem) ||
               (aCommand == CLDog::Commands::RebootPC)) {
        interval = CLDog::Intervals::PowerOff;
    }

    m_NextRequestTime = QDateTime::currentDateTime().addMSecs(interval);
}

//--------------------------------------------------------------------------------
