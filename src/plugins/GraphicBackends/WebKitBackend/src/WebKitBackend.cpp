/* @file Реализация плагина. */

#include "WebKitBackend.h"

#include <QtCore/QDir>
#include <QtCore/QMetaEnum>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslKey>
#include <QtWebKitWidgets/QGraphicsWebView>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebPage>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

namespace CWebKitBackend {
/// Название плагина.
const char PluginName[] = "Webkit";

/// Тип данного графического движка.
const char Type[] = "web";

/// Директория с ключами
const QString KeysDir = "keys";

/// Pem-файл
const QString Pem_File = "local.pem";

/// Key-файл
const QString KeyFile = "local.key";
} // namespace CWebKitBackend

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new WebKitBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> Enum_Parameters() {
    return QVector<SDK::Plugin::SPluginParameter>(1) << SDK::Plugin::SPluginParameter(
               SDK::Plugin::Parameters::Debug,
               SDK::Plugin::SPluginParameter::Bool,
               false,
               QT_TRANSLATE_NOOP("WebkitBackendParameters", "#debug_mode"),
               QT_TRANSLATE_NOOP("WebkitBackendParameters", "#debug_mode_howto"),
               false);
}

} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(
    SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                          SDK::PaymentProcessor::CComponents::GraphicsBackend,
                          CWebKitBackend::PluginName),
    &CreatePlugin,
    &Enum_Parameters);

//------------------------------------------------------------------------------
WebKitBackend::WebKitBackend(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_Factory(aFactory), m_InstancePath(aInstancePath), m_Engine(0), m_CoreProxy(0) {}

//------------------------------------------------------------------------------
WebKitBackend::~WebKitBackend() {
    m_Items.clear();
}

//------------------------------------------------------------------------------
QString WebKitBackend::getPluginName() const {
    return CWebKitBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap WebKitBackend::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void WebKitBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString WebKitBackend::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool WebKitBackend::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool WebKitBackend::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem>
WebKitBackend::getItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    QMap<QString, std::shared_ptr<WebGraphicsItem>>::iterator it = m_Items.find(aInfo.name);

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
bool WebKitBackend::removeItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    foreach (auto item, m_Items.values(aInfo.name)) {
        if (item->getContext() == aInfo.context) {
            return m_Items.remove(aInfo.name, item) != 0;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString WebKitBackend::getType() const {
    return CWebKitBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItem_Info> WebKitBackend::getItem_List() {
    return QList<SDK::GUI::GraphicsItem_Info>();
}

//------------------------------------------------------------------------------
bool WebKitBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    m_Engine = aEngine;
    m_CoreProxy = static_cast<SDK::PaymentProcessor::Scripting::Core *>(
        m_Engine->getGraphicsHost()->getInterface<QObject>(
            SDK::PaymentProcessor::Scripting::CProxyNames::Core));

    // Импорт ssl сертификата
    QFile pem(m_Factory->getKernelDataDirectory() + QDir::separator() + CWebKitBackend::KeysDir +
              QDir::separator() + CWebKitBackend::Pem_File);
    if (pem.open(QIODevice::ReadOnly)) {
        QSslConfiguration conf = QSslConfiguration::defaultConfiguration();

        QSslCertificate cert(pem.readAll());
        conf.setLocalCertificate(cert);
        m_Engine->getLog()->write(LogLevel::Normal, "WebKitBackend: Pem certifiacate added.");

        QFile key(m_Factory->getKernelDataDirectory() + QDir::separator() +
                  CWebKitBackend::KeysDir + QDir::separator() + CWebKitBackend::KeyFile);
        if (key.open(QIODevice::ReadOnly)) {
            QSslKey k(key.readAll(), QSsl::Rsa);
            conf.setPrivateKey(k);
            m_Engine->getLog()->write(LogLevel::Normal,
                                      "WebKitBackend: Key for certifiacate added.");
        } else {
            m_Engine->getLog()->write(LogLevel::Error, "WebKitBackend: Can't open key file.");
        }

        QSslConfiguration::setDefaultConfiguration(conf);
    } else {
        m_Engine->getLog()->write(LogLevel::Error, "WebKitBackend: Can't open pem file.");
    }

    return true;
}

//------------------------------------------------------------------------------
void WebKitBackend::shutdown() {}

//------------------------------------------------------------------------------
