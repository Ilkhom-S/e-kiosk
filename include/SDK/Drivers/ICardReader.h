/* @file Интерфейс драйвера кард-ридера. */
#pragma once

#include <QtCore/QMetaType>

#include <SDK/Drivers/CardReader/CardReaderStatus.h>
#include <SDK/Drivers/IDevice.h>

//------------------------------------------------------------------------------
namespace SDK {
namespace Driver {

/// Типы карт.
namespace ECardType {
enum Enum {
    RF, /// Радиочастотные.

    MS,    /// С магнитной полосой.
    IC,    /// С чипом.
    MSIC,  /// С магнитной полосой и чипом.
    MSICRF /// Радиочастотные c магнитной полосой и чипом.
};
} // namespace ECardType

class ICardReader : public IDevice {
public:
    /// Карта вставлена.
    static const char *InsertedSignal; // = SIGNAL(inserted(ECardType::Enum, const QVariantMap &));

    /// Карта извлечена.
    static const char *EjectedSignal; // = SIGNAL(ejected());

    /// Проверка доступности устройства и карты.
    virtual bool isDeviceReady() const = 0;

    /// Выбросить карту (для моторизированных ридеров) или отключить электрически (для
    /// немоторизованных).
    virtual void eject() = 0;

protected:
    virtual ~ICardReader() {}
};

} // namespace Driver
} // namespace SDK

Q_DECLARE_METATYPE(SDK::Driver::ECardType::Enum);

//--------------------------------------------------------------------------------
