/* @file Прото-HID-устройство. */

#pragma once

#include <QtCore/QVariant>

#include <SDK/Drivers/IHID.h>

#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoHID : public ProtoDevice, public MetaDevice<SDK::Driver::IHID> {
    Q_OBJECT

    SET_DEVICE_TYPE(Scanner)

signals:
    /// Событие о новых введённых данных.
    void data(const QVariantMap &aData);
};

//--------------------------------------------------------------------------------
