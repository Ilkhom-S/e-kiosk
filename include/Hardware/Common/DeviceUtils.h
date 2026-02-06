/* @file Утилиты для логики устройств. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

#include "Hardware/Common/FunctionTypes.h"

//---------------------------------------------------------------------------
/// Данные устройств.
typedef QMap<QString, QString> TDeviceData;

//---------------------------------------------------------------------------
namespace DeviceUtils {
/// Проверить сложную (вида x.y.z.0...) прошивку на актуальность.
bool isComplexFirmwareOld(const QString &aFirmware, const QString &aActualFirmware);

/// Получение заданной компоненты параметров устройства.
QString getPartDeviceData(const TDeviceData &aData, bool aHideEmpty = true, int aLevel = 0);
} // namespace DeviceUtils

//--------------------------------------------------------------------------------
