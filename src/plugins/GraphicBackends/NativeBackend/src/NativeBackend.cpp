/* @file Реализация плагина. */

#include "NativeBackend.h"

#include <QtCore/QRegularExpression>

#include <SDK/GUI/IGraphicsEngine.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginFactory.h>
#include <SDK/Plugins/PluginInitializer.h>

namespace CNativeBackend {
/// Тип данного графического движка.
const char Type[] = "native";
const char PluginName[] = "Native";
const char Item[] = "item";
} // namespace CNativeBackend

//------------------------------------------------------------------------------
namespace {

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new NativeBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> Enum_Parameters() {
    return QVector<SDK::Plugin::SPluginParameter>(1) << SDK::Plugin::SPluginParameter(
               SDK::Plugin::Parameters::Debug,
               SDK::Plugin::SPluginParameter::Bool,
               false,
               QT_TRANSLATE_NOOP("NativeBackendParameters", "#debug_mode"),
               QT_TRANSLATE_NOOP("NativeBackendParameters", "#debug_mode_howto"),
               false);
}

} // namespace

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(makePath(SDK::PaymentProcessor::Application,
                                         SDK::PaymentProcessor::CComponents::GraphicsBackend,
                                         CNativeBackend::PluginName),
                                &CreatePlugin,
                                &Enum_Parameters,
                                NativeBackend);

//------------------------------------------------------------------------------
NativeBackend::NativeBackend(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_Factory(aFactory), m_InstancePath(aInstancePath), m_Engine(0), m_Core(0) {}

//------------------------------------------------------------------------------
NativeBackend::~NativeBackend() {}

//------------------------------------------------------------------------------
QString NativeBackend::getPluginName() const {
    return CNativeBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap NativeBackend::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void NativeBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//------------------------------------------------------------------------------
QString NativeBackend::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool NativeBackend::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool NativeBackend::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem>
NativeBackend::getItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    // TODO Context
    TGraphicItemsCache::iterator it = m_CachedItems.find(aInfo.name);

    if (it == m_CachedItems.end()) {
        auto plugin = m_Factory->getPluginLoader()->createPluginPtr(aInfo.directory + aInfo.name);

        if (!plugin.expired()) {
            m_LoadedPlugins.push_back(plugin);

            auto item = std::dynamic_pointer_cast<SDK::GUI::IGraphicsItem, SDK::Plugin::IPlugin>(
                plugin.lock());
            it = m_CachedItems.insert(aInfo.name, item);
        } else {
            m_Engine->getLog()->write(
                LogLevel::Error, QString("Failed to create '%1' graphics item.").arg(aInfo.name));
        }
    }

    return it == m_CachedItems.end() ? std::weak_ptr<SDK::GUI::IGraphicsItem>() : it.value();
}

//------------------------------------------------------------------------------
bool NativeBackend::removeItem(const SDK::GUI::GraphicsItem_Info &aInfo) {
    foreach (auto item, m_CachedItems.values(aInfo.name)) {
        if (item->getContext() == aInfo.context) {
            return m_CachedItems.remove(aInfo.name, item) != 0;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
QString NativeBackend::getType() const {
    return CNativeBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItem_Info> NativeBackend::getItem_List() {
    if (m_Item_List.isEmpty()) {
        QStringList items = m_Factory->getPluginLoader()->getPluginList(
            QRegularExpression("PaymentProcessor\\.GraphicsItem\\..*"));

        foreach (const QString &item, items) {
            SDK::GUI::GraphicsItem_Info item_Info;
            item_Info.name = item.split(".").last(); // Последняя секция - имя айтема
            item_Info.type = CNativeBackend::Type;
            item_Info.directory = "PaymentProcessor.GraphicsItem.";

            m_Item_List[item_Info.name] = item_Info;
        }
    }

    return m_Item_List.values();
}

//------------------------------------------------------------------------------
bool NativeBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    m_Engine = aEngine;
    m_Core = m_Engine->getGraphicsHost()->getInterface<SDK::PaymentProcessor::ICore>(
        SDK::PaymentProcessor::CInterfaces::ICore);

    return true;
}

//------------------------------------------------------------------------------
void NativeBackend::shutdown() {
    m_CachedItems.clear();

    while (!m_LoadedPlugins.empty()) {
        auto ptr = m_LoadedPlugins.front();

        m_Factory->getPluginLoader()->destroyPluginPtr(ptr);

        m_LoadedPlugins.pop_front();
    }
}

//------------------------------------------------------------------------------