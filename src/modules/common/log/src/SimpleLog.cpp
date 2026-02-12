/* @file Реализация простого логгера в файл. */

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QThread>

#ifdef Q_OS_WIN
#include <Shlwapi.h>
#include <windows.h>

#pragma comment(lib, "Shlwapi.lib")
#endif

#ifdef Q_OS_MAC
#include <climits>
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#ifdef Q_OS_LINUX
#include <limits.h>
#include <unistd.h>
#endif

#include <Common/Version.h>

#include <SysUtils/ISysUtils.h>

#include "SimpleLog.h"

//---------------------------------------------------------------------------
SimpleLog::SimpleLog(const QString &aName, LogType::Enum aType, LogLevel::Enum aMaxLogLevel)
    : m_InitOk(false), m_MaxLogLevel(aMaxLogLevel), m_Name(aName), m_Destination(aName),
      m_Type(aType), m_Padding(0), m_DuplicateCounter(0) {}

//---------------------------------------------------------------------------
SimpleLog::~SimpleLog() {
    if (m_InitOk) {
        // Note: Do NOT call virtual methods during destruction.
        // adjustPadding(), safeWrite(), and getName() are virtual methods
        // and calling them in the destructor causes undefined behavior.
        // The derived class may have already been destroyed at this point.
        // Logging footer is skipped to maintain safe destruction semantics.
    }
}

//---------------------------------------------------------------------------
const QString &SimpleLog::getName() const {
    return m_Name;
}

//---------------------------------------------------------------------------
const QString &SimpleLog::getDestination() const {
    return m_Destination;
}

//---------------------------------------------------------------------------
void SimpleLog::setDestination(const QString &aDestination) {
    switch (m_Type) {
    case LogType::File: {
        if (m_Destination != aDestination) {
            m_Destination = aDestination;
            m_InitOk = false;
        }

        break;
    }

    case LogType::Console:
    case LogType::Debug:
        break;

    default:
        break;
    }
}

//---------------------------------------------------------------------------
LogType::Enum SimpleLog::getType() const {
    return m_Type;
}

//---------------------------------------------------------------------------
void SimpleLog::setLevel(LogLevel::Enum aLevel) {
    m_MaxLogLevel = aLevel;
}

//---------------------------------------------------------------------------
void SimpleLog::adjustPadding(int aStep) {
    m_Padding = qMax(0, m_Padding + aStep);
}

//---------------------------------------------------------------------------
void SimpleLog::logRotate() {
    if (!isInitiated()) {
        init();
    }
}

//---------------------------------------------------------------------------
void SimpleLog::write(LogLevel::Enum aLevel, const QString &aMessage) {
    if (!isInitiated() && !init()) {
        return;
    }

    if (aLevel > m_MaxLogLevel) {
        return;
    }

    safeWrite(aLevel, aMessage);
}

//---------------------------------------------------------------------------
void SimpleLog::write(LogLevel::Enum aLevel, const QString &aMessage, const QByteArray &aData) {
    write(aLevel, aMessage + aData.toHex().data());
}

//---------------------------------------------------------------------------
bool SimpleLog::init() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRecursiveMutex fileListMutex;
#else
    static QMutex fileListMutex(QMutex::Recursive);
