/* @file Объявление класса менеджера плагинов. */

#pragma once

#include <QtCore/QFutureSynchronizer>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QPluginLoader>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>

// Plugin SDK
#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IServiceState.h>
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPluginLoader.h>

#include "System/IApplication.h"

//------------------------------------------------------------------------------
class PluginService : public QObject,
                      public ILogable,
                      public SDK::PaymentProcessor::IService,
                      public SDK::PaymentProcessor::IServiceState,
                      private SDK::Plugin::IKernel {
    Q_OBJECT

public:
    PluginService(IApplication *aApplication);
    virtual ~PluginService();

    static PluginService *instance(IApplication *aApplication);

    /// Методы интерфейса IService

    /// Инициализация сервиса.
    virtual bool initialize();

    /// IService: Закончена инициализация всех сервисов.
    virtual void finishInitialize();

    /// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
    virtual bool canShutdown();

    /// Завершение работы сервиса.
    virtual bool shutdown();

    /// Возвращает имя сервиса.
    virtual QString getName() const;

    /// Список зависимостей.
    virtual const QSet<QString> &getRequiredServices() const;

    /// Получить параметры сервиса.
    virtual QVariantMap getParameters() const;

    /// Сброс служебной информации.
    virtual void resetParameters(const QSet<QString> &aParameters);

    /// Возвращает список загруженных плагинов
    virtual QString getState() const;

    /// Возвращает загрузчик плагинов.
    virtual SDK::Plugin::IPluginLoader *getPluginLoader();

    /// Методы интерфейса SDK::Plugin::IKernel
    /// Возвращает логгер.
    virtual ILog *getLog(const QString &aName = "") const;

    /// Возвращает версию ядра.
    virtual QString getVersion() const;

    /// Возвращает текущий язык приложения.
    virtual QString getLanguage() const;

    /// Возвращает рабочую папку ядра.
    virtual QString getDirectory() const;

    /// Возвращает каталог для хранения данных/конфигов приложения.
    virtual QString getDataDirectory() const;

    /// Возвращает каталог для хранения лог-файлов приложения.
    virtual QString getLogsDirectory() const;

    /// Возвращает корневую папку для расширений.
    virtual QString getPluginDirectory() const;

    /// Сообщает имеется ли для данной пары плагин-объект конфигурация.
    /// Используется для хранения конфигураций плагинов в прикладной программе.
    virtual bool canConfigurePlugin(const QString &aInstancePath) const;

    /// Возвращает конфигурацию для данной пары плагин-объект.
    /// Используется для хранения конфигураций плагинов в прикладной программе.
    virtual QVariantMap getPluginConfiguration(const QString &aInstancePath) const;

    /// Сообщает сможет ли сохранить конфигурацию для данной пары плагин-объект.
    /// Используется для хранения конфигураций плагинов в прикладной программе.
    virtual bool canSavePluginConfiguration(const QString &aInstancePath) const;

    /// Сохраняет конфигурацию для данной пары плагин-объект.
    /// Используется для хранения конфигураций плагинов в прикладной программе.
    virtual bool savePluginConfiguration(const QString &aInstancePath,
                                         const QVariantMap &aParameters);

    virtual SDK::Plugin::IExternalInterface *getInterface(const QString &aInterface);

    /// Возвращает загрузчик плагинов.
    using SDK::Plugin::IKernel::getPluginLoader;
    virtual SDK::Plugin::IPluginLoader *getPluginLoader() const { return m_PluginLoader; }

private:
    /// Проверяет плагин на валидность и если необходимо запрещает его загрузку.
    void verifyPlugins();

private:
    /// Интерфейс приложения.
    IApplication *m_Application;

    /// Загрузчик плагинов.
    SDK::Plugin::IPluginLoader *m_PluginLoader;

    /// Потоки проверки подписи плагинов.
    QFutureSynchronizer<void> m_PluginVerifierSynchronizer;

    QMap<QString, QString> m_SignedPlugins;
    QStringList m_UnsignedPlugins;

    /// Кешированный путь к логам (избегаем создания temporary QString через DLL boundary)
    QString m_LogsDirectory;

    /// Кешированный путь к данным (избегаем создания temporary QString через DLL boundary)
    QString m_DataDirectory;
};

//------------------------------------------------------------------------------
