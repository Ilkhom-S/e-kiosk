/* @file Класс для конвертирования json-описаний интерфейса пользователя в объекты QML */

#include "Skin.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QFontDatabase>

#include "Log.h"

namespace CSkin {
const char DefaultSkinName[] = "default";
const char Param_SkinName[] = "skin_name";
const char Param_ProviderId[] = "provider_id";
} // namespace CSkin

//------------------------------------------------------------------------------
Skin::Skin(const QObject *aApplication, const QString &aInterfacePath, const QString &aUserPath)
    : m_Name(CSkin::DefaultSkinName), m_InterfacePath(aInterfacePath),
      m_GuiService(aApplication->property("graphics").value<QObject *>()) {
    auto skinExist = [&](const QString &aName) -> bool {
        return QFile::exists(skinConfigFileName(aName));
    };

    auto getSkinName = [](const QString &aIniFile) -> QString {
        QSettings settings(aIniFile, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("ui");

        return settings.value("skin", "").toString();
    };

    // Имя скина из дистрибутива
    QString interfaceSkinName = getSkinName(m_InterfacePath + QDir::separator() + "interface.ini");

    // Имя скина, установленного пользователем
    QString userSkinName = getSkinName(aUserPath + QDir::separator() + "user.ini");

    // Приоритеты: пользовательский, дистрибутив, дефолтный
    m_Name = skinExist(userSkinName)
                ? userSkinName
                : (skinExist(interfaceSkinName) ? interfaceSkinName : CSkin::DefaultSkinName);
    m_PrevName = m_Name;

    loadSkinConfig();

    Log(Log::Normal) << QString("SET SKIN '%1'.").arg(m_Name);

    // Настройки брендирования
    auto getProviderSkinName = [](const QString &aIniFile) -> QVariantMap {
        QSettings settings(aIniFile, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("skin");

        // Читаем конфигурацию скинов для конкретных операторов(оператор=имя_скина)
        QVariantMap skins;
        foreach (auto const key, settings.allKeys()) {
            foreach (QString provider, settings.value(key).toStringList()) {
                skins.insert(provider, key);
            }
        }

        return skins;
    };

    m_ProviderSkinConfig = getProviderSkinName(m_InterfacePath + QDir::separator() + "interface.ini");
    QVariantMap userProviderSkinConfig =
        getProviderSkinName(aUserPath + QDir::separator() + "user.ini");
    m_ProviderSkinConfig =
        userProviderSkinConfig.isEmpty() ? m_ProviderSkinConfig : userProviderSkinConfig;
    m_ProviderSkinConfig.insert("-1", m_Name);

    // Зарегистрируем шрифты всех скинов
    QDirIterator dirEntry(QString("%1/skins").arg(m_InterfacePath),
                          QString("*.ttf*;*.otf").split(";"),
                          QDir::Files,
                          QDirIterator::Subdirectories);
    while (dirEntry.hasNext()) {
        dirEntry.next();

        if (QFontDatabase::addApplicationFont(dirEntry.filePath()) == -1) {
            Log(Log::Error) << QString("Failed to add font '%1'.").arg(dirEntry.fileName());
        }
    }
}

//------------------------------------------------------------------------------
bool Skin::loadSkinConfig() {
    // Загружаем настройки интерфейса.
    QFile json(skinConfigFileName(m_Name));

    if (json.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument result = QJsonDocument::from_Json(json.readAll(), &error);

        if (QJsonParseError::NoError == error.error) {
            QVariantMap config = result.object().toVariantMap();
            m_Config.clear();

            foreach (QString key, config.keys()) {
                if (!key.startsWith("color.") && !key.startsWith("font.")) {
                    QString path = m_InterfacePath + QDir::separator() + "skins" +
                                   QDir::separator() + m_Name + QDir::separator() +
                                   config.value(key).toString();

                    if (!QFile::exists(path)) {
                        Log(Log::Debug) << QString("SkinProvider: failed to load texture '%1' from "
                                                   "'%2'. Try loading "
                                                   "texture from 'default'.")
                                               .arg(key)
                                               .arg(path);
                        path = m_InterfacePath + QDir::separator() + "skins" + QDir::separator() +
                               "default" + QDir::separator() + config.value(key).toString();
                    }

                    QUrl url(path);
                    url.setScheme("file");
                    m_Config[key] = url.path();
                } else {
                    m_Config[key] = config.value(key);
                }
            }

            return true;
        } else {
            Log(Log::Error) << QString("Skin: failed to parse skin config '%1(%2)': %3.")
                                   .arg(skinConfigFileName(m_Name))
                                   .arg(error.offset)
                                   .arg(error.errorString());
        }
    } else {
        Log(Log::Error) << QString("Skin: failed to open skin config file '%1'.")
                               .arg(skinConfigFileName(m_Name));
    }

    return false;
}

//------------------------------------------------------------------------------
QFont Skin::font(const QString &aFontName) const {
    QVariantMap::const_iterator it = m_Config.find(aFontName);
    if (it != m_Config.end()) {
        QFont f;
        QVariantMap p = it->toMap();

        f.setBold(p["bold"].toBool());
        f.setPixelSize(p["pixelSize"].toInt());
        f.setFamily(p["family"].toString());
        f.setCapitalization(p["capitalization"] == "Font.AllUppercase" ? QFont::AllUppercase
                                                                       : QFont::MixedCase);

        return f;
    }

    Log(Log::Error) << QString("Failed to load font '%1'.").arg(aFontName);
    return QFont();
}

//------------------------------------------------------------------------------
QString Skin::color(const QString &aColorName) const {
    QVariantMap::const_iterator it = m_Config.find(aColorName);
    if (it != m_Config.end()) {
        return it->toString();
    } else {
        Log(Log::Error) << QString("Failed to load color '%1'.").arg(aColorName);
        return ("#FF00FF");
    }
}

//------------------------------------------------------------------------------
QString Skin::image(const QString &aImageId) const {
    QString scene = m_GuiService->property("topScene").value<QString>();
    QString pathWithScene = QString("%1/%2").arg(scene).arg(aImageId);

    // Сначала ищем путь в виде текущая_сцена/имя_ресурса
    QVariantMap::const_iterator it = m_Config.find(pathWithScene);

    if (it != m_Config.end()) {
        return it->toString();
    }

    it = m_Config.find(aImageId);

    return it != m_Config.end() ? it->toString() : "";
}

//------------------------------------------------------------------------------
QString Skin::getName() const {
    return m_Name;
}

//------------------------------------------------------------------------------
QVariantMap Skin::getConfiguration() const {
    return m_Config;
}

//------------------------------------------------------------------------------
void Skin::reload(const QVariantMap &aParams) {
    bool result = false;

    if (!aParams.isEmpty()) {
        m_PrevName = m_Name;
        QString providerId = aParams.value(CSkin::Param_ProviderId).toString();
        m_Name = providerId.isEmpty() ? m_Name : m_ProviderSkinConfig.value(providerId).toString();
        result = loadSkinConfig();
    }

    // Пустые параметры - вернуть предыдущий скин
    // Если не удалось - загружаем предыдущий скин
    if (aParams.isEmpty() || !result) {
        m_Name = m_PrevName;
        result = loadSkinConfig();
    }

    Log(Log::Normal) << QString("UPDATE SKIN '%1'. RESULT %2").arg(m_Name).arg(result);
}

//------------------------------------------------------------------------------
bool Skin::needReload(const QVariantMap &aParams) const {
    QString pid = aParams.value(CSkin::Param_ProviderId).toString();
    QString name = aParams.value(CSkin::Param_SkinName).toString();

    if (pid.toInt() == -1) {
        return false;
    }

    return (aParams.empty() && m_PrevName != m_Name) ||
           m_ProviderSkinConfig.keys().contains(pid) &&
               m_ProviderSkinConfig.value(name).toString() != m_Name;
}

//------------------------------------------------------------------------------
QString Skin::skinConfigFileName(const QString &aName) const {
    return m_InterfacePath + "/skins/" + aName + "/config.json";
}

//------------------------------------------------------------------------------
