/* @file Фабрика плагинов TemplatePlugin. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

/// Фабрика для создания экземпляров TemplatePlugin.
/// Отвечает за жизненный цикл плагинов TemplatePlugin.
class TemplatePluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "template_plugin.json")

  public:
    /// Конструктор фабрики.
    TemplatePluginFactory();

    /// Деструктор фабрики.
    ~TemplatePluginFactory();

#pragma region Override PluginFactory methods

    /// Возвращает название плагина.
    /// @return QString с названием плагина
    virtual QString getName() const override;

    /// Возвращает описание плагина.
    /// @return QString с описанием плагина
    virtual QString getDescription() const override;

    /// Возвращает список доступных плагинов.
    /// @return QStringList с именами плагинов
    virtual QStringList getPluginList() const override;

    /// Создаёт экземпляр плагина.
    /// @param aInstancePath Путь к экземпляру плагина
    /// @param aConfigPath Путь к конфигурации
    /// @return указатель на созданный плагин или nullptr
    virtual SDK::Plugin::IPlugin *createPlugin(const QString &aInstancePath, const QString &aConfigPath) override;

    /// Уничтожает экземпляр плагина.
    /// @param aPlugin Указатель на плагин для уничтожения
    /// @return true если уничтожение успешно
    virtual bool destroyPlugin(SDK::Plugin::IPlugin *aPlugin) override;

#pragma endregion
};