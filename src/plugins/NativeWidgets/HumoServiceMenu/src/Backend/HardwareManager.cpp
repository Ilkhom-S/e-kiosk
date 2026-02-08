/* @file Менеджер для работы с железом */

#include "HardwareManager.h"

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Plugins/IPluginLoader.h>

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
namespace CHardwareManager {
const QString StatusOK = "#1d7e00";
const QString StatusWarning = "#ffc014";
const QString StatusError = "#7e0000";
} // namespace CHardwareManager

//------------------------------------------------------------------------
HardwareManager::HardwareManager(SDK::Plugin::IEnvironment *aFactory, PPSDK::ICore *aCore)
    : m_Factory(aFactory), m_Core(aCore) {
    m_DeviceService = m_Core->getDeviceService();

    connect(m_DeviceService,
            SIGNAL(deviceDetected(const QString &)),
            this,
            SIGNAL(deviceDetected(const QString &)));
    connect(m_DeviceService, SIGNAL(detectionStopped()), this, SIGNAL(detectionStopped()));
    connect(m_DeviceService,
            SIGNAL(deviceStatusChanged(
                const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)),
            this,
            SLOT(deviceStatusChanged(
                const QString &, SDK::Driver::EWarningLevel::Enum, const QString &)));

    foreach (QString config, m_DeviceService->getConfigurations(false)) {
        m_CurrentConfiguration[config] = m_DeviceService->getDeviceConfiguration(config);
    }
}

//------------------------------------------------------------------------
HardwareManager::~HardwareManager() {}

//------------------------------------------------------------------------
bool HardwareManager::isConfigurationChanged() const {
    return !(m_CurrentConfiguration == getConfiguration());
}

//------------------------------------------------------------------------
void HardwareManager::resetConfiguration() {
    m_CurrentConfiguration = getConfiguration();
}

//------------------------------------------------------------------------
void HardwareManager::detect(const QString &aDeviceType) {
    m_DeviceService->detect(aDeviceType);
}

//------------------------------------------------------------------------
void HardwareManager::stopDetection() {
    m_DeviceService->stopDetection();
}

//------------------------------------------------------------------------
QStringList HardwareManager::getConfigurations() const {
    return m_DeviceService->getConfigurations();
}

//------------------------------------------------------------------------
void HardwareManager::setConfigurations(const QStringList &aConfigurations) {
    m_DeviceService->saveConfigurations(aConfigurations);
}

//------------------------------------------------------------------------
QVariantMap HardwareManager::getConfiguration() const {
    QVariantMap result;
    QStringList configList(m_DeviceService->getConfigurations());

    foreach (QString config, configList) {
        result[config] = getDeviceConfiguration(config);
    }

    return result;
}

//------------------------------------------------------------------------
bool HardwareManager::setConfiguration(const QVariantMap &aConfig) {
    if (m_CurrentConfiguration == aConfig) {
        return true;
    }

    // Обновляем параметры устройств
    foreach (QString config, aConfig.keys()) {
        m_DeviceService->setDeviceConfiguration(config, aConfig[config].toMap());
    }

    return m_DeviceService->saveConfigurations(aConfig.keys());
}

//------------------------------------------------------------------------
QVariantMap HardwareManager::getDeviceConfiguration(const QString &aConfigName) const {
    QVariantMap config = m_DeviceService->getDeviceConfiguration(aConfigName);

    // Если конфигурация пустая, то не найден необходимый ресурс. Какой-нибудь сом-порт.
    // Попробуем получить описание через плагин устройства
    if (config.isEmpty()) {
        SDK::Plugin::IPlugin *plugin =
            m_Factory->getPluginLoader()->createPlugin(aConfigName, aConfigName);
        SDK::Driver::IDevice *device = dynamic_cast<SDK::Driver::IDevice *>(plugin);

        if (device) {
            config = device->getDeviceConfiguration();
        }
    }

    return config;
}

//------------------------------------------------------------------------
void HardwareManager::setDeviceConfiguration(const QString &aConfigurationName,
                                             const QVariantMap &aConfig) {
    m_DeviceService->setDeviceConfiguration(aConfigurationName, aConfig);
}

//------------------------------------------------------------------------
QStringList HardwareManager::getDriverList() const {
    return m_DeviceService->getDriverList();
}

