#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

class LogClean : public QThread
{

  private:
    void removeTmpDir()
    {
        QDir _dir;
        QString parent_folder = _dir.absolutePath();

        QDirIterator directories(parent_folder, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while (directories.hasNext())
        {
            directories.next();
            if (directories.filePath().contains("tmp_"))
            {
                removeDir(directories.filePath());
            }
        }
    }

    bool removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName))
        {
            Q_FOREACH (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                                             QDir::AllDirs | QDir::Files,
                                                         QDir::DirsFirst))
            {
                if (info.isDir())
                {
                    result = removeDir(info.absoluteFilePath());
                }
                else
                {
                    result = QFile::remove(info.absoluteFilePath());
                }

                if (!result)
                {
                    return result;
                }
            }
            result = dir.rmdir(dirName);
        }

        return result;
    }

    void removeOldLogs()
    {
        QDir _dir("log");
        QString dirPath = _dir.absolutePath();

        // Локальный лог
        removeFile(dirPath);
    }

    bool removeFile(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName))
        {
            Q_FOREACH (QFileInfo info, dir.entryInfoList(QDir::Files))
            {
                QDate _logDate = QDate::fromString(info.fileName().left(10), "dd.MM.yyyy");
                if (_logDate < QDate::currentDate().addDays(-61))
                {
                    result = QFile::remove(info.absoluteFilePath());

                    if (!result)
                    {
                        return result;
                    }
                }
            }
        }

        return result;
    }

    void removeOldValidatorLogs()
    {
        QDir _dir("ValidatorLog");
        QString dirPath = _dir.absolutePath();

        QDir dir(dirPath);

        if (dir.exists(dirPath))
        {
            Q_FOREACH (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                                             QDir::AllDirs | QDir::Files,
                                                         QDir::DirsFirst))
            {
                QDate _logDate = QDate::fromString(info.fileName().left(10), "yyyy-MM-dd");

                if (_logDate < QDate::currentDate().addDays(-61))
                {
                    if (info.isDir())
                    {
                        removeDir(info.absoluteFilePath());
                    }
                    else
                    {
                        QFile::remove(info.absoluteFilePath());
                    }
                }
            }
        }
    }

  protected:
    void run()
    {
        // Удалим папки tmp updater-a
        removeTmpDir();
        msleep(1000);

        removeOldLogs();
        msleep(1000);

        removeOldValidatorLogs();
        msleep(1000);
    }
};
