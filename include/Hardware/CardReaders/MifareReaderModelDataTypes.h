/* @file Типы данных моделей Mifare-ридеров ACS. */
#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//------------------------------------------------------------------------------
namespace CMifareReader {
struct SModelData : public CUSBDevice::SProductData {
    int sam;
    bool ccid;

    SModelData() : sam(0), ccid(true) {}
    SModelData(const QString &aModel, int aSAM, bool aCCID, bool aVerified)
        : CUSBDevice::SProductData(aModel, aVerified), sam(aSAM), ccid(aCCID) {}
};
} // namespace CMifareReader

//------------------------------------------------------------------------------
