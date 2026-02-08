/* @file Помощник по конфигурации. */

#include "DeviceConfigManager.h"

#include "Hardware/Protocols/Common/ProtocolUtils.h"

//--------------------------------------------------------------------------------
DeviceConfigManager::DeviceConfigManager() : m_ConfigurationGuard(QReadWriteLock::Recursive) {}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setConfiguration(const QVariantMap &aConfiguration) {
    for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it) {
        setConfigParameter(it.key(), it.value());
    }
}

//--------------------------------------------------------------------------------
QVariantMap DeviceConfigManager::getConfiguration() const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_Configuration;
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setConfigParameter(const QString &aName, const QVariant &aValue) {
    QWriteLocker lock(&m_ConfigurationGuard);

    m_Configuration.insert(aName, aValue);
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::setLConfigParameter(const QString &aName, const QByteArray &aData) {
    QByteArray data = ProtocolUtils::clean(aData.simplified());
    QString value = QString::from_Utf8(data);

    setConfigParameter(aName, value);
}

//--------------------------------------------------------------------------------
QVariant DeviceConfigManager::getConfigParameter(const QString &aName) const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_Configuration.value(aName);
}

//--------------------------------------------------------------------------------
QVariant DeviceConfigManager::getConfigParameter(const QString &aName,
                                                 const QVariant &aDefault) const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_Configuration.value(aName, aDefault);
}

//--------------------------------------------------------------------------------
void DeviceConfigManager::removeConfigParameter(const QString &aName) {
    QWriteLocker lock(&m_ConfigurationGuard);

    m_Configuration.remove(aName);
}

//--------------------------------------------------------------------------------
bool DeviceConfigManager::containsConfigParameter(const QString &aName) const {
    QReadLocker lock(&m_ConfigurationGuard);

    return m_Configuration.contains(aName);
}

//--------------------------------------------------------------------------------
