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

//------------------------------------------------------------------------------
template <class T> class DevicePluginBase : public SDK::Plugin::IPlugin, public T {
public:
    DevicePluginBase(const QString &aPluginName,
                     SDK::Plugin::IEnvironment *aEnvironment,
                     const QString &aInstancePath)
        : m_InstanceName(aInstancePath), m_Environment(aEnvironment) {
        m_PluginName = aPluginName + " plugin";
        this->setLog(aEnvironment->getLog(""));

        if (m_Environment) {
            SDK::Plugin::IPluginLoader *pluginLoader = m_Environment->getPluginLoader();

            if (pluginLoader) {
                QString path =
                    m_InstanceName.section(SDK::Plugin::CPlugin::InstancePathSeparator, 0, 0);
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

    virtual ~DevicePluginBase() {}

    /// Возвращает название плагина.
    virtual QString getPluginName() const { return m_PluginName; }

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const { return T::getDeviceConfiguration(); }

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters) {
        T::setDeviceConfiguration(aParameters);
    }

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration() {
        return m_Environment->saveConfiguration(m_InstanceName, T::getDeviceConfiguration());
    }

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const { return m_InstanceName; }

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const { return true; }

private:
    QString m_PluginName;
    QString m_InstanceName;
    SDK::Plugin::IEnvironment *m_Environment;
};

//--------------------------------------------------------------------------------
