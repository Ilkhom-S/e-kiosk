/* @file Различные средства отладки. */

#pragma once

#include <QtCore/QString>

#include <Common/ILog.h>

//---------------------------------------------------------------------------
class TraceLogger {
public:
    /// Конструктор. Логирует вход в функцию.
    TraceLogger(const char *aFileName, const char *aFuncName, int aLineNumber) {
        m_FileName = aFileName;
        m_FuncName = aFuncName;

        ILog::getInstance(m_LogName)->write(LogLevel::Normal,
                                            QString("%1Entering %2() (%3:%4)")
                                                .arg(QString(" ").repeated(m_Indent))
                                                .arg(m_FuncName)
                                                .arg(m_FileName)
                                                .arg(aLineNumber));

        m_Indent++;
    }

    /// Деструктор. Логирует выход из функции.
    ~TraceLogger() {
        m_Indent--;

        ILog::getInstance(m_LogName)->write(LogLevel::Normal,
                                            QString("%1Leaving  %2() (%3)")
                                                .arg(QString(" ").repeated(m_Indent))
                                                .arg(m_FuncName)
                                                .arg(m_FileName));
    }

private:
    const char *m_FileName;
    const char *m_FuncName;

private:
    static int m_Indent;
    static const char *m_LogName;
};

#define LOG_TRACE() TraceLogger traceLogger(__FILE__, __FUNCTION__, __LINE__)

#define ENABLE_TRACE_LOGGER(aLogName)                                                              \
    int TraceLogger::m_Indent = 0;                                                                 \
    const char *TraceLogger::m_LogName = aLogName;
