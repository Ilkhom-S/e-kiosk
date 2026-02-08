/* @file Функции для перевода строк в QML. */

#include "Translator.h"

#include <QtCore/QDir>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>

#include <memory>

#include "Log.h"

Translator::Translator(const QString &aInterfacePath) : m_InterfacePath(aInterfacePath) {
    // Загружаем настройки интерфейса.
    QSettings settings(m_InterfacePath + QDir::separator() + "interface.ini", QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("locale");

    foreach (QString key, settings.allKeys()) {
        if (key == "default") {
            m_DefaultLanguage = settings.value(key).toString();
        } else {
            m_Languages[key] = settings.value(key).toString();
        }
    }

    m_CurrentLanguage = (m_DefaultLanguage.isEmpty() && !m_Languages.empty())
                            ? m_Languages.begin().key()
                            : m_DefaultLanguage;
}

//------------------------------------------------------------------------------
QString Translator::tr(const QString &aString) {
    QString translation;
    QString moduleName = aString.section('#', 0, 0);

    if (!moduleName.isEmpty()) {
        QMap<QString, QTranslator *>::iterator translator = m_Translators.find(moduleName);

        if (translator == m_Translators.end()) {
            // Подгружаем транслятор.
            std::unique_ptr<QTranslator> tr(new QTranslator());
            if (tr->load(moduleName,
                         m_InterfacePath + QDir::separator() + "locale",
                         "",
                         QString("_%1.qm").arg(m_CurrentLanguage))) {
                translator = m_Translators.insert(moduleName, tr.release());
            } else {
                Log(Log::Error) << "Failed to load translation file " << moduleName << " in "
                                << m_InterfacePath;
                return aString;
            }
        }

        translation =
            (*translator)->translate(moduleName.toLatin1().data(), aString.toLatin1().data());
    }

    return translation.isEmpty() ? aString : translation;
}

//------------------------------------------------------------------------------
void Translator::setLanguage(const QString &aLanguage) {
    if (aLanguage != m_CurrentLanguage) {
        if (!m_Languages.contains(aLanguage)) {
            Log(Log::Error) << "Language " << aLanguage << " is not supported.";
            return;
        }

        // Удаляем все загруженные трансляторы.
        foreach (QTranslator *tr, m_Translators.values()) {
            delete tr;
        }

        m_Translators.clear();
        m_CurrentLanguage = aLanguage;
        emit languageChanged();
    }
}

//------------------------------------------------------------------------------
QString Translator::getLanguage() const {
    return m_CurrentLanguage;
}

//------------------------------------------------------------------------------
QString Translator::getDefaultLanguage() const {
    return m_DefaultLanguage;
}

//------------------------------------------------------------------------------
QStringList Translator::getLanguageList() const {
    QStringList result;

    foreach (QString key, m_Languages.keys()) {
        result << QString("%1.%2").arg(key).arg(m_Languages[key]);
    }

    return result;
}

//------------------------------------------------------------------------------
