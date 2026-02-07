/* @file Интерфейс плагина. */

#pragma once

#include <QtCore/QString>
#include <QtCore/QVariantMap>

// Plugin SDK
#include <SDK/Plugins/IPluginEnvironment.h>

namespace SDK {
namespace Plugin {

namespace CPlugin {
/// Разделитель пути плагина и имени экземпляра.
extern const char InstancePathSeparator[];

/// Измененные имена параметров.
extern const char ModifiedKeys[];

/// Измененные значения параметров.
extern const char ModifiedValues[];
} // namespace CPlugin

//------------------------------------------------------------------------------
/// Интерфейс плагина.
class IPlugin {
    // Для доступа фабрики к деструктору
    friend class PluginFactory;

public:
    /// Возвращает название плагина.
    virtual QString getPluginName() const = 0;

    /// Возвращает имя файла конфигурации без расширения (ключ +
    /// идентификатор).
    virtual QString getConfigurationName() const = 0;

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const = 0;

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aConfiguration) = 0;

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл
    /// или хранилище прикладной программы).
    virtual bool saveConfiguration() = 0;

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const = 0;

protected:
    friend class PluginDeleter;
    virtual ~IPlugin() {}
};

//---------------------------------------------------------------------------
/// Удалятор для смарт-поинтера
class PluginDeleter {
public:
    void operator()(IPlugin *aPlugin) { delete aPlugin; }
};

//------------------------------------------------------------------------------
} // namespace Plugin
} // namespace SDK

// Объявление интерфейса, доступного из библиотеки.
// Q_DECLARE_INTERFACE(SDK::Plugin::IPlugin, "Humo.*.System.Plugin")

//------------------------------------------------------------------------------
