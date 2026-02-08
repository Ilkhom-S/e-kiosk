/* @file Config cleaner */

#pragma once

#include <QtCore/QStringList>

#include "DeviceConfigManager.h"

//---------------------------------------------------------------------------
class ConfigCleaner {
public:
    ConfigCleaner(DeviceConfigManager *aConfigManager, const QStringList &aKeys)
        : m_Performer(aConfigManager), m_Keys(aKeys) {}

    ~ConfigCleaner() {
        if (m_Performer) {
            foreach (const QString &key, m_Keys) {
                m_Performer->removeConfigParameter(key);
            }
        }
    }

private:
    /// Класс-владелец конфига.
    DeviceConfigManager *m_Performer;

    /// Список ключей конфига для удаления данных.
    QStringList m_Keys;
};

//--------------------------------------------------------------------------------
