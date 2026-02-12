/* @file Базовый класс AT-совместимого модема. */

#include "ATModemBase.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QRegularExpression>

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------------
ATModem_Base::ATModem_Base() : m_Modem_ConfigTimeout(CATGSMModem::Timeouts::Config) {
    // данные порта
    m_PortParameters[EParameters::BaudRate].append(
        EBaudRate::BR115200); // preferable for work, but not works in autobauding mode
    m_PortParameters[EParameters::BaudRate].append(
        EBaudRate::BR57600); // default after Mobile Equipment (ME) restart
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

    m_PortParameters[EParameters::Parity].append(EParity::No);

    // Отключено - записи в лог дублируются.
    // m_IOMessageLogging = true;
}

//--------------------------------------------------------------------------------
bool ATModem_Base::checkAT(int aTimeout) {
    m_IOPort->clear();

    int index = 0;
    bool result = false;

    do {
        result = processCommand(AT::Commands::AT, aTimeout);
    } while ((++index < 5) && !result);

    return result;
}

//--------------------------------------------------------------------------------
bool ATModem_Base::isConnected() {
    if (!checkAT(CATGSMModem::Timeouts::Default)) {
        m_IOPort->close();
        return false;
    }

    enableLocalEcho(false);

    QByteArray answer;

    if (!processCommand(AT::Commands::ATI, answer)) {
        m_IOPort->close();
        return false;
    }

    setDeviceName(answer);

    // Выводим конфигурацию модема в лог.
    if (processCommand(AT::Commands::ATandV, answer, m_Modem_ConfigTimeout)) {
        toLog(LogLevel::Normal,
              QString("Modem configuration: %1").arg(QString::fromLatin1(answer)));
    }

    QString modemInfo;
    getInfo(modemInfo);

    m_IOPort->close();

    return true;
}

//--------------------------------------------------------------------------------
void ATModem_Base::setDeviceName(const QByteArray &aFullName) {
    if (!isAutoDetecting()) {
        toLog(LogLevel::Normal, QString("Full modem info: %1").arg(QString::fromLatin1(aFullName)));
    }

    QString deviceName = aFullName.simplified();

    if (!deviceName.isEmpty()) {
        m_DeviceName = deviceName;
    }
}

//--------------------------------------------------------------------------------
bool ATModem_Base::reset() {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    // Сбрасываем модем.
    toLog(LogLevel::Normal, "Resetting modem to factory defaults...");
    bool result = processCommand(AT::Commands::ATandF0);

    onPoll();

    m_IOPort->close();

    return result;
}

//--------------------------------------------------------------------------------
bool ATModem_Base::setInitString(const QString &aInitString) {
    toLog(LogLevel::Normal, QString("Setting initialization string '%1'.").arg(aInitString));

    if (!checkConnectionAbility()) {
        return false;
    }

    QRegularExpression regExp("^AT");
    QString initString = QString(aInitString).remove(regExp);

    enableLocalEcho(false);

    bool result = processCommand(initString.toLatin1());

    m_IOPort->close();

    return result;
}

//--------------------------------------------------------------------------------
bool ATModem_Base::enableLocalEcho(bool aEnable) {
    QByteArray command = QByteArray(AT::Commands::ATE) + (aEnable ? "1" : "0");

    return processCommand(command);
}

//--------------------------------------------------------------------------------
QByteArray ATModem_Base::makeCommand(const QString &aCommand) {
    return QByteArray("AT").append(aCommand.toLatin1()).append(ASCII::CR);
}

//--------------------------------------------------------------------------------
bool ATModem_Base::processCommand(const QByteArray &aCommand, int aTimeout) {
    QByteArray answer;

    return processCommand(aCommand, answer, aTimeout);
}

//--------------------------------------------------------------------------------
bool ATModem_Base::processCommand(const QByteArray &aCommand, QByteArray &aAnswer, int aTimeout) {
    aAnswer.clear();

    if (!m_IOPort->write(makeCommand(aCommand))) {
        return false;
    }

    bool result = false;

    QElapsedTimer commandTimer;
    commandTimer.start();

    do {
        QByteArray data;

        if (m_IOPort->read(data)) {
            aAnswer.append(data);

            // Ищем сообщения о положительном результате.
            int pos = aAnswer.indexOf("OK");

            if (pos >= 0) {
                aAnswer.chop(aAnswer.size() - pos);
                result = true;

                break;
            }

            // Ищем сообщения о отрицательном результате.
            pos = aAnswer.indexOf("ERROR");

            if (pos >= 0) {
                aAnswer.chop(aAnswer.size() - pos);
                result = false;

                break;
            }
        }

        SleepHelper::msleep(CATGSMModem::Pauses::AnswerAttempt);
    } while (commandTimer.elapsed() < aTimeout);

    // Обязательный таймаут в 100 мс. после каждой операции.
    SleepHelper::msleep(CATGSMModem::Pauses::Answer);

    if (!result) {
        toLog(LogLevel::Error, QString("Bad answer : %1").arg(aAnswer.data()));
    }

    return result;
}

//--------------------------------------------------------------------------------
