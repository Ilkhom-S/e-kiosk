/* @file Реализация задачи архивации журнальных файлов. */

// Qt
#include <QtCore/QFileInfo>
#include <QtCore/QSet>

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

#include <algorithm>

// SDK
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Модули
#include <Common/BasicApplication.h>

#include <Packer/Packer.h>
#include <System/IApplication.h>

// Проект
#include "LogArchiver.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CLogArchiver {
const QString DateFormat = "yyyy.MM.dd"; // Формат даты для имён архивов
const int BytesInMB = 1048576;           // Количество байт в мегабайте (2^20)
} // namespace CLogArchiver

//---------------------------------------------------------------------------
/// Проверяет, является ли файл архивом по расширению
bool isArchive(const QFileInfo &aFileInfo) {
    return aFileInfo.suffix().toLower() == "zip" || aFileInfo.suffix().toLower() == "7z";
}

//---------------------------------------------------------------------------
LogArchiver::LogArchiver(const QString &aName, const QString &aLogName, const QString &aParams)
    : ITask(aName, aLogName, aParams), ILogable(aLogName), m_Canceled(false),
      m_Packer("", nullptr) {
    auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

    if (app) {
        PPSDK::ICore *core = app->getCore();
        PPSDK::TerminalSettings *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
            core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

        m_MaxSize = terminalSettings->getLogsMaxSize();
        m_LogDir = QDir(app->getWorkingDirectory() + "/logs");
        m_KernelPath = app->getWorkingDirectory();

        m_Packer.setToolPath(m_KernelPath);
    }

    m_Packer.setLog(getLog());
}

//---------------------------------------------------------------------------
LogArchiver::~LogArchiver() {}

//---------------------------------------------------------------------------
inline uint qHash(QDate key) {
    return key.toJulianDay();
}

//---------------------------------------------------------------------------
void LogArchiver::execute() {
    if (m_MaxSize < 1 || !m_LogDir.exists()) {
        toLog(LogLevel::Error, "Failed execute: (max_size < 1) OR (log_dir not exist)");

        emit finished(m_Name, false);
        return;
    }

    ILog::logRotateAll();

    foreach (auto date, getDatesForPack()) {
        if (!m_Canceled) {
            packLogs(date);
        }
    }

    if (!m_Canceled) {
        checkArchiveSize();
    }

    emit finished(m_Name, true);
}

//---------------------------------------------------------------------------
bool LogArchiver::cancel() {
    m_Canceled = true;
    m_Packer.terminate();

    return true;
}

//---------------------------------------------------------------------------
bool LogArchiver::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
}

//---------------------------------------------------------------------------
QString LogArchiver::logArchiveFileName(QDate aDate) {
    return m_LogDir.absoluteFilePath(
        QString("%1_logs.7z").arg(aDate.toString(CLogArchiver::DateFormat)));
}

//---------------------------------------------------------------------------
void LogArchiver::packLogs(QDate aDate) {
    toLog(LogLevel::Normal,
          QString("Packs logs '%1'").arg(aDate.toString(CLogArchiver::DateFormat)));

    bool updateArchive = QFile::exists(logArchiveFileName(aDate));

    // pack files to archive
    m_Packer.setUpdateMode(updateArchive);
    m_Packer.setFormat(Packer::SevenZip);
    m_Packer.setLevel(7);
    m_Packer.setTimeout(60 * 60 * 1000); // 1 час в миллисекундах
    m_Packer.setRecursive(true);

    QStringList toCompress;
    toCompress << QString("logs/%1*").arg(aDate.toString("yyyy.MM.dd"))
               << QString("receipts/%1*").arg(aDate.toString("yyyy.MM.dd"));

    QStringList archiveWildcards;
    archiveWildcards << "*.zip" << "*.7z";

    if (!m_Packer.pack(logArchiveFileName(aDate), m_KernelPath, toCompress, archiveWildcards)
             .isEmpty()) {
        if (m_Packer.exitCode() == 0) {
            toLog(LogLevel::Normal,
                  QString("Result code: %1; Output: %2")
                      .arg(m_Packer.exitCode())
                      .arg(m_Packer.messages()));
        }

        toLog(LogLevel::Normal, "Pack OK");

        if (!m_Canceled) {
            removeLogs(aDate);
        }
    } else {
        toLog(LogLevel::Error,
              QString("Pack failed: exitCode=%1 message='%2'")
                  .arg(m_Packer.exitCode())
                  .arg(m_Packer.messages()));

        // если мы не обновляем архив - удаляем неудачный архив
        if (!updateArchive) {
            removeFile(QFileInfo(logArchiveFileName(aDate)));
        }
    }
}

