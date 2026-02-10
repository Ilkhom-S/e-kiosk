/* @file Бэкенд для визуализации QML виджетов. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtQml/QQmlEngine>

#include <SDK/GUI/IGraphicsBackend.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginFactory.h>

#include <memory>

#include "QMLGraphicsItem.h"

//------------------------------------------------------------------------------
class QMLBackend : public QObject, public SDK::Plugin::IPlugin, public SDK::GUI::IGraphicsBackend {
    Q_OBJECT

public:
    QMLBackend(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath);

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

#pragma region SDK::GUI::IGraphicsBackend interface

    /// Инициализируем
    virtual bool initialize(SDK::GUI::IGraphicsEngine *aEngine);

    /// Очищаем ресурсы
    virtual void shutdown();

    /// Создаёт (или возвращает из кэша) графический элемент по описанию.
    virtual std::weak_ptr<SDK::GUI::IGraphicsItem> getItem(const SDK::GUI::GraphicsItem_Info &aInfo);

    /// Удаляет графический элемент по описанию
    virtual bool removeItem(const SDK::GUI::GraphicsItem_Info &aInfo);

    /// Возвращает тип движка.
    virtual QString getType() const;

    /// Возвращает список экранов, с которыми работает бэкэнд
    virtual QList<SDK::GUI::GraphicsItem_Info> getItem_List();

#pragma endregion

private slots:
    void onWarnings(const QList<QQmlError> &aWarnings);

private:
    typedef QMultiMap<QString, std::shared_ptr<QMLGraphicsItem>> TGraphicItemsCache;

    QString m_InstancePath;
    QVariantMap m_Parameters;

    SDK::Plugin::IEnvironment *m_Factory;
    SDK::GUI::IGraphicsEngine *m_Engine;
    SDK::PaymentProcessor::ICore *m_Core;

    QQmlEngine m_QMLEngine;
    TGraphicItemsCache m_CachedItems;
};

//------------------------------------------------------------------------------
