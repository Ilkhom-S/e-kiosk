/* @file Лог-менеджер. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QMutex>

#include <Common/ILog.h>

#include <memory>

//---------------------------------------------------------------------------
class LogManager {
public:
    LogManager();
    virtual ~LogManager();

    /// Создать или получить экземпляр объекта логгирования
    virtual ILog *getLog(const QString &aName, LogType::Enum aType);

    /// Закрыть все журнальные файлы, например при переходе времени на следующие
    /// сутки
    virtual void logRotateAll();

    /// Установить уровень логирования для всех логов
    virtual void setGlobalLevel(LogLevel::Enum aMaxLogLevel);

protected:
    QMap<QString, std::shared_ptr<ILog>> m_Logs;
    QMutex m_Mutex;
    LogLevel::Enum m_MaxLogLevel;
};

//---------------------------------------------------------------------------
