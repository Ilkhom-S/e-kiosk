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

    getLog()->write(LogLevel::Info, QString("Searching for translations for %1 in:").arg(appName));
    for (const QString &path : searchPaths) {
        getLog()->write(LogLevel::Info, QString("  - %1").arg(path));
    }

    bool translationLoaded = false;
    for (const QString &searchPath : searchPaths)
    {
        QDir translationsDir(searchPath);
        if (!translationsDir.exists()) {
            getLog()->write(LogLevel::Info, QString("Directory does not exist: %1").arg(searchPath));
            continue;
        }

        QStringList translationFilters;
        translationFilters << QString("%1_*.qm").arg(appName);
        translationsDir.setNameFilters(translationFilters);

        QStringList translationFiles = translationsDir.entryList(QDir::Files, QDir::Name);
        getLog()->write(LogLevel::Info, QString("Found %1 potential translation files in %2").arg(translationFiles.size()).arg(searchPath));
        
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
