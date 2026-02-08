/* @file Сторожевой таймер OSMP. */

#include "OSMP.h"

#include <QtCore/QRegularExpression>

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
OSMP::OSMP() {
    // Данные порта.
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // Данные устройства.
    m_DeviceName = "OSMP Watchdog";

    // Реальное время таймаута = 25 минут, пинг раз в 7 минут.
    m_PingTimer.setInterval(7 * 60 * 1000);

    m_Data[EOSMPCommandId::IdentificationData] = "1.00";

    // Команды.
    m_Data[EOSMPCommandId::Identification] = "OSP\x01";
    m_Data[EOSMPCommandId::ResetModem] = "OSP\x02";
    m_Data[EOSMPCommandId::StartTimer] = "OSP\x03";
    m_Data[EOSMPCommandId::StopTimer] = "OSP\x04";
    m_Data[EOSMPCommandId::Ping] = "OSP\x05";
    m_Data[EOSMPCommandId::RebootPC] = "OSP\xAE";
}

//----------------------------------------------------------------------------
bool OSMP::isConnected() {
    QByteArray answer;

    if (!perform_Command(m_Data[EOSMPCommandId::Identification], &answer)) {
        return false;
    }

    answer = ProtocolUtils::clean(answer);

    // 1. Используем QRegularExpression.
    // QStringLiteral позволяет избежать лишних аллокаций памяти.
    QRegularExpression regExp(QStringLiteral("WDT.*v([0-9\\.]+)"));

    // 2. Выполняем сопоставление (match автоматически конвертирует QByteArray в QString)
    QRegularExpressionMatch match = regExp.match(QString::fromUtf8(answer));

    // 3. Проверяем наличие совпадения через hasMatch()
    // и сравниваем захваченную группу через captured(1)
    if (!match.hasMatch() || (match.captured(1) != m_Data[EOSMPCommandId::IdentificationData])) {
        return false;
    }

    if (!m_Connected) {
        if (perform_Command(COSMP::WrongDeviceCheck, &answer) && !answer.isEmpty()) {
            toLog(LogLevel::Error,
                  m_DeviceName + QStringLiteral(": Unknown device trying to impersonate the device "
                                                "based on OSMP protocol."));
            return false;
        }

        m_IOPort->close();
        m_IOPort->open();
        SleepHelper::msleep(COSMP::ReopenPortPause);
    }

    return true;
}

//----------------------------------------------------------------------------
// TODO: сделать свич на линию питания.
bool OSMP::reset(const QString &aLine) {
    if (!checkConnectionAbility()) {
        return false;
    }

    if (aLine == SDK::Driver::LineTypes::Modem) {
        return perform_Command(m_Data[EOSMPCommandId::ResetModem]);
    } else if (aLine == SDK::Driver::LineTypes::Terminal) {
        return perform_Command(m_Data[EOSMPCommandId::RebootPC]);
    }

    return false;
}

//----------------------------------------------------------------------------
bool OSMP::perform_Command(const QByteArray &aCommand, QByteArray *aAnswer) {
    MutexLocker lock(&m_ExternalMutex);

    QByteArray data;
    QByteArray &answer = aAnswer ? *aAnswer : data;
    answer.clear();

    if (!m_IOPort->write(aCommand)) {
        return false;
    }

    SleepHelper::msleep(100);

    if ((aCommand != m_Data[EOSMPCommandId::Identification]) &&
        (aCommand != m_Data[EOSMPCommandId::RebootPC]) &&
        (aCommand != m_Data[EOSMPCommandId::GetSensorStatus])) {
        return true;
    }

    return m_IOPort->read(answer) && !answer.isEmpty();
}

//----------------------------------------------------------------------------
void OSMP::setPingEnable(bool aEnabled) {
    WatchdogBase::setPingEnable(aEnabled);

    EOSMPCommandId::Enum commandId =
        aEnabled ? EOSMPCommandId::StartTimer : EOSMPCommandId::StopTimer;
    perform_Command(m_Data[commandId]);
}

//-----------------------------------------------------------------------------
void OSMP::onPing() {
    perform_Command(m_Data[EOSMPCommandId::Ping]);
}

//--------------------------------------------------------------------------------
