/* @file Config cleaner */

#pragma once

#include <QtCore/QStringList>

#include "DeviceConfigManager.h"

//---------------------------------------------------------------------------
class ConfigCleaner {
public:
    ConfigCleaner(DeviceConfigManager *aConfigManager, const QStringList &aKeys)
        : mPerformer(aConfigManager), mKeys(aKeys) {}

    ~ConfigCleaner() {
        if (mPerformer) {
            foreach (const QString &key, mKeys) {
                mPerformer->removeConfigParameter(key);
            }
        }
    }

private:
    /// Класс-владелец конфига.
    DeviceConfigManager *mPerformer;

    /// Список ключей конфига для удаления данных.
    QStringList mKeys;
};

//--------------------------------------------------------------------------------
