/* @file Функции для перевода строк в QML. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Project
#include "Log.h"
#include "Translator.h"

Translator::Translator(const QString &aInterfacePath) : mInterfacePath(aInterfacePath)
{
    // Загружаем настройки интерфейса.
    QSettings settings(mInterfacePath + QDir::separator() + "interface.ini", QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("locale");

    foreach (QString key, settings.allKeys())
    {
        if (key == "default")
        {
            mDefaultLanguage = settings.value(key).toString();
        }
        else
        {
            mLanguages[key] = settings.value(key).toString();
        }
    }

    mCurrentLanguage =
        (mDefaultLanguage.isEmpty() && !mLanguages.empty()) ? mLanguages.begin().key() : mDefaultLanguage;
}

//------------------------------------------------------------------------------
QString Translator::tr(const QString &aString)
{
    QString translation;
    QString moduleName = aString.section('#', 0, 0);

    if (!moduleName.isEmpty())
    {
        QMap<QString, QTranslator *>::iterator translator = mTranslators.find(moduleName);

        if (translator == mTranslators.end())
        {
            // Подгружаем транслятор.
            QScopedPointer<QTranslator> tr(new QTranslator());
            if (tr->load(moduleName, mInterfacePath + QDir::separator() + "locale", "",
                         QString("_%1.qm").arg(mCurrentLanguage)))
            {
                translator = mTranslators.insert(moduleName, tr.take());
            }
            else
            {
                Log(Log::Error) << "Failed to load translation file " << moduleName << " in " << mInterfacePath;
                return aString;
            }
        }

        translation = (*translator)->translate(moduleName.toLatin1().data(), aString.toLatin1().data());
    }

    return translation.isEmpty() ? aString : translation;
}

//------------------------------------------------------------------------------
void Translator::setLanguage(const QString &aLanguage)
{
    if (aLanguage != mCurrentLanguage)
    {
        if (!mLanguages.contains(aLanguage))
        {
            Log(Log::Error) << "Language " << aLanguage << " is not supported.";
            return;
        }

        // Удаляем все загруженные трансляторы.
        foreach (QTranslator *tr, mTranslators.values())
        {
            delete tr;
        }

        mTranslators.clear();
        mCurrentLanguage = aLanguage;
        emit languageChanged();
    }
}

//------------------------------------------------------------------------------
QString Translator::getLanguage() const
{
    return mCurrentLanguage;
}

//------------------------------------------------------------------------------
QString Translator::getDefaultLanguage() const
{
    return mDefaultLanguage;
}

//------------------------------------------------------------------------------
QStringList Translator::getLanguageList() const
{
    QStringList result;

    foreach (QString key, mLanguages.keys())
    {
        result << QString("%1.%2").arg(key).arg(mLanguages[key]);
    }

    return result;
}

//------------------------------------------------------------------------------
