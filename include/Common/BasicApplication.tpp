/* @file Реализация шаблонного класса BasicQtApplication. */

#pragma once

// Qt includes needed for template implementation
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

template <typename T>
BasicQtApplication<T>::BasicQtApplication(const QString &aName, const QString &aVersion, int &aArgumentCount,
                                          char **aArguments)
    : BasicApplication(aName, aVersion, aArgumentCount, aArguments), mQtApplication(aArgumentCount, aArguments, true)
{
    // Set application name and version on the Qt application instance
    mQtApplication.setApplicationName(aName);
    mQtApplication.setApplicationVersion(aVersion);

    // Load translations automatically
    QFileInfo fileInfo(mQtApplication.applicationFilePath());
    QString appName = fileInfo.baseName();

    // Check multiple possible locations for translation files
    QStringList searchPaths;
    searchPaths << getWorkingDirectory();                                  // Working directory
    searchPaths << QDir(getWorkingDirectory()).absoluteFilePath("locale"); // locale subdirectory in working dir

    // On macOS, also check app bundle locations
#ifdef Q_OS_MACOS
    QString appDir = mQtApplication.applicationDirPath();
    searchPaths << QDir(appDir).absoluteFilePath("../Resources/locale"); // Contents/Resources/locale
    searchPaths << QDir(appDir).absoluteFilePath("locale");              // Contents/MacOS/locale (fallback)
#endif

    getLog()->write(LogLevel::Normal, QString("DEBUG: Searching for translations for %1").arg(appName));
    for (const QString &path : searchPaths)
    {
        getLog()->write(LogLevel::Normal, QString("DEBUG: Checking path: %1").arg(path));
        QDir testDir(path);
        if (testDir.exists())
        {
            QStringList files = testDir.entryList(QStringList() << "*.qm", QDir::Files);
            getLog()->write(LogLevel::Normal, QString("DEBUG: Directory exists, found %1 .qm files").arg(files.size()));
        }
        else
        {
            getLog()->write(LogLevel::Normal, QString("DEBUG: Directory does not exist"));
        }
    }

    bool translationLoaded = false;
    for (const QString &searchPath : searchPaths)
    {
        QDir translationsDir(searchPath);
        if (!translationsDir.exists())
        {
            continue;
        }

        // Debug: list all .qm files in directory first
        QStringList allQmFiles = translationsDir.entryList(QStringList() << "*.qm", QDir::Files);
        getLog()->write(LogLevel::Info,
                        QString("DEBUG: All .qm files in %1: %2").arg(searchPath).arg(allQmFiles.join(", ")));

        QStringList translationFilters;
        translationFilters << QString("%1_*.qm").arg(appName);
        translationsDir.setNameFilters(translationFilters);

        QStringList translationFiles = translationsDir.entryList(QDir::Files, QDir::Name);
        getLog()->write(LogLevel::Info,
                        QString("DEBUG: Looking for files matching: %1").arg(translationFilters.join(", ")));
        getLog()->write(LogLevel::Info,
                        QString("DEBUG: Found files with setNameFilters: %1").arg(translationFiles.join(", ")));

        if (!translationFiles.isEmpty())
        {
            QString translationFile = translationsDir.absoluteFilePath(translationFiles.first());
            getLog()->write(LogLevel::Info, QString("Trying to load: %1").arg(translationFile));
            mTranslator = QSharedPointer<QTranslator>(new QTranslator(&mQtApplication));

            if (mTranslator->load(translationFile))
            {
                mQtApplication.installTranslator(mTranslator.data());
                getLog()->write(LogLevel::Normal, QString("Translation %1 loaded.").arg(translationFile));
                translationLoaded = true;
                break; // Stop after loading the first translation file
            }
            else
            {
                getLog()->write(LogLevel::Warning, QString("Failed to load translation %1.").arg(translationFile));
            }
        }
    }
    if (!translationLoaded)
    {
        getLog()->write(LogLevel::Normal, QString("No translations found for %1 in searched locations.").arg(appName));
    }

    // Now that Qt application is created, write the log header
    writeLogHeader();
}