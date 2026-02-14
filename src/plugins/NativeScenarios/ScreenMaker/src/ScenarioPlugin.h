
#pragma once

#include <ScenarioEngine/Scenario.h>

// Plugin SDK
#include <SDK/Plugins/IFactory.h>

#include "MainScenario.h"

class IApplication;

//--------------------------------------------------------------------------
namespace SDK {
namespace PaymentProcessor {
class ICore;
} // namespace PaymentProcessor

namespace Plugin {
class IEnvironment;
} // namespace Plugin
} // namespace SDK

namespace PPSDK = SDK::PaymentProcessor;

namespace CScenarioPlugin {
const QString PluginName = "ScreenMaker";
} // namespace CScenarioPlugin

namespace ScreenMaker {

//--------------------------------------------------------------------------
class MainScenarioPlugin : public SDK::Plugin::IFactory<GUI::Scenario> {
public:
    MainScenarioPlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
        : m_Environment(aFactory), m_InstancePath(aInstancePath) {}

public:
    /// Возвращает название плагина.
    virtual QString getPluginName() const { return CScenarioPlugin::PluginName; }

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const {
        QVariantMap parameters;
        parameters["url"] = m_Url;
        return parameters;
    }

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters) {
        QString url = aParameters["url"].toString();
        m_Url = url.isEmpty() ? m_Url : url;
    }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const { return m_InstancePath; }

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration() {
        return m_Environment->saveConfiguration(CScenarioPlugin::PluginName, getConfiguration());
    }

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const { return true; }

    /// Возвращает список имен классов, которые создает фабрика.
    virtual QStringList getClassNames() const { return QStringList(CScenarioPlugin::PluginName); }

    /// Создает класс c заданным именем.
    virtual GUI::Scenario *create(const QString &aClassName) const {
        void *voidPtr = reinterpret_cast<void *>(
            m_Environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
        PPSDK::ICore *core = reinterpret_cast<PPSDK::ICore *>(voidPtr);
        return new MainScenario(core, m_Environment->getLog(aClassName));
    }

private:
    QString m_Url;
    QString m_InstancePath;
    SDK::Plugin::IEnvironment *m_Environment;
};

} // namespace ScreenMaker
