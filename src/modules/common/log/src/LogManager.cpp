/* @file Лог-менеджер. */

#include "LogManager.h"

#include "SimpleLog.h"

LogManager gLogManager;

//---------------------------------------------------------------------------
LogManager::LogManager() : m_MaxLogLevel(LogLevel::Normal) {
#if defined(_DEBUG) || defined(DEBUG_INFO)
    m_MaxLogLevel = LogLevel::Debug;
#endif // _DEBUG || DEBUG_INFO
}

//---------------------------------------------------------------------------
LogManager::~LogManager() {
    QMutexLocker lock(&m_Mutex);

    while (!m_Logs.isEmpty()) {
        auto log = m_Logs.take(m_Logs.keys().first());

        log.reset();
    }
}

//---------------------------------------------------------------------------
ILog *LogManager::getLog(const QString &aName, LogType::Enum aType) {
    QString name = QString("%1%2").arg(aName).arg(aType);

    QMutexLocker lock(&m_Mutex);

    if (m_Logs.contains(name)) {
        return m_Logs.value(name).get();
    }

    std::shared_ptr<ILog> newlog(new SimpleLog(aName, aType, m_MaxLogLevel));
    m_Logs.insert(name, newlog);

    return newlog.get();
}

//---------------------------------------------------------------------------
void LogManager::logRotateAll() {
    QMutexLocker lock(&m_Mutex);

    foreach (auto log, m_Logs.values()) {
        log->logRotate();
    }
}

//---------------------------------------------------------------------------
void LogManager::setGlobalLevel(LogLevel::Enum aMaxLogLevel) {
    m_MaxLogLevel = aMaxLogLevel;
}

//---------------------------------------------------------------------------
