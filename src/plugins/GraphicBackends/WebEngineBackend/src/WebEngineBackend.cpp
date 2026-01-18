/* @file Реализация плагина WebEngineBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QMetaEnum>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslKey>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

// Project
#include "WebEngineBackend.h"

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
    SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath) {
        return new WebEngineBackend(aFactory, aInstancePath);
    }

    QVector<SDK::Plugin::SPluginParameter> EnumParameters() {
        return QVector<SDK::Plugin::SPluginParameter>(1) << SDK::Plugin::SPluginParameter(
                   SDK::Plugin::Parameters::Debug, SDK::Plugin::SPluginParameter::Bool, false,
                   QT_TRANSLATE_NOOP("WebEngineBackendParameters", "#debug_mode"),
                   QT_TRANSLATE_NOOP("WebEngineBackendParameters", "#debug_mode_howto"), false);
    }

} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                                                      SDK::PaymentProcessor::CComponents::GraphicsBackend,
                                                      CWebEngineBackend::PluginName),
                                &CreatePlugin, &EnumParameters);

//------------------------------------------------------------------------------
WebEngineBackend::WebEngineBackend(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : mFactory(aFactory), mInstancePath(aInstancePath), mEngine(0), mCoreProxy(0) {
}

//------------------------------------------------------------------------------
WebEngineBackend::~WebEngineBackend() {
    mItems.clear();
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getPluginName() const {
    return CWebEngineBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap WebEngineBackend::getConfiguration() const {
    return mParameters;
}

//------------------------------------------------------------------------------
void WebEngineBackend::setConfiguration(const QVariantMap &aParameters) {
    mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getConfigurationName() const {
    return mInstancePath;
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
std::weak_ptr<SDK::GUI::IGraphicsItem> WebEngineBackend::getItem(const SDK::GUI::GraphicsItemInfo &aInfo) {
    QMap<QString, std::shared_ptr<WebGraphicsItem>>::iterator it = mItems.find(aInfo.name);

    if (it != mItems.end() && it.value()->getContext() == aInfo.context) {
        return it.value();
    }

    std::shared_ptr<WebGraphicsItem> item(new WebGraphicsItem(aInfo, mCoreProxy, mEngine->getLog()),
                                          SDK::GUI::GraphicsItemDeleter());

    if (item->isValid()) {
        mItems.insert(aInfo.name, item);
    } else {
        mEngine->getLog()->write(LogLevel::Error, item->getError());
    }

    return item;
}

//------------------------------------------------------------------------------
bool WebEngineBackend::removeItem(const SDK::GUI::GraphicsItemInfo &aInfo) {
    foreach (auto item, mItems.values(aInfo.name)) {
        if (item->getContext() == aInfo.context) {
            return mItems.remove(aInfo.name, item) != 0;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString WebEngineBackend::getType() const {
    return CWebEngineBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItemInfo> WebEngineBackend::getItemList() {
    return QList<SDK::GUI::GraphicsItemInfo>();
}

//------------------------------------------------------------------------------
bool WebEngineBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    mEngine = aEngine;
    mCoreProxy = static_cast<SDK::PaymentProcessor::Scripting::Core *>(
        mEngine->getGraphicsHost()->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

    // Импорт ssl сертификата
    QFile pem(mFactory->getKernelDataDirectory() + QDir::separator() + CWebEngineBackend::KeysDir + QDir::separator() +
              CWebEngineBackend::PemFile);
    if (pem.open(QIODevice::ReadOnly)) {
        QSslConfiguration conf = QSslConfiguration::defaultConfiguration();

        QSslCertificate cert(pem.readAll());
        conf.setLocalCertificate(cert);
        mEngine->getLog()->write(LogLevel::Normal, "WebEngineBackend: Pem certifiacate added.");

        QFile key(mFactory->getKernelDataDirectory() + QDir::separator() + CWebEngineBackend::KeysDir +
                  QDir::separator() + CWebEngineBackend::KeyFile);
        if (key.open(QIODevice::ReadOnly)) {
            QSslKey k(key.readAll(), QSsl::Rsa);
            conf.setPrivateKey(k);
            mEngine->getLog()->write(LogLevel::Normal, "WebEngineBackend: Key for certifiacate added.");
        } else {
            mEngine->getLog()->write(LogLevel::Error, "WebEngineBackend: Can't open key file.");
        }

        QSslConfiguration::setDefaultConfiguration(conf);
    } else {
        mEngine->getLog()->write(LogLevel::Error, "WebEngineBackend: Can't open pem file.");
    }

    return true;
}

//------------------------------------------------------------------------------
void WebEngineBackend::shutdown() {
}

//------------------------------------------------------------------------------