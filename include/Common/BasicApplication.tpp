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

    QFileInfo fileInfo(mQtApplication.applicationFilePath());

    // Load translations automatically
    QDir translationsDir(getWorkingDirectory());
    QStringList translationFilters;
    translationFilters << QString("%1_*.qm").arg(fileInfo.baseName());
    translationsDir.setNameFilters(translationFilters);

    QStringList translationFiles = translationsDir.entryList(QDir::Files, QDir::Name);
    if (!translationFiles.isEmpty())
    {
        QString translationFile = translationsDir.absoluteFilePath(translationFiles.first());
        mTranslator = QSharedPointer<QTranslator>(new QTranslator(&mQtApplication));

        if (mTranslator->load(translationFile))
        {
            mQtApplication.installTranslator(mTranslator.data());
            getLog()->write(LogLevel::Normal, QString("Translation %1 loaded.").arg(translationFile));
        }
        else
        {
            getLog()->write(LogLevel::Warning, QString("Failed to load translation %1.").arg(translationFile));
        }
    }

    // Now that Qt application is created, write the log header
    writeLogHeader();
}
