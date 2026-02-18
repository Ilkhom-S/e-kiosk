/* @file Реализация простого логгера в файл. */

#pragma once

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <Common/ILog.h>

#include "LogManager.h"

//---------------------------------------------------------------------------
class SimpleLog;

//---------------------------------------------------------------------------
class DestinationFile {
    QFile m_File;
    FILE *m_StdFile;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRecursiveMutex m_Mutex;
#else
    QMutex m_Mutex;
#endif
    QTextStream m_LogStream;
    QString m_FileName;

protected:
    DestinationFile()
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        : m_StdFile(nullptr), m_Mutex(QMutex::Recursive), m_LogStream(&m_File)
#else
        : m_StdFile(nullptr), m_LogStream(&m_File)
#endif
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_LogStream.setCodec("utf-8");
#endif
    }

    friend class SimpleLog;

public:
    bool open(const QString &aLogPath) {
        m_File.close();

        if (m_StdFile) {
            fflush(m_StdFile);
            fclose(m_StdFile);
        }

// Cross-platform file open
#ifdef Q_OS_WIN
        m_StdFile = _fsopen(aLogPath.toLocal8Bit().constData(), "ab+", _SH_DENYNO);
#else
        m_StdFile = fopen(aLogPath.toLocal8Bit().constData(), "ab+");
#endif

        bool isOK = m_StdFile && m_File.open(m_StdFile, QIODevice::Append | QIODevice::Text);

        m_FileName = isOK ? aLogPath : "";

        return isOK;
    }

    bool isOpen() const { return m_File.isOpen(); }

    QString fileName() const { return m_FileName; }

    void write(const QString &aMessage) {
        QMutexLocker locker(&m_Mutex);

        m_LogStream << aMessage;
        m_LogStream.flush();
    }
};

//---------------------------------------------------------------------------
typedef QSharedPointer<DestinationFile> DestinationFilePtr;

//---------------------------------------------------------------------------
class SimpleLog : public ILog {
public:
    explicit SimpleLog(const QString &aName = "Default",
                       LogType::Enum aType = LogType::File,
                       LogLevel::Enum aMaxLogLevel = LogLevel::Normal);
    virtual ~SimpleLog();

    // Методы интерфейса ILog
    /// Возвращает имя экземпляра лога.
    virtual const QString &getName() const;

    /// Возвращает тип вывода данного экземпляра лога.
    virtual LogType::Enum getType() const;

    /// Возвращает направление вывода.
    virtual const QString &getDestination() const;

    /// Устанавливает направление вывода.
    virtual void setDestination(const QString &aDestination);

    /// Устанавливает минимальный уровень, ниже которого логгирование
    /// игнорируется.
    virtual void setLevel(LogLevel::Enum aLevel);

    /// Устанавливает уровень отступа для древовидных логов.
    virtual void adjustPadding(int aStep);

    /// Производит запись в лог.
    virtual void write(LogLevel::Enum aLevel, const QString &aMessage);

    /// Производит запись в лог c форматированием данных.
    virtual void write(LogLevel::Enum aLevel, const QString &aMessage, const QByteArray &aData);

    /// Принудительно закрыть журнал. Функция write заново его откроет.
    virtual void logRotate();

protected:
    virtual bool init();
    virtual bool isInitiated();
    virtual void safeWrite(LogLevel::Enum aLevel, const QString &aMessage);

private:
    virtual void writeHeader();

private:
    bool m_InitOk;
    LogLevel::Enum m_MaxLogLevel;

    QString m_Name;
    QString m_Destination;
    LogType::Enum m_Type;
    int m_Padding;

    // Свертка повторяющихся сообщений.
    // При получении одинаковых сообщений подряд, выводим "last message repeated N times"
    int m_DuplicateCounter;
    QString m_LastMessage;
    LogLevel::Enum m_LastLevel;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRecursiveMutex m_WriteMutex;
#else
    QMutex m_WriteMutex;
#endif

    /// Сбросить счетчик повторов и записать сводку (если есть)
    void flushDuplicateCounter();

    DestinationFilePtr m_CurrentFile;
};

//---------------------------------------------------------------------------
