/* @file Реализация шаблонного класса BasicQtApplication. */

#pragma once

// Qt includes needed for template implementation
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

template <typename T>
BasicQtApplication<T>::BasicQtApplication(const QString &aName,
                                          const QString &aVersion,
                                          int &aArgumentCount,
                                          char **aArguments)
    : BasicApplication(aName, aVersion, aArgumentCount, aArguments),
      m_QtApplication(aArgumentCount, aArguments, true) {
    // Set application name and version on the Qt application instance
    m_QtApplication.setApplicationName(aName);
    m_QtApplication.setApplicationVersion(aVersion);

    // Detect and load translations
    QFileInfo fileInfo(m_QtApplication.applicationFilePath());
    QString appName = fileInfo.baseName();

    // Determine locale: try settings -> system locale -> fallback to 'en'
    QString locale = m_Settings->value("common/locale", "").toString();

    if (locale.isEmpty()) {
        // Try system locale, extract language part (e.g., "ru_RU" -> "ru")
        QString systemLocale = QLocale::system().name();    // e.g., "ru_RU", "en_US"
        locale = systemLocale.split('_').first().toLower(); // Extract language code
    }

    // Fallback: check if we have translations for this locale
    // If not, fallback to 'en'
    QString candidateLocale = locale;

    // Check multiple possible locations for translation files
    QStringList searchPaths;
    searchPaths << getWorkingDirectory(); // Working directory
    searchPaths << QDir(getWorkingDirectory())
                       .absoluteFilePath("locale"); // locale subdirectory in working dir

    // Lambda to check if locale file exists
    auto localeFileExists = [&](const QString &localeCode) -> QString {
        for (const QString &searchPath : searchPaths) {
            QDir translationsDir(searchPath);
            if (!translationsDir.exists())
                continue;

            QString fileName = QString("%1_%2.qm").arg(appName, localeCode);
            QString filePath = translationsDir.absoluteFilePath(fileName);
            if (QFile::exists(filePath)) {
                return filePath;
            }
        }
        return QString();
    };

    // Try to find translation for requested locale
    QString translationFile = localeFileExists(candidateLocale);

    // If not found, fallback to English
    if (translationFile.isEmpty() && candidateLocale != "en") {
        getLog()->write(LogLevel::Warning,
                        QString("Translation for locale '%1' not found, falling back to 'en'.")
                            .arg(candidateLocale));
        translationFile = localeFileExists("en");
        candidateLocale = "en";
    }

    // Load the translation file
    bool translationLoaded = false;
    if (!translationFile.isEmpty()) {
        m_Translator = QSharedPointer<QTranslator>(new QTranslator(&m_QtApplication));
        if (m_Translator->load(translationFile)) {
            m_QtApplication.installTranslator(m_Translator.data());
            m_Language = candidateLocale;
            getLog()->write(LogLevel::Normal,
                            QString("Translation loaded: %1 (locale: %2)")
                                .arg(translationFile, candidateLocale));
            translationLoaded = true;
        } else {
            getLog()->write(LogLevel::Warning,
                            QString("Failed to load translation %1.").arg(translationFile));
        }
    }

    if (!translationLoaded) {
        m_Language = "en"; // Default fallback
        getLog()->write(LogLevel::Normal,
                        QString("No translations found for %1, using default.").arg(appName));
    }

    // Now that Qt application is created, write the log header
    writeLogHeader();
}
