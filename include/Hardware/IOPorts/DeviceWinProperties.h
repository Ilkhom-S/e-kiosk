/* @file Описатель универсальных свойств устройств. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CDeviceProperties {
// Вместо "SPDRP" используем общий префикс
extern const char Prefix[];

// Определяем кроссплатформенные идентификаторы свойств
enum EType : unsigned int {
    FriendlyName = 0,
    DeviceDesc,
    Manufacturer,
    Class,
    Driver,
    Enumerator,
    Location,
    BusAddress,
    PhysName
};
} // namespace CDeviceProperties

//--------------------------------------------------------------------------------
/// Описатель свойств устройств. Работает на всех ОС без windows.h
class DeviceProperties : public CDescription<unsigned int> {
public:
    DeviceProperties() {
        // Вместо APPEND(SPDRP_...) используем свои внутренние ID
        append(CDeviceProperties::FriendlyName, "FriendlyName");
        append(CDeviceProperties::DeviceDesc, "DeviceDesc");
        append(CDeviceProperties::Manufacturer, "Manufacturer");
        append(CDeviceProperties::Class, "Class");
        append(CDeviceProperties::Driver, "Driver");
        append(CDeviceProperties::Enumerator, "Enumerator");
        append(CDeviceProperties::Location, "Location");
        append(CDeviceProperties::BusAddress, "BusAddress");
        append(CDeviceProperties::PhysName, "PhysName");
    }
};