//------------------------------------------------------------------------
PPSDK::TModelList HardwareManager::getModelList(const QString &aFilter) const {
    return m_DeviceService->getModelList(aFilter);
}

//------------------------------------------------------------------------
SDK::Plugin::TParameterList HardwareManager::getDriverParameters(const QString &aDriverPath) const {
    return m_DeviceService->getDriverParameters(aDriverPath);
}

//------------------------------------------------------------------------
QString HardwareManager::createDevice(const QString &aDriverPath, const QVariantMap &aConfig) {
    return m_DeviceService->createDevice(aDriverPath, aConfig);
}

//------------------------------------------------------------------------
bool HardwareManager::checkDevice(const QString &aConfigName) {
    DSDK::IDevice *device = m_DeviceService->acquireDevice(aConfigName);

    if (device) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------
DSDK::IDevice *HardwareManager::getDevice(const QString &aConfigName) {
    return m_DeviceService->acquireDevice(aConfigName);
}

//------------------------------------------------------------------------
void HardwareManager::releaseDevice(const QString &aConfigName) {
    DSDK::IDevice *device(getDevice(aConfigName));

    if (device) {
        m_DeviceService->releaseDevice(device);
    }
}

//------------------------------------------------------------------------
void HardwareManager::releaseAll() {
    m_DeviceService->releaseAll();
}

//------------------------------------------------------------------------
void HardwareManager::updateStatuses() {
    QStringList configurations(getConfigurations());

    foreach (const QString &configuration, configurations) {
        auto status = m_DeviceService->getDeviceStatus(configuration);
        if (status) {
            deviceStatusChanged(configuration, status->level(), status->description());
        }
    }
}

//------------------------------------------------------------------------
void HardwareManager::deviceStatusChanged(const QString &aConfigName,
                                          DSDK::EWarningLevel::Enum aLevel,
                                          const QString &aDescription) {
    QString newStatus(aDescription);

    if (newStatus.isEmpty()) {
        newStatus = tr("#unknown_status");
    }

    QString statusColor;

    switch (aLevel) {
    case DSDK::EWarningLevel::OK: {
        statusColor = CHardwareManager::StatusOK;
        break;
    }
    case DSDK::EWarningLevel::Warning: {
        statusColor = CHardwareManager::StatusWarning;
        break;
    }
    case DSDK::EWarningLevel::Error: {
        statusColor = CHardwareManager::StatusError;
        break;
    }
    }

    emit deviceStatusChanged(aConfigName, newStatus, statusColor, aLevel);
}

//---------------------------------------------------------------------------
bool HardwareManager::isFiscalPrinterPresent(bool aVirtual, bool aCheckPrintFullZReport) {
    bool isFiscal = false;
    bool isVirtualFiscal = false;

    // Проверяем наличие пусть даже и облачного ФР
    bool haveVirtualFiscal =
        m_Core->getPrinterService()->canPrintReceipt(PPSDK::CReceiptType::ZReport, false);

    // Проверяем наличие аппаратного ФР в фискальном режиме или принтера для виртуального
    // фискальника
    QStringList configNames = getConfigurations();

    foreach (const QString config, configNames) {
        QString deviceType = config.section(".", 2, 2);

        if (aVirtual && DSDK::CComponents::isPrinter(deviceType)) {
            auto device = getDevice(config);
            auto printer = device ? dynamic_cast<DSDK::IPrinter *>(device) : nullptr;

            isVirtualFiscal =
                isVirtualFiscal || (haveVirtualFiscal && printer->isDeviceReady(false));
        }

        if ((deviceType == DSDK::CComponents::DocumentPrinter) ||
            (deviceType == DSDK::CComponents::FiscalRegistrator)) {
            auto device = getDevice(config);
            auto fiscalPrinter = device ? dynamic_cast<DSDK::IFiscalPrinter *>(device) : nullptr;

            isFiscal =
                isFiscal || (fiscalPrinter && fiscalPrinter->isFiscal() &&
                             fiscalPrinter->isDeviceReady(false) &&
                             (aCheckPrintFullZReport ? fiscalPrinter->canProcessZBuffer() : true));
        }
    }

    return aVirtual ? (isFiscal || isVirtualFiscal) : isFiscal;
}

//------------------------------------------------------------------------
