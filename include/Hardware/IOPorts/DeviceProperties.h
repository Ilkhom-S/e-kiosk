/* @file DeviceProperties.h — Универсальные структуры данных устройств. */
#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

/** @struct SVPID
 *  @brief Идентификаторы USB (VID/PID). Полностью независимы от ОС.
 */
struct SVPID {
    quint16 VID;
    quint16 PID;

    SVPID() : VID(0), PID(0) {}
    SVPID(quint16 aVID, quint16 aPID) : VID(aVID), PID(aPID) {}

    // В C++14/17 рекомендуется добавить const для безопасности
    bool isValid() const { return VID != 0 && PID != 0; }
};

// Переименовано в TDeviceProperties для кроссплатформенности
typedef QMap<QString, QString> TDeviceProperties;

/** @struct SDeviceProperties
 *  @brief Кроссплатформенная структура свойств устройства.
 */
struct SDeviceProperties : public SVPID {
    QString path;           // Системный путь (напр., COM1 или /dev/ttyUSB0)
    TDeviceProperties data; // Карта произвольных свойств
};

// Псевдонимы для обратной совместимости (чтобы не менять весь код сразу)
typedef SDeviceProperties SWinDeviceProperties;
typedef QMap<QString, SDeviceProperties> TDevicePropertiesMap;
typedef TDevicePropertiesMap TWinDeviceProperties;
typedef QMap<QString, QStringList> TSourceDeviceData;
