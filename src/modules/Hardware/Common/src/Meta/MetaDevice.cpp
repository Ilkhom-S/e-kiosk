/* @file Базовый класс устройства. */

#include "MetaDevice.h"

#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <QtCore/QtAlgorithms>

#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/IIOPort.h>
#include <SDK/Drivers/IMifareReader.h>
#include <SDK/Drivers/IModem.h>
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IWatchdog.h>

using namespace SDK::Driver;

// MetaDevice constants definitions
namespace CMetaDevice {
const char DefaultName[] = "Meta device";
} // namespace CMetaDevice

//-------------------------------------------------------------------------------
template <class T>
MetaDevice<T>::MetaDevice()
    : m_DeviceName(CMetaDevice::DefaultName), m_LogDate(QDate::currentDate()),
      m_OperatorPresence(false), m_FiscalServerPresence(false), m_DetectingPosition(0),
      m_Initialized(ERequestStatus::Fail), m_ExitTimeout(ULONG_MAX), m_InitializationError(false) {}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::subscribe(const char * /*aSignal*/,
                              QObject * /*aReceiver*/,
                              const char * /*aSlot*/) {
    return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::unsubscribe(const char * /*aSignal*/, QObject * /*aReceiver*/) {
    return false;
}

//--------------------------------------------------------------------------------
template <class T> QString MetaDevice<T>::getName() const {
    QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();

    return deviceName.isEmpty() ? m_DeviceName : deviceName;
}

//--------------------------------------------------------------------------------
template <class T> void MetaDevice<T>::initialize() {
    logDeviceData(getDeviceData());

    m_Initialized = ERequestStatus::Success;
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::release() {
    if (m_Thread.isRunning()) {
        m_Thread.quit();

        if (!isWorkingThread()) {
            m_Thread.wait(m_ExitTimeout);
        }
    }

    m_Initialized = ERequestStatus::Fail;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void MetaDevice<T>::updateFirmware(const QByteArray & /*aBuffer*/) {}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::canUpdateFirmware() {
    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::isAutoDetecting() const {
    return getConfigParameter(CHardwareSDK::SearchingType).toString() ==
           CHardwareSDK::SearchingTypes::AutoDetecting;
}

//--------------------------------------------------------------------------------
template <class T> void MetaDevice<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it) {
        setConfigParameter(it.key(), it.value());
    }

    m_OperatorPresence =
        aConfiguration.value(CHardwareSDK::OperatorPresence, m_OperatorPresence).toBool();
    m_FiscalServerPresence =
        aConfiguration.value(CHardwareSDK::FiscalServerPresence, m_FiscalServerPresence).toBool();
}

//--------------------------------------------------------------------------------
template <class T> QVariantMap MetaDevice<T>::getDeviceConfiguration() const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_Configuration;
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::setDeviceParameter(const QString &aName,
                                       const QVariant &aValue,
                                       const QString &aExtensibleName,
                                       bool aUpdateExtensible) {
    QString value = aValue.toString().simplified();

    // В Qt 6 QVariant::type() помечен как устаревший (obsolete).
    // QVariant::typeId() — современная и более производительная замена, доступная и в 5.15.
    auto typeId = aValue.typeId();

    if (typeId == QMetaType::QByteArray) {
        value = ProtocolUtils::clean(aValue.toByteArray());
    } else if (typeId == QMetaType::QString) {
        value = ProtocolUtils::clean(aValue.toString());
    } else if (typeId == QMetaType::Bool) {
        value = aValue.toBool() ? CDeviceData::Values::Yes : CDeviceData::Values::No;
    }

    if (aExtensibleName.isEmpty()) {
        QWriteLocker locker(&m_ConfigurationGuard);
        m_DeviceData.insert(aName, value);
    } else if (!value.isEmpty()) {
        // Использование QStringBuilder (через % вместо +) позволяет избежать
        // создания промежуточных объектов строк при конкатенации.
        value = aName % " " % value;

        if (!aUpdateExtensible) {
            QReadLocker locker(&m_ConfigurationGuard);

            // Использование value() вместо оператора [] предотвращает случайное
            // создание пустых записей в контейнере при чтении.
            QString extensibleValue = m_DeviceData.value(aExtensibleName);

            if (!extensibleValue.isEmpty()) {
                value = extensibleValue % ", " % value;
            }
        }

        {
            QWriteLocker locker(&m_ConfigurationGuard);
            m_DeviceData.insert(aExtensibleName, value);
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> QVariant MetaDevice<T>::getDeviceParameter(const QString &aName) const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_DeviceData.value(aName);
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::containsDeviceParameter(const QString &aName) const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_DeviceData.contains(aName) && !m_DeviceData.value(aName).isEmpty();
}

//--------------------------------------------------------------------------------
template <class T> void MetaDevice<T>::removeDeviceParameter(const QString &aName) {
    QWriteLocker lock(&m_ConfigurationGuard);

    m_DeviceData.remove(aName);
}

//---------------------------------------------------------------------------
template <class T> void MetaDevice<T>::logDeviceData(const SLogData &aData) const {
    toLog(LogLevel::Normal, "Plugin path: " + getConfigParameter(CHardware::PluginPath).toString());

    if (!aData.plugin.isEmpty())
        toLog(LogLevel::Normal, "Plugin data:" + aData.plugin);
    if (!aData.device.isEmpty())
        toLog(LogLevel::Normal, "Device data:" + aData.device);
    if (!aData.config.isEmpty())
        toLog(LogLevel::Normal, "Config data:" + aData.config);

    QReadLocker lock(&m_ConfigurationGuard);

    if (m_Configuration[CHardwareSDK::RequiredDevice].template value<IDevice *>() &&
        !aData.requiredDevice.isEmpty()) {
        toLog(LogLevel::Normal, "Required device data:" + aData.requiredDevice);
    }
}

//---------------------------------------------------------------------------
template <class T> SLogData MetaDevice<T>::getDeviceData() const {
    QReadLocker lock(&m_ConfigurationGuard);

    IDevice *requiredDevice =
        m_Configuration.value(CHardwareSDK::RequiredDevice).template value<IDevice *>();
    SLogData result;

    if (requiredDevice) {
        TDeviceData data;
        QStringList names = getConfigParameter(CHardware::RequiredResourceNames).toStringList();
        QVariantMap configuration = requiredDevice->getDeviceConfiguration();

        foreach (const QString &name, names) {
            QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
            data.insert(key, configuration[key].toString());
        }

        result.requiredDevice = DeviceUtils::getPartDeviceData(data);
    }

    TDeviceData data;
    QStringList names = getConfigParameter(CHardware::PluginParameterNames).toStringList();

    foreach (const QString &name, names) {
        QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
        data.insert(key, getConfigParameter(key).toString());
    }

    result.plugin = DeviceUtils::getPartDeviceData(data, false);
    result.device = DeviceUtils::getPartDeviceData(m_DeviceData, false);

    QVariantMap dealerSettings = getConfigParameter(CHardware::ConfigData).toMap();
    names = dealerSettings.keys();
    data.clear();

    foreach (const QString &name, names) {
        QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
        data.insert(key, dealerSettings[key].toString());
    }

    result.config = DeviceUtils::getPartDeviceData(data);

    return result;
}

//--------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *MetaDevice<T>::getDetectingIterator() {
    m_DetectingPosition = 0;

    return this;
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::find() {
    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::moveNext() {
    return (m_DetectingPosition++ == 0);
}

//--------------------------------------------------------------------------------
template <class T> void MetaDevice<T>::setLog(ILog *aLog) {
    m_Log = aLog;
}

//--------------------------------------------------------------------------------
template <class T> bool MetaDevice<T>::isWorkingThread() {
    return &m_Thread == QThread::currentThread();
}

//--------------------------------------------------------------------------------

// Explicit template instantiations
template class MetaDevice<ICardReader>;
template class MetaDevice<ICashAcceptor>;
template class MetaDevice<IDevice>;
template class MetaDevice<IDispenser>;
template class MetaDevice<IFiscalPrinter>;
template class MetaDevice<IHID>;
template class MetaDevice<IIOPort>;
template class MetaDevice<IMifareReader>;
template class MetaDevice<IModem>;
template class MetaDevice<IPrinter>;
template class MetaDevice<IWatchdog>;
