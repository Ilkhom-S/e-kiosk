/* @file Реализация плагина. */

#include "QMLBackend.h"

#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickItem>

#ifndef Q_OS_MACOS
#include <QtWebEngine/QtWebEngine>
#endif

#include <Common/ILog.h>

#include <SDK/GUI/MessageBoxParams.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

namespace CQMLBackend {
/// Тип данного графического движка.
const char Type[] = "qml";
const char PluginName[] = "QML";
const char TypesExportNamespace[] = "Types";
} // namespace CQMLBackend

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new QMLBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> Enum_Parameters() {
    return QVector<SDK::Plugin::SPluginParameter>(1) << SDK::Plugin::SPluginParameter(
               SDK::Plugin::Parameters::Debug,
               SDK::Plugin::SPluginParameter::Bool,
               false,
               QT_TRANSLATE_NOOP("QMLBackendParameters", "#debug_mode"),
               QT_TRANSLATE_NOOP("QMLBackendParameters", "#debug_mode_howto"),
               false);
}

} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(makePath(SDK::PaymentProcessor::Application,
                                         SDK::PaymentProcessor::CComponents::GraphicsBackend,
                                         CQMLBackend::PluginName),
                                &CreatePlugin,
                                &Enum_Parameters,
                                QMLBackend);

//------------------------------------------------------------------------------
QMLBackend::QMLBackend(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath) : m_Factory(aFactory), m_Engine(0), m_InstancePath(aInstancePath) {
    
    
    

#ifndef Q_OS_MACOS
    QtWebEngine::initialize();
#endif

    // Регистрируем типы событий.
    qmlRegisterUncreatableMetaObject(
        SDK::PaymentProcessor::EEventType::staticMetaObject, // The meta-object from Q_NAMESPACE
        QString("%1.%2")
            .arg(SDK::PaymentProcessor::Scripting::CProxyNames::Core)
            .arg(CQMLBackend::TypesExportNamespace)
            .toLatin1(),
        1,
        0,
        "EventType",
        "EventType enum is readonly.");
    qmlRegisterUncreatableType<SDK::GUI::MessageBoxParams>(
        QString("%1.%2")
            .arg(SDK::PaymentProcessor::Scripting::CProxyNames::Core)
            .arg(CQMLBackend::TypesExportNamespace)
            .toLatin1(),
        1,
        0,
        "MessageBox",
        "MessageBoxParams enum is readonly.");

    connect(&m_QMLEngine,
            SIGNAL(warnings(const QList<QQmlError> &)),
            this,
            SLOT(onWarnings(const QList<QQmlError> &)));
}

//------------------------------------------------------------------------------
QString QMLBackend::getPluginName() const {
    return CQMLBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap QMLBackend::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void QMLBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString QMLBackend::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool QMLBackend::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool QMLBackend::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem>
QMLBackend::getItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    TGraphicItemsCache::iterator it = m_CachedItems.find(aInfo.name);

    if (it != m_CachedItems.end() && it.value()->getContext() == aInfo.context) {
        return it.value();
    }

    std::shared_ptr<QMLGraphicsItem> item(
        new QMLGraphicsItem(aInfo, &m_QMLEngine, m_Engine->getLog()),
        SDK::GUI::GraphicsItem_Deleter());

    if (item->isValid()) {
        m_CachedItems.insert(aInfo.name, item);
    } else {
        m_Engine->getLog()->write(LogLevel::Error, item->getError());
    }

    return item;
}

//------------------------------------------------------------------------------
bool QMLBackend::removeItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    foreach (auto item, m_CachedItems.values(aInfo.name)) {
        if (item->getContext() == aInfo.context) {
            return m_CachedItems.remove(aInfo.name, item) != 0;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString QMLBackend::getType() const {
    return CQMLBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItem_Info> QMLBackend::getItem_List() {
    return {};
}

//------------------------------------------------------------------------------
bool QMLBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    m_Engine = aEngine;
    m_Core = m_Engine->getGraphicsHost()->getInterface<SDK::PaymentProcessor::ICore>(
        SDK::PaymentProcessor::CInterfaces::ICore);

    foreach (auto objectName, m_Engine->getGraphicsHost()->getInterfacesName()) {
        if (SDK::PaymentProcessor::CInterfaces::ICore != objectName) {
            auto *object = m_Engine->getGraphicsHost()->getInterface<QObject>(objectName);
            if (object) {
                m_QMLEngine.rootContext()->setContextProperty(objectName, object);
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
void QMLBackend::shutdown() {}

//------------------------------------------------------------------------------
void QMLBackend::onWarnings(const QList<QQmlError> &aWarnings) {
    QString warnings;
    foreach (QQmlError e, aWarnings) {
        warnings += e.toString() + "\n";
    }

    m_Engine->getLog()->write(LogLevel::Warning, warnings);
}

//------------------------------------------------------------------------------
