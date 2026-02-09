/* @file Сторожевой таймер OSMP 2.5. */

#include "OSMP2.5.h"

#include "OSMP2.5Data.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
OSMP25::OSMP25() {
    // Данные порта.
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // Данные устройства.
    m_DeviceName = "OSMP 2.5";
    m_ioMessageLogging = ELoggingType::None;
    setConfigParameter(CHardware::Watchdog::CanRegisterKey, true);
    setConfigParameter(CHardware::Watchdog::CanWakeUpPC, true);
    m_PingTimer.setInterval(COSMP25::PingInterval);
}

//----------------------------------------------------------------------------
bool OSMP25::isConnected() {
    QByteArray answer;

    if (!processCommand(COSMP25::Commands::GetVersion, &answer)) {
        return false;
    }

    if (ProtocolUtils::clean(answer).isEmpty() && isAutoDetecting()) {
        toLog(LogLevel::Error, m_DeviceName + ": Unknown device trying to impersonate this device");
        return false;
    }

    m_Verified = answer.contains(COSMP25::ModelTag);
    setDeviceParameter(CDeviceData::Version, answer);

    return true;
}

//--------------------------------------------------------------------------------
bool OSMP25::updateParameters() {
    QByteArray answer;

    if (processCommand(COSMP25::Commands::SerialNumber, &answer)) {
        QByteArray typeBuffer = answer.left(2);
        uint type = 0;

        for (int i = 0; i < typeBuffer.size(); ++i) {
            type += uint(uchar(typeBuffer[i])) << ((typeBuffer.size() - i - 1) * 8);
        }

        setDeviceParameter(CDeviceData::Type, type);
        setDeviceParameter(CDeviceData::SerialNumber, answer.mid(2).toHex());
    }

    for (int i = 0; i < COSMP25::MaxKeys; ++i) {
        if (processCommand(COSMP25::Commands::ReadKey, QByteArray(1, uchar(i)), &answer) &&
            (answer.size() > 1)) {
            QString key =
                QString("%1_%2").arg(CDeviceData::Watchdogs::Key).arg(i, 2, 10, QChar(ASCII::Zero));
            setDeviceParameter(key, answer.mid(1, 8).toHex());
            setDeviceParameter(CDeviceData::Type, int(uchar(answer[0])), key);
        }
    }

    return processCommand(COSMP25::Commands::SetModem_Pause,
                          QByteArray(1, COSMP25::Modem_ResettingPause)) &&
           processCommand(COSMP25::Commands::SetPCPause,
                          QByteArray(1, COSMP25::PCResettingPause)) &&
           processCommand(COSMP25::Commands::SetPingTimeout, QByteArray(1, COSMP25::PingTimeout));
}

//----------------------------------------------------------------------------
// TODO: сделать свич на линию питания.
bool OSMP25::reset(const QString &aLine) {
    if (!checkConnectionAbility()) {
        return false;
    }

    if (!m_StatusCollectionHistory.isEmpty() && (m_Initialized == ERequestStatus::Fail)) {
        toLog(LogLevel::Error, QString("%1: Cannot reset line %2").arg(m_DeviceName).arg(aLine));
        return false;
    }

    if (!isWorkingThread() || (m_Initialized == ERequestStatus::InProcess)) {
        QMetaObject::invokeMethod(
            this, "reset", Qt::BlockingQueuedConnection, Q_ARG(QString, aLine));
    } else if (aLine == SDK::Driver::LineTypes::Modem) {
        return processCommand(COSMP25::Commands::ResetModem);
    } else if (aLine == SDK::Driver::LineTypes::Terminal) {
        return processCommand(COSMP25::Commands::ResetPC);
    }

    return false;
}

//---------------------------------------------------------------------------
bool OSMP25::getStatus(TStatusCodes &aStatusCodes) {
    QTime PCWakingUpTime = getConfigParameter(CHardware::Watchdog::PCWakingUpTime).toTime();

    if (!PCWakingUpTime.isNull() && (PCWakingUpTime != m_PCWakingUpTime)) {
        int secsTo = PCWakingUpTime.secsTo(QTime::currentTime());

        if (secsTo < 0) {
            secsTo += 24 * 60 * 60;
        }

        int intervals = qRound(double(secsTo) / COSMP25::PCWakingUpInterval);

        if ((secsTo < COSMP25::PCWakingUpInterval) ||
            (std::abs(COSMP25::PCWakingUpInterval * intervals - secsTo) < COSMP25::PCWakingUpLag)) {
            QByteArray answer;
            bool resetPCWakingUpTimeResult = true;
            bool needResetPCWakeUpTime = !m_PCWakingUpTime.isNull();

            if (!needResetPCWakeUpTime) {
                if (!processCommand(COSMP25::Commands::PCWakeUpTime, &answer) &&
                    !answer.isEmpty()) {
                    resetPCWakingUpTimeResult = false;
                    toLog(LogLevel::Error, m_DeviceName + ": Cannot get wake up timeout");
                } else {
                    needResetPCWakeUpTime = answer[0];
                }
            }

            if (needResetPCWakeUpTime && resetPCWakingUpTimeResult &&
                !processCommand(COSMP25::Commands::ResetPCWakeUpTime)) {
                resetPCWakingUpTimeResult = false;
                toLog(LogLevel::Error, m_DeviceName + ": Cannot reset wake up timeout");
            }

            if (resetPCWakingUpTimeResult) {
                toLog(LogLevel::Normal,
                      QString("%1: Set wake up timeout to %2 hours -> %3")
                          .arg(m_DeviceName)
                          .arg(intervals / 2.0)
                          .arg(PCWakingUpTime.toString(COSMP25::TimeLogFormat)));

                if (processCommand(COSMP25::Commands::PCWakeUpTime,
                                   QByteArray(1, uchar(intervals)))) {
                    m_PCWakingUpTime = PCWakingUpTime;
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------
TResult OSMP25::execCommand(const QByteArray &aCommand,
                            const QByteArray &aCommandData,
                            QByteArray *aAnswer) {
    MutexLocker lock(&m_ExternalMutex);

    m_Protocol.setPort(m_IOPort);
    m_Protocol.setLog(m_Log);

    return m_Protocol.processCommand(aCommand + aCommandData, aAnswer);
}

//----------------------------------------------------------------------------
void OSMP25::setPingEnable(bool aEnabled) {
    WatchdogBase::setPingEnable(aEnabled);

    char command = aEnabled ? COSMP25::Commands::SetPingEnable : COSMP25::Commands::SetPingDisable;
    processCommand(command);
}

//-----------------------------------------------------------------------------
void OSMP25::onPing() {
    processCommand(COSMP25::Commands::Ping);
}

//--------------------------------------------------------------------------------
void OSMP25::registerKey() {
    START_IN_WORKING_THREAD(registerKey)

    QByteArray answer;
    bool result = processCommand(COSMP25::Commands::WriteKey, &answer) &&
                  (answer.isEmpty() || (answer[1] != COSMP25::KeyRegisteringExpired));

    emit keyRegistered(result);
}

//--------------------------------------------------------------------------------
