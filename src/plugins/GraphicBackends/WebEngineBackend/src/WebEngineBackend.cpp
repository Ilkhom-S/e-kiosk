/**
 * @file Реализация плагина WebEngineBackend.
 */

#include "WebEngineBackend.h"

#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QtCore/QDir>
#include <QtCore/QMetaEnum>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslKey>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

namespace CWebEngineBackend {
/// Название плагина.
const char PluginName[] = "WebEngine";

/// Тип данного графического движка.
const char Type[] = "web";

/// Директория с ключами
const QString KeysDir = "keys";

/// Pem-файл
const QString PemFile = "local.pem";

/// Key-файл
const QString KeyFile = "local.key";
} // namespace CWebEngineBackend

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *createPlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new WebEngineBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> enumParameters() {
    return QVector<SDK::Plugin::SPluginParameter>(1) << SDK::Plugin::SPluginParameter(
               SDK::Plugin::Parameters::Debug,
               SDK::Plugin::SPluginParameter::Bool,
               false,
               QT_TRANSLATE_NOOP("WebEngineBackendParameters", "#debug_mode"),
               QT_TRANSLATE_NOOP("WebEngineBackendParameters", "#debug_mode_howto"),
               false);
}

} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(makePath(SDK::PaymentProcessor::Application,
                                         SDK::PaymentProcessor::CComponents::GraphicsBackend,
                                         CWebEngineBackend::PluginName),
                                &createPlugin,
                                &enumParameters,
                                WebEngineBackend);

//------------------------------------------------------------------------------
WebEngineBackend::WebEngineBackend(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath)
    : m_Factory(aFactory), m_InstancePath(aInstancePath), m_Engine(nullptr), m_CoreProxy(nullptr) {}

//------------------------------------------------------------------------------
WebEngineBackend::~WebEngineBackend() {
    m_Items.clear();
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getPluginName() const {
    return CWebEngineBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap WebEngineBackend::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void WebEngineBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool WebEngineBackend::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool WebEngineBackend::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem>
WebEngineBackend::getItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    QMultiMap<QString, std::shared_ptr<WebGraphicsItem>>::iterator it = m_Items.find(aInfo.name);

    if (it != m_Items.end() && it.value()->getContext() == aInfo.context) {
        return it.value();
    }

    std::shared_ptr<WebGraphicsItem> item(
        new WebGraphicsItem(aInfo, m_CoreProxy, m_Engine->getLog()),
        SDK::GUI::GraphicsItem_Deleter());

    if (item->isValid()) {
        m_Items.insert(aInfo.name, item);
    } else {
        m_Engine->getLog()->write(LogLevel::Error, item->getError());
    }

    return item;
}

//------------------------------------------------------------------------------
bool WebEngineBackend::removeItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    foreach (auto item, m_Items.values(aInfo.name)) {
        if (item->getContext() == aInfo.context) {
            return m_Items.remove(aInfo.name, item) != 0;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getType() const {
    return CWebEngineBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItem_Info> WebEngineBackend::getItem_List() {
    return QList<SDK::GUI::GraphicsItem_Info>();
}

//------------------------------------------------------------------------------
bool WebEngineBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    m_Engine = aEngine;
    m_CoreProxy = static_cast<SDK::PaymentProcessor::Scripting::Core *>(
        m_Engine->getGraphicsHost()->getInterface<QObject>(
            SDK::PaymentProcessor::Scripting::CProxyNames::Core));

    // Импорт ssl сертификата
    QFile pem(m_Factory->getKernelDataDirectory() + QDir::separator() + CWebEngineBackend::KeysDir +
              QDir::separator() + CWebEngineBackend::PemFile);
    if (pem.open(QIODevice::ReadOnly)) {
        QSslConfiguration conf = QSslConfiguration::defaultConfiguration();

        QSslCertificate cert(pem.readAll());
        conf.setLocalCertificate(cert);
        m_Engine->getLog()->write(LogLevel::Normal, "WebEngineBackend: Pem certifiacate added.");

        QFile key(m_Factory->getKernelDataDirectory() + QDir::separator() +
                  CWebEngineBackend::KeysDir + QDir::separator() + CWebEngineBackend::KeyFile);
        if (key.open(QIODevice::ReadOnly)) {
            QSslKey k(key.readAll(), QSsl::Rsa);
            conf.setPrivateKey(k);
            m_Engine->getLog()->write(LogLevel::Normal,
                                      "WebEngineBackend: Key for certifiacate added.");
        } else {
            m_Engine->getLog()->write(LogLevel::Error, "WebEngineBackend: Can't open key file.");
        }

        QSslConfiguration::setDefaultConfiguration(conf);
    } else {
        m_Engine->getLog()->write(LogLevel::Error, "WebEngineBackend: Can't open pem file.");
    }

    return true;
}

//------------------------------------------------------------------------------
void WebEngineBackend::shutdown() {}

//------------------------------------------------------------------------------
