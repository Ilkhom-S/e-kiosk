/* @file Сервис для работы с настройками. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

class SettingsManager;
class IApplication;

//---------------------------------------------------------------------------
class SettingsService : public SDK::PaymentProcessor::IService,
                        public SDK::PaymentProcessor::ISettingsService {
public:
    explicit SettingsService(IApplication *aApplication);
    virtual ~SettingsService() override;

    /// IService: инициализация сервиса.
    virtual bool initialize() override;

    /// IService: Закончена инициализация всех сервисов.
    virtual void finishInitialize() override;

    /// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
    virtual bool canShutdown() override;

    /// IService: остановка сервиса.
    virtual bool shutdown() override;

    /// IService: имя сервиса.
    virtual QString getName() const override;

    /// IService: список зависимостей.
    virtual const QSet<QString> &getRequiredServices() const override;

    /// Получить параметры сервиса.
    virtual QVariantMap getParameters() const override;

    /// Сброс служебной информации.
    virtual void resetParameters(const QSet<QString> &aParameters) override;

    /// ISettingsService: получить настройки.
    virtual SDK::PaymentProcessor::ISettingsAdapter *
    getAdapter(const QString &aAdapterName) override;

    /// ISettingsService: сохранить настройки.
    virtual bool saveConfiguration() override;

    SettingsManager *getSettingsManager() const;

    /// Получить адаптер настроек по его имени.
    template <typename T> T *getAdapter() {
        QString adapterName = T::getAdapterName();
        return static_cast<T *>(
            m_SettingsAdapters.contains(adapterName) ? m_SettingsAdapters[adapterName] : 0);
    }

    /// Возвращает список адаптеров.
    QList<SDK::PaymentProcessor::ISettingsAdapter *> enumerateAdapters() const;

    /// Получить экземпляр сервиса.
    static SettingsService *instance(IApplication *aApplication);

private:
    SettingsManager *m_SettingsManager;
    bool m_RestoreConfiguration;
    QMap<QString, SDK::PaymentProcessor::ISettingsAdapter *> m_SettingsAdapters;

    IApplication *m_Application;
};

//---------------------------------------------------------------------------
