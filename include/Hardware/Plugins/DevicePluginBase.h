/* @file Шаблонный плагин для драйверов. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/IPluginLoader.h>

// Hardware SDK
#include <SDK/Drivers/HardwareConstants.h>

#include <Hardware/Common/HardwareConstants.h>

// Forward declaration for makeDriverPath
template <class T> QString makeDriverPath();

#include <Hardware/Plugins/CommonParameters.h>
#include <utility>

//------------------------------------------------------------------------------
template <class T> class DevicePluginBase : public SDK::Plugin::IPlugin, public T {
public:
    DevicePluginBase(const QString &aPluginName,
                     SDK::Plugin::IEnvironment *aEnvironment,
                     QString aInstancePath)
        : m_instanceName(std::move(aInstancePath)), m_environment(aEnvironment) {
        m_pluginName = aPluginName + " plugin";
        this->setLog(aEnvironment->getLog(""));

        if (m_environment) {
            SDK::Plugin::IPluginLoader *pluginLoader = m_environment->getPluginLoader();

            if (pluginLoader) {
                QString path =
                    m_instanceName.section(SDK::Plugin::CPlugin::InstancePathSeparator, 0, 0);
                SDK::Plugin::TParameterList parameterList =
                    pluginLoader->getPluginParametersDescription(path);
                QStringList pluginParameterNames;
                QStringList requiredResourceNames;

                foreach (const SDK::Plugin::SPluginParameter &parameter, parameterList) {
                    if (parameter.name == CHardwareSDK::RequiredResource) {
                        path = parameter.defaultValue.toString().section(
                            SDK::Plugin::CPlugin::InstancePathSeparator, 0, 0);
                        SDK::Plugin::TParameterList rrParameterList =
                            pluginLoader->getPluginParametersDescription(path);

                        foreach (const SDK::Plugin::SPluginParameter &rrParameter,
                                 rrParameterList) {
                            if (!rrParameter.readOnly) {
                                requiredResourceNames << rrParameter.name;
                            }
                        }
                    } else {
                        pluginParameterNames << parameter.name;
                    }
                }

                QVariantMap configuration;
                configuration.insert(CHardware::PluginParameterNames, pluginParameterNames);
                configuration.insert(CHardware::RequiredResourceNames, requiredResourceNames);
                configuration.insert(CHardware::PluginPath, makeDriverPath<T>());
                T::setDeviceConfiguration(configuration);
            }
        }
    }

    ~DevicePluginBase() override = default;

    /// Возвращает название плагина.
    [[nodiscard]] QString getPluginName() const override { return m_pluginName; }

    /// Возвращает параметры плагина.
    [[nodiscard]] QVariantMap getConfiguration() const override {
        return T::getDeviceConfiguration();
    }

    /// Настраивает плагин.
    void setConfiguration(const QVariantMap &aParameters) override {
        T::setDeviceConfiguration(aParameters);
    }

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    bool saveConfiguration() override {
        return m_environment->saveConfiguration(m_instanceName, T::getDeviceConfiguration());
    }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    [[nodiscard]] QString getConfigurationName() const override { return m_instanceName; }

    /// Проверяет успешно ли инициализировался плагин при создании.
    [[nodiscard]] bool isReady() const override { return true; }

private:
    QString m_pluginName;
    QString m_instanceName;
    SDK::Plugin::IEnvironment *m_environment;
};

//--------------------------------------------------------------------------------
