/* @file Помощник по логу. */

#pragma once

#include <Common/ILog.h>

//--------------------------------------------------------------------------------
class DeviceLogManager {
public:
    DeviceLogManager() : m_Log(nullptr) {}
    DeviceLogManager(ILog *aLog) : m_Log(aLog) {}

    /// Логгировать.
    void toLog(LogLevel::Enum aLevel, const QString &aMessage) const {
        if (m_Log) {
            m_Log->write(aLevel, aMessage);
        } else {
            qCritical("Log pointer is empty. Message:%s.", aMessage.toLocal8Bit().data());
        }
    }

    /// Установить лог.
    void setLog(ILog *aLog) { m_Log = aLog; }

protected:
    /// Лог.
    ILog *m_Log;
};

//--------------------------------------------------------------------------------