//---------------------------------------------------------------------------
QList<QDate> LogArchiver::getDatesForPack() const {
    QSet<QDate> result;

    foreach (auto file, m_LogDir.entryInfoList(QDir::Files | QDir::Dirs, QDir::Name)) {
        if (m_Canceled) {
            break;
        }

        QDate date = QDate::fromString(file.fileName().left(10), CLogArchiver::DateFormat);
        if (date.isValid() && date != QDate::currentDate()) {
            if ((file.isFile() && !isArchive(file)) || file.isDir()) {
                result << date;
            }
        }
    }

    foreach (auto dir, QDir(m_KernelPath + "/receipts").entryInfoList(QDir::Dirs, QDir::Name)) {
        if (m_Canceled) {
            break;
        }

        QDate date = QDate::fromString(dir.fileName().left(10), CLogArchiver::DateFormat);
        if (date.isValid() && date != QDate::currentDate()) {
            result << date;
        }
    }

    // Qt6: QSet no longer has toList(), use QList constructor with iterators
    QList<QDate> list(result.cbegin(), result.cend());
    std::sort(list.begin(), list.end());

    return list;
}

//---------------------------------------------------------------------------
void LogArchiver::removeLogs(QDate aDate) {
    auto clearLogDir = [this](const QDir &aDir, const QDate &aDate) {
        foreach (auto file,
                 aDir.entryInfoList(QStringList(aDate.toString("yyyy.MM.dd*")), QDir::Files)) {
            if (!isArchive(file)) {
                removeFile(file);
            }
        }
    };

    clearLogDir(m_LogDir, aDate);

    // чистим подпапки
    foreach (auto dir, m_LogDir.entryInfoList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot)) {
        clearLogDir(dir.filePath(), aDate);
    }

    auto clearSubdir = [&](QFileInfo &aFileInfo) {
        QDir dir(aFileInfo.absoluteFilePath());

        foreach (auto file, dir.entryInfoList(QDir::Files)) {
            removeFile(file);
        }
    };

    QList<QDir> dirsToRemove;
    dirsToRemove << m_LogDir << QDir(m_KernelPath + "/receipts");

    foreach (auto dir, dirsToRemove) {
        foreach (auto file,
                 dir.entryInfoList(QStringList(aDate.toString("yyyy.MM.dd*")), QDir::Dirs)) {
            clearSubdir(file);

            toLog(LogLevel::Debug, QString("Remove dir %1").arg(file.absoluteFilePath()));

            m_LogDir.rmdir(file.absoluteFilePath());
        }
    }
}

//---------------------------------------------------------------------------
void LogArchiver::checkArchiveSize() {
    toLog(LogLevel::Normal, QString("Check logs size limit. Max size is %1 Mb").arg(m_MaxSize));

    QStringList archiveWildcards;
    archiveWildcards << "*.zip" << "*.7z";

    QList<QFileInfo> files = m_LogDir.entryInfoList(archiveWildcards, QDir::Files, QDir::Name);

    auto fileSizeSumm = [](const QList<QFileInfo> &files) -> qint64 {
        qint64 summ = 0;
        foreach (auto file, files) {
            summ += file.size();
        }
        return summ;
    };

    while (!files.isEmpty() && fileSizeSumm(files) > qint64(m_MaxSize) * CLogArchiver::BytesInMB &&
           !m_Canceled) {
        removeFile(files.takeFirst());
    }

    toLog(LogLevel::Normal,
          QString("Logs archive size is %1 Mb")
              .arg(double(fileSizeSumm(files)) / CLogArchiver::BytesInMB, 0, 'f', 2));
}

//---------------------------------------------------------------------------
bool LogArchiver::removeFile(const QFileInfo &aFile) {
    bool result = QFile::remove(aFile.absoluteFilePath());

    toLog(result ? LogLevel::Normal : LogLevel::Error,
          QString("Remove [%1] %2").arg(aFile.absoluteFilePath()).arg(result ? "OK" : "Error"));

    return result;
}

//---------------------------------------------------------------------------
