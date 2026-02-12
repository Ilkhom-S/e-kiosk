/* @file Сторожевой таймер Alarm. */

#include "Alarm.h"

#include <QtCore/QBuffer>
#include <QtCore/QElapsedTimer>

#include "AlarmData.h"

using namespace SDK::Driver::IOPort::COM;

//----------------------------------------------------------------------------
Alarm::Alarm() {
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    m_DeviceName = "Alarm";
    m_SensorDisabledValue = true;
}

//----------------------------------------------------------------------------
bool Alarm::isConnected() {
    CAlarm::CCommandIntervals commandIntervals;

    for (auto it = commandIntervals.data().begin(); it != commandIntervals.data().end(); ++it) {
        TAnswer answer;

        if (!m_IOPort->write(QByteArray(1, it.key())) || !getAnswer(answer) || answer.isEmpty()) {
            return false;
        }

        CAlarm::TInterval interval = it.value();

        if (std::find_if(answer.begin(), answer.end(), [&](char state) -> bool {
                return qBound(it.value().first, uchar(state), it.value().second) == uchar(state);
            }) == answer.end()) {
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
bool Alarm::reset(const QString & /*aLine*/) {
    if (!checkConnectionAbility()) {
        return false;
    }

    toLog(LogLevel::Normal, m_DeviceName + ": resetting modem");

    return m_IOPort->write(QByteArray(1, CAlarm::Commands::ResetModem));
}

//---------------------------------------------------------------------------
bool Alarm::getStatus(TStatusCodes &aStatusCodes) {
    TAnswer answer;

    if (!getAnswer(answer)) {
        return false;
    }

    if (answer.isEmpty()) {
        return isConnected();
    }

    foreach (char state, answer) {
        for (auto it = CAlarm::SensorCodeSpecification.data().begin();
             it != CAlarm::SensorCodeSpecification.data().end();
             ++it) {
            if (((state >> it.key()) & 1) != 0) {
                aStatusCodes << it.value();
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool Alarm::getAnswer(TAnswer &aAnswer) {
    MutexLocker locker(&m_ExternalMutex);

    aAnswer.clear();
    QByteArray answer;

    QElapsedTimer clockTimer;
    clockTimer.restart();

    do {
        if (!m_IOPort->read(answer, 100)) {
            return false;
        }
    } while ((clockTimer.elapsed() < CAlarm::DefaultTimeout) && answer.isEmpty());

    for (char i : answer) {
        aAnswer << i;
    }

    return true;
}

//--------------------------------------------------------------------------------------
