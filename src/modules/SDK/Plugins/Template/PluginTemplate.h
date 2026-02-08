/* @file Шаблон объявления плагина. */

#pragma once

#include <QtCore/QObject>

#include <Common/ILogable.h>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginFactory.h>

//------------------------------------------------------------------------------
/// Пример реализации плагина с функциональностью "Hello World".
class Plugin : public QObject, public SDK::Plugin::IPlugin, public ILogable {
    Q_OBJECT

public:
    Plugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath);
    ~Plugin();

public:
#pragma region SDK::Plugin::IPlugin interface

    /// IPlugin: Возвращает название плагина.
    virtual QString getPluginName() const override;

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const override;

    /// IPlugin: Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const override;

    /// IPlugin: Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aConfiguration) override;

    /// IPlugin: Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище
    /// прикладной программы).
    virtual bool saveConfiguration() override;

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const override;

#pragma endregion

    /// Пример функциональности плагина - возвращает приветственное сообщение.
    QString getHelloMessage() const;

private:
    SDK::Plugin::IEnvironment *m_Environment;
    QString m_InstancePath;
    QVariantMap m_Parameters;
    QString m_HelloMessage;
};

//------------------------------------------------------------------------------
