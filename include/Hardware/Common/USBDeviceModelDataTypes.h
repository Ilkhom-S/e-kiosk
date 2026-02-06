/* @file Типы данных моделей USB-устройств. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

#include "Hardware/Common/DeviceModelDataTypes.h"

//--------------------------------------------------------------------------------
namespace CUSBDevice {
typedef SModelDataBase SProductData;
typedef QMap<quint16, SProductData> TProductData;

//--------------------------------------------------------------------------------
struct SDetectingData {
    QString vendor;
    quint16 PID;
    QString model;

    SDetectingData() : PID(0) {}
    SDetectingData(const QString &aVendor, quint16 aPID, const QString &aModel)
        : vendor(aVendor), PID(aPID), model(aModel) {}

    QString getModel() const { return vendor + " " + model; }
};

/// Вызывается из статического функционала. Если будет константа - на момент вызова она не будет
/// создана.
#define DECLARE_USB_MODEL(aVar, aVendor, aPID, aName)                                              \
    struct aVar : public CUSBDevice::SDetectingData {                                              \
        aVar() : CUSBDevice::SDetectingData(CUSBVendors::aVendor, aPID, aName) {}                  \
    };
} // namespace CUSBDevice

//--------------------------------------------------------------------------------
