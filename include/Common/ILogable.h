/* @file Интерфейс для логирования. */

#pragma once

//--------------------------------------------------------------------------------
#include "ILog.h"

class QString;

//--------------------------------------------------------------------------------
/// Класс, упрощающий вывод в лог.
class ILogable {
public:
    ILogable(ILog *aLog = nullptr) : m_Log(aLog) {}
    ILogable(const QString &aLogName) : m_Log(ILog::getInstance(aLogName)) {}

    virtual ~ILogable() {}

    inline void setLog(ILog *aLog) { m_Log = aLog; }

protected:
    inline void toLog(LogLevel::Enum aLevel, const QString &aMessage) const {
        if (m_Log) {
            m_Log->write(aLevel, aMessage);
        } else {
            qCritical("Log pointer is empty. Message:%s.", aMessage.toLocal8Bit().data());
        }
    }

    inline ILog *getLog() const { return m_Log; }

private:
    ILog *m_Log;
};

//---------------------------------------------------------------------------
