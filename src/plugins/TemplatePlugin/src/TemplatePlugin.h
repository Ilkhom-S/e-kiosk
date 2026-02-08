/* @file Реализация плагина TemplatePlugin. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/IPlugin.h>

/// Реализация плагина TemplatePlugin.
/// Демонстрирует базовую структуру плагина EKiosk.
class Q_DECL_EXPORT TemplatePlugin : public QObject, public SDK::Plugin::IPlugin {
    Q_OBJECT

public:
    /// Конструктор плагина.
    /// @param aEnvironment Указатель на окружение плагина
    /// @param aInstancePath Путь к экземпляру плагина
    TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath);

    /// Деструктор плагина.
    ~TemplatePlugin();

#pragma region SDK::Plugin::IPlugin interface

    /// IPlugin: Возвращает название плагина.
    /// @return QString с названием плагина
    QString getPluginName() const override;

    /// Возвращает имя файла конфигурации.
    /// @return QString с именем конфигурации
    QString getConfigurationName() const override;

    /// IPlugin: Возвращает текущую конфигурацию.
    /// @return QVariantMap с параметрами плагина
    QVariantMap getConfiguration() const override;

    /// IPlugin: Устанавливает новую конфигурацию.
    /// @param aConfiguration Новые параметры конфигурации
    void setConfiguration(const QVariantMap &aConfiguration) override;

    /// IPlugin: Сохраняет конфигурацию.
    /// @return true если сохранение успешно
    bool saveConfiguration() override;

    /// Проверяет готовность плагина.
    /// @return true если плагин готов к работе
    bool isReady() const override;

#pragma endregion

    /// Возвращает приветственное сообщение.
    /// @return QString с сообщением
    QString getHelloMessage() const;

    /// Выполняет основную работу плагина.
    /// Пример метода для демонстрации функциональности.
    void doWork();

    /// Обрабатывает ошибки плагина.
    /// @param errorMessage Сообщение об ошибке
    void handleError(const QString &errorMessage);

private:
    /// Указатель на окружение плагина.
    SDK::Plugin::IEnvironment *m_Environment;

    /// Путь к экземпляру плагина.
    QString m_InstancePath;

    /// Текущая конфигурация плагина.
    QVariantMap m_Configuration;

    /// Приветственное сообщение.
    QString m_HelloMessage;
};