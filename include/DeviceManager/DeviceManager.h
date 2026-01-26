/* @file Менеджер устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPluginLoader.h>

// Driver SDK
#include <SDK/Drivers/IDevice.h>

#include <Common/ILogable.h>

typedef QMultiMap<SDK::Driver::IDevice *, SDK::Driver::IDevice *> TDeviceDependencyMap;
typedef QPair<QString, SDK::Driver::IDevice *> TNamedDevice;

//--------------------------------------------------------------------------------
class DeviceManager : public QObject, public ILogable {
    Q_OBJECT

  public:
    DeviceManager(SDK::Plugin::IPluginLoader *aPluginLoader);
    ~DeviceManager();

    /// Инициализация DeviceManager.
    bool initialize();

    /// Освобождение ресурсов.
    void shutdown();

    /// Ищет все подключенные устройства без сохранения конфигурации.
    QStringList detect(const QString &aDeviceType);

    /// Подключение/захват устройства, c заданной конфигурацией. По умолчанию конфигурация грузится из конфига плагина.
    SDK::Driver::IDevice *acquireDevice(const QString &aInstancePath, const QString &aConfigPath = "");

    /// Отключение/освобождение указанного устройства.
    void releaseDevice(SDK::Driver::IDevice *aDevice);

    /// Создание устройства. Возвращает имя новой конфигурации.
    Q_INVOKABLE
    TNamedDevice createDevice(const QString &aDriverPath, const QVariantMap &aConfig, bool aDetecting = false);

    /// Сохранить конфигурацию устройства.
    void saveConfiguration(SDK::Driver::IDevice *aDevice);

    /// Получение списка драйверов (поддерживаемых устройств).
    QMap<QString, QStringList> getModelList(const QString &aFilter);

    /// Получение списка доступных драйверов.
    QStringList getDriverList() const;

    /// Останавливает процесс поиска устройств.
    void stopDetection();

    /// Возвращает список созданных устройств.
    QStringList getAcquiredDevicesList() const;

    /// Устанавливает конфигурацию устройству.
    void setDeviceConfiguration(SDK::Driver::IDevice *aDevice, const QVariantMap &aConfig);

    /// Получить конфигурацию устройства.
    QVariantMap getDeviceConfiguration(SDK::Driver::IDevice *aDevice) const;

    /// Получить список возможных значений для параметров устройств.
    SDK::Plugin::TParameterList getDriverParameters(const QString &aDriverPath) const;

    /// Проверить правильность путей плагинов драйверов. Workaround для обратной совместимости при накате обновления без
    /// авто поиска драйверов, при изменении путей.
    // TODO: убрать после реализации авто поиска через мониторинг.
    void checkInstancePath(QString &aInstancePath);
    void checkITInstancePath(QString &aInstancePath);

    /// Попробовать изменить путь плагина драйвера. Workaround для обратной совместимости при накате обновления без
    /// авто поиска драйверов, при изменении путей.
    typedef QSet<QString> TNewPaths;
    typedef QMap<QString, TNewPaths> TPaths;
    void changeInstancePath(QString &aInstancePath, const QString &aConfigPath, const TPaths &aPaths);

  private:
    /// Пробует найти устройство aDevicePath.
    TNamedDevice findDevice(SDK::Driver::IDevice *aRequired, const QStringList &aDevicesToFind);

    /// Пробует найти устройство aDriverName, не имеющее зависимых устройств
    typedef QList<TNamedDevice> TFallbackDevices;
    void findSimpleDevice(const QString &aDriverName, QStringList &aResult, TFallbackDevices &aFallbackDevices,
                          QStringList &aNonMarketDetectedDriverNames);

    /// Было ли устройство данного типа уже найдено.
    bool isDetected(const QString &aConfigName);

    /// Добавить устройство в список найденных.
    void markDetected(const QString &aConfigName);

    /// Предикат для сортировки списка устройств при автодетекте.
    bool deviceSortPredicate(const QString &aLhs, const QString &aRhs) const;

    /// Установить лог для устройства.
    void setDeviceLog(SDK::Driver::IDevice *aDevice, bool aDetecting);

    /// Получить имя лога простого устройства.
    QString getSimpleDeviceLogName(SDK::Driver::IDevice *aDevice, bool aDetecting);

    /// Получить имя лога устройства.
    QString getDeviceLogName(SDK::Driver::IDevice *aDevice, bool aDetecting);

    /// Логировать данные зависимых устройств.
    void logRequiredDeviceData();

  signals:
    /// Сигнал об обнаружении нового устройства.
    void deviceDetected(const QString &aConfigName, SDK::Driver::IDevice *device);

    /// Сигнал прогресса.
    void progress(int aCurrent, int aMax);

  private slots:
    /// Обновить конфигурацию устройства (без зависимых устройств).
    void onConfigurationChanged();

  private:
    SDK::Plugin::IPluginLoader *mPluginLoader;

    /// Список свободных системных зависимых устройств
    QSet<QString> mFreeSystemNames;

    /// Таблица принадлежности драйвера и системного имени зависимого устройства
    QMultiMap<QString, QString> mRDSystemNames;

    /// Таблица имен требуемых ресурсов.
    QMap<QString, QString> mRequiredResources;

    /// Таблица зависимостей.
    TDeviceDependencyMap mDeviceDependencyMap;

    /// Таблица драйверов для конкретного устройства.
    QMap<QString, QStringList> mDriverList;

    /// Флаг остановки поиска устройств.
    char mStopFlag;

    /// Таблица всех возможных значений параметров устройств.
    QMap<QString, SDK::Plugin::TParameterList> mDriverParameters;

    /// Список типов найденных устройств.
    QSet<QString> mDetectedDeviceTypes;
    QMutex mAccessMutex;

    /// Данные логов устройств.
    typedef QMap<bool, int> TLogData;
    typedef QMap<SDK::Driver::IDevice *, TLogData> TDevicesLogData;
    TDevicesLogData mDevicesLogData;

    /// Список путей всех использованных устройств.
    QStringList mAcquiredDevices;
};

//--------------------------------------------------------------------------------
