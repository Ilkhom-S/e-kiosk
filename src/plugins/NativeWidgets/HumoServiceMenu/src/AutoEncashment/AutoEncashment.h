/* @file Плагин автоинскасации терминала */

#pragma once

#include <QtWidgets/QGraphicsProxyWidget>

// Plugin SDK
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/Plugins/IPlugin.h>

#include "AutoEncashmentWindow.h"

class HumoServiceBackend;

//--------------------------------------------------------------------------
class AutoEncashment : public SDK::Plugin::IPlugin, public SDK::GUI::IGraphicsItem {
public:
    AutoEncashment(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath);
    ~AutoEncashment();

#pragma region SDK::Plugin::IPlugin interface

    /// Возвращает название плагина.
    virtual QString getPluginName() const;

    /// Возвращает параметры плагина.
    virtual QVariantMap getConfiguration() const;

    /// Настраивает плагин.
    virtual void setConfiguration(const QVariantMap &aParameters);

    /// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
    virtual QString getConfigurationName() const;

    /// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной
    /// программы).
    virtual bool saveConfiguration();

    /// Проверяет успешно ли инициализировался плагин при создании.
    virtual bool isReady() const;

#pragma endregion

#pragma region SDK::GUI::IGraphicsItem interface

    /// Вызывается перед отображением виджета.
    virtual void show();

    /// Вызывается для сброса/настройки виджета.
    virtual void reset(const QVariantMap &aParameters);

    /// Вызывается перед сокрытием виджета.
    virtual void hide();

    /// Посылает уведомление виджету.
    virtual void notify(const QString &aReason, const QVariantMap &aParameters);

    /// Проверяет готов ли виджет.
    virtual bool isValid() const;

    /// Возвращает описание ошибки.
    virtual QString getError() const;

    // Возвращает виджет.
    virtual QQuickItem *getWidget() const;

    // возвращает нативный виджет.
    virtual QWidget *getNativeWidget() const { return m_AutoEncashmentWindow; }

    /// Возвращает контекст виджета.
    virtual QVariantMap getContext() const;

#pragma endregion

private:
    bool m_IsReady;
    QString m_InstancePath;
    SDK::Plugin::IEnvironment *m_Environment;
    QVariantMap m_Parameters;

    QGraphicsProxyWidget *m_MainWidget;
    AutoEncashmentWindow *m_AutoEncashmentWindow;
    QSharedPointer<HumoServiceBackend> m_Backend;
};

//--------------------------------------------------------------------------