#endif
    static QMap<QString, DestinationFilePtr> fileList;

    bool needWriteHeader = false;

    switch (m_Type) {
    case LogType::File: {
        QMutexLocker locker(&fileListMutex);

        if (fileList.contains(m_Destination)) {
            m_CurrentFile = fileList[m_Destination];

            if (!m_CurrentFile || !m_CurrentFile->isOpen()) {
                fileList.remove(m_Destination);

                return init();
            }

            // Тут происходит смена даты файла при смене суток.
            if (m_CurrentFile->fileName().contains(QDate::currentDate().toString("yyyy.MM.dd"))) {
                break;
            }
            fileList.remove(m_Destination);
            return init();
        }
        QString logPath;
        QString workingDirectory;

#ifdef Q_OS_WIN
        TCHAR szPath[MAX_PATH] = {0};

        if (GetModuleFileName(0, szPath, MAX_PATH)) {
            QFileInfo info(QDir::toNativeSeparators(
                QString::from_WCharArray(reinterpret_cast<const wchar_t *>(szPath))));
            QString settingsFilePath = QDir::toNativeSeparators(info.absolutePath() + "/" +
                                                                info.completeBaseName() + ".ini");
            QSettings m_Settings(ISysUtils::rm_BOM(settingsFilePath), QSettings::IniFormat);
            m_Settings.setIniCodec("UTF-8");

            if (m_Settings.contains("common/working_directory")) {
                QString directory = m_Settings.value("common/working_directory").toString();
                workingDirectory = QDir::toNativeSeparators(QDir::cleanPath(
                    (QDir::isAbsolutePath(directory) ? "" : (info.absolutePath() + "/")) +
                    directory));
            } else {
                workingDirectory = info.absolutePath();
            }
        }
#else
        // Unix implementation (Linux/macOS)
        {
            // Get the executable path using /proc/self/exe (Linux) or _NSGetExecutablePath
            // (macOS)
            QString exePath;
#ifdef __linux__
            char buf[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
            if (len != -1) {
                buf[len] = '\0';
                exePath = QString::fromLocal8Bit(buf);
            }
#elif defined(__APPLE__)
            char buf[PATH_MAX];
            uint32_t size = sizeof(buf);
            if (_NSGetExecutablePath(buf, &size) == 0) {
                char resolved[PATH_MAX];
                if (realpath(buf, resolved) != nullptr) {
                    exePath = QString::fromLocal8Bit(resolved);
                } else {
                    exePath = QString::fromLocal8Bit(buf);
                }
            }
#endif
            if (exePath.isEmpty()) {
                // Fallback: use current working directory + argv[0] from QCoreApplication
                // This might not work if QApplication isn't created yet
                exePath = QDir::currentPath() + "/tray"; // fallback
            }

            QFileInfo exeInfo(exePath);
            QString iniDirPath = exeInfo.absolutePath();

            // For macOS app bundles, the .ini file is in the directory containing the .app
            // bundle, not inside the bundle itself
#ifdef __APPLE__
            if (exePath.contains(".app/Contents/MacOS/")) {
                // Go up three levels: from Contents/MacOS/ to the directory containing the .app
                QDir bundleDir(exeInfo.absolutePath()); // Contents/MacOS
                bundleDir.cdUp();                       // Contents
                bundleDir.cdUp();                       // .app bundle
                bundleDir.cdUp();                       // directory containing .app
                iniDirPath = bundleDir.absolutePath();
            }
#endif

            QString settingsFilePath =
                QDir::toNativeSeparators(iniDirPath + "/" + exeInfo.completeBaseName() + ".ini");
            QSettings mSettings(ISysUtils::rm_BOM(settingsFilePath), QSettings::IniFormat);

            if (mSettings.contains("common/working_directory")) {
                QString directory = mSettings.value("common/working_directory").toString();
                if (QDir::isAbsolutePath(directory)) {
                    workingDirectory = QDir::toNativeSeparators(QDir::cleanPath(directory));
                } else {
                    workingDirectory =
                        QDir::toNativeSeparators(QDir::cleanPath(iniDirPath + "/" + directory));
                }
            } else {
                // For macOS app bundles, default to the directory containing the .app bundle
#ifdef __APPLE__
                if (exePath.contains(".app/Contents/MacOS/")) {
                    workingDirectory = iniDirPath; // This is already the bin directory
                } else {
                    workingDirectory = iniDirPath;
                }
#else
                workingDirectory = iniDirPath;
#endif
            }
        }
#endif
        logPath = workingDirectory + "/logs/" + QDate::currentDate().toString("yyyy.MM.dd ") +
                  m_Destination + ".log";

        QFileInfo logPathInfo(logPath);

        if (!logPathInfo.exists()) {
            QDir logDir(logPathInfo.absolutePath());

            if (!logDir.mkpath(logPathInfo.absolutePath())) {
                return false;
            }
        }

        m_CurrentFile = DestinationFilePtr(new DestinationFile());

        if (!m_CurrentFile->open(logPath)) {
            return false;
        }

        needWriteHeader = true;

        fileList[m_Destination] = m_CurrentFile;
        break;
    }

    case LogType::Console:
    case LogType::Debug:
        break;

    default:
        return false;
    }

    m_InitOk = true;

    if (needWriteHeader) {
        writeHeader();
    }

    return true;
}

//---------------------------------------------------------------------------
bool SimpleLog::isInitiated() {
    // Тут проверяем сменился ли день
    if (m_Type == LogType::File && m_CurrentFile &&
        !m_CurrentFile->fileName().contains(QDate::currentDate().toString("yyyy.MM.dd"))) {
        return false;
    }

    return m_InitOk;
}

//---------------------------------------------------------------------------
void SimpleLog::writeHeader() {
    write(LogLevel::Normal,
          QString("%1 LOG [%2] STARTED. %3 %4.")
              .arg(QString("*").repeated(32))
              .arg(getName())
              .arg(Humo::Application)
              .arg(Humo::getVersion()));
}

//---------------------------------------------------------------------------
void SimpleLog::safeWrite(LogLevel::Enum aLevel, const QString &aMessage) {
    if (!m_InitOk) {
        return;
    }

    QString formattedMessage = QTime::currentTime().toString("hh:mm:ss.zzz ");

    auto threadId = reinterpret_cast<quint64>(QThread::currentThreadId());
    formattedMessage += QString("T:%1 ").arg(threadId, 8, 16, QLatin1Char('0'));

    switch (aLevel) {
    case LogLevel::Normal:
        formattedMessage += "[I] ";
        break;

    case LogLevel::Warning:
        formattedMessage += "[W] ";
        break;

    case LogLevel::Error:
        formattedMessage += "[E] ";
        break;

    case LogLevel::Fatal:
        formattedMessage += "C] "; // Critical instead of Fatal
        break;

    case LogLevel::Debug:
        formattedMessage += "[D] ";
        break;

    case LogLevel::Trace:
        formattedMessage += "[T] ";
        break;

    default:
        formattedMessage += "[U] ";
        break;
    }

    formattedMessage += QString(" %1%2\n").arg("", m_Padding * 3).arg(aMessage);

    switch (m_Type) {
    case LogType::File:
        m_CurrentFile->write(formattedMessage);
        break;

    case LogType::Debug:
        qDebug() << formattedMessage;
        break;

    case LogType::Console:
        printf("%s", formattedMessage.toLocal8Bit().data());
        break;

    default:
        break;
    }
}

//---------------------------------------------------------------------------
