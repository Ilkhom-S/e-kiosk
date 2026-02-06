/* @file Прото-Mifare-кард-ридер. */

#pragma once

#include <SDK/Drivers/IMifareReader.h>

#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoMifareReader : public ProtoDevice, public MetaDevice<SDK::Driver::IMifareReader> {
    Q_OBJECT

    SET_DEVICE_TYPE(CardReader)

signals:
    /// Карта вставлена.
    void inserted(SDK::Driver::ECardType::Enum, const QVariantMap &);

    /// Карта извлечена.
    void ejected();
};

//--------------------------------------------------------------------------------
