/* @file Реализация сервиса для работы с устройствами. */

#pragma once

#include <QtCore/QFutureWatcher>
#include <QtCore/QMutex>
#include <QtCore/QSet>
#include <QtCore/QStringList>

#include <Common/ILog.h>

// PP Core
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginParameters.h>

#include "IntegratedDrivers.h"

class IApplication;
class IHardwareDatabaseUtils;
class DeviceManager;

// TODO #29565 - метод перезаписи статуса модема, пока не отрефакторили соединение и модем в единое
// целое
#pragma deprecated(overwriteDeviceStatus)

/// Варианты момента создания устройства
namespace EDeviceCreationOrder {
enum Enum { OnDemand, AtStart };
} // namespace EDeviceCreationOrder

//------------------------------------------------------------------------------
/// Реализация сервиса для работы с устройствами.
class DeviceService : public SDK::PaymentProcessor::IDeviceService,
                      public SDK::PaymentProcessor::IService {
    Q_OBJECT

public:
    /// структура - статус устройства
    class Status : public SDK::PaymentProcessor::IDeviceStatus {
    public:
        Status();
        Status(const Status &aStatus);
        explicit Status(SDK::Driver::EWarningLevel::Enum aLevel, QString aDescription, int aStatus);

    public:
        /// Уровень тревожности
        virtual SDK::Driver::EWarningLevel::Enum level() const;

        /// Описание статуса
        virtual const QString &description() const;

        /// Проверить содержимое статуса на удовлетворение определенному уровню
        bool isMatched(SDK::Driver::EWarningLevel::Enum aLevel) const;

    public:
        SDK::Driver::EWarningLevel::Enum m_Level;
        QString m_Description;
        int m_Status;
    };

public:
    /// Получение экземпляра DeviceService.
    static DeviceService *instance(IApplication *aApplication);

    DeviceService(IApplication *aApplication);
    virtual ~DeviceService();

    /// Инициализация сервиса.
    virtual bool initialize();

    /// IService: Закончена инициализация всех сервисов.
    virtual void finishInitialize();

    /// Возвращает false, если сервис не может быть остановлен в текущий момент.
    virtual bool canShutdown();

    /// Завершение работы сервиса.
    virtual bool shutdown();

    /// Возвращает имя сервиса.
    virtual QString getName() const;

    /// Получение списка зависимостей.
    virtual const QSet<QString> &getRequiredServices() const;

    /// Получить параметры сервиса.
    virtual QVariantMap getParameters() const;

    /// Сброс служебной информации.
    virtual void resetParameters(const QSet<QString> &aParameters);

    // IDeviceService

    /// Неблокирующий поиск всех устройств.
    virtual void detect(const QString &aDeviceType);

    /// Прервать поиск устройств.
    virtual void stopDetection();

    /// Получить полный список конфигураций.
    virtual QStringList getConfigurations(bool aAllowOldConfigs = true) const;

    /// Сохранить cписок конфигураций.
    virtual bool saveConfigurations(const QStringList &aConfigList);

    /// Добавить список параметров, необходимых для инициализации устройств.
    virtual void setInitParameters(const QString &aDeviceType, const QVariantMap &aParameters);

    /// Подключение/захват устройства. aDeviceNumber - номер среди одинаковых устройств.
    virtual SDK::Driver::IDevice *acquireDevice(const QString &aInstancePath);

    /// Создание устройства.
    virtual QString createDevice(const QString &aDriverPath, const QVariantMap &aConfig);

    /// Отключение/освобождение указанного устройства.
    virtual void releaseDevice(SDK::Driver::IDevice *aDevice);

    /// Обновить прошивку устройства.
    virtual UpdateFirmwareResult updateFirmware(const QByteArray &aFirmware,
                                                const QString &aDeviceGUID);

    /// Получение списка параметров драйвера.
    virtual SDK::Plugin::TParameterList getDriverParameters(const QString &aDriverPath) const;

    /// Получить конфигурацию утройства и всех, связанных с ним.
    virtual QVariantMap getDeviceConfiguration(const QString &aConfigName);

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QString &aConfigName, const QVariantMap &aConfig);

    /// Получение списка драверов (поддерживаемых устройств).
    virtual SDK::PaymentProcessor::TModelList getModelList(const QString &aFilter) const;

    /// Получение списка драйверов.
    virtual QStringList getDriverList() const;

    /// Возвращает список имен созданных устройств.
    virtual QStringList getAcquiredDevicesList() const;

    /// Получить имя конфига по устроойству.
    virtual QString getDeviceConfigName(SDK::Driver::IDevice *aDevice);

    /// Получить статус устройства по имени конфигурации.
    virtual QSharedPointer<SDK::PaymentProcessor::IDeviceStatus>
    getDeviceStatus(const QString &aConfigName);

    /// Освобождает все устройства.
    virtual void releaseAll();

    /// Перезаписать статус устройства (#29565)
    virtual void overwriteDeviceStatus(SDK::Driver::IDevice *aDevice,
                                       SDK::Driver::EWarningLevel::Enum aLevel,
                                       const QString &aDescription,
                                       int aStatus);

private:
    void doDetect(const QString &aDeviceType);
    bool initializeDevice(const QString &aConfigName, SDK::Driver::IDevice *aDevice);
    void statusChanged(SDK::Driver::IDevice *aDevice, Status &aStatus);

private slots:
    void onDeviceDetected(const QString &aConfigName, SDK::Driver::IDevice *aDevice);
    void onDetectionFinished();
    void onDeviceStatus(SDK::Driver::EWarningLevel::Enum aLevel,
                        const QString &aDescription,
                        int aStatus);

private:
    DeviceManager *m_DeviceManager;

    /// Общий список параметров, необходимых для инициализации устройств.
    QMap<QString, QVariantMap> m_InitParameters;

    /// Здесь хранится результат запуска detectа.
    QFutureWatcher<void> m_DetectionResult;

    mutable QRecursiveMutex m_AccessMutex;

    /// Список всех использованных устройств.
    typedef QMap<QString, SDK::Driver::IDevice *> TAcquiredDevices;
    TAcquiredDevices m_AcquiredDevices;

    /// Кэш для статусов устройств.
    QMap<QString, Status> m_DeviceStatusCache;

    /// Параметр момента создания устройств
    QMap<QString, int> m_DeviceCreationOrder;

    IApplication *m_Application;
    ILog *m_Log;
    IHardwareDatabaseUtils *m_DatabaseUtils;

    IntegratedDrivers m_IntegratedDrivers;
};

//------------------------------------------------------------------------------
