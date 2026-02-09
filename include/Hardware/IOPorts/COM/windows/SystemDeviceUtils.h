/* @file Набор функционала для работы с системными ресурсами с использованием SetupDi. */

#pragma once

// Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <QtCore/QUuid>

#include <Hardware/IOPorts/DeviceProperties.h>
#include <setupapi.h>
#include <windows.h>

#include "Hardware/Common/ASCII.h"
#include "Hardware/IOPorts/DeviceWinProperties.h"

/// Работа с реестром.
namespace CRegistrySerialPort {
/// Начальный путь для поиска
extern const char InitialPath[];

/// Начальный путь для быстрого поиска
const wchar_t QuickInitialPath[] = L"HARDWARE\\DEVICEMAP\\SERIALCOMM";

/// Свойство для формирования пути для открытия порта.
extern const char PathProperty[];

/// Зарезервированные символы QRegExp.
const char RegexSymbols[] = {'+', '-', '*', '?', ')', '(', '{', '}', '^', '.'};

/// Размер Uuid-а в строковом представлении.
const int UuidSize = QUuid().toString().size();

/// Размер линии ключа гуида.
const int LineSize = 21;

/// Размер ключа Uuid-а.
const int MaxUuidSize = UuidSize + 2 * (LineSize + 1);
} // namespace CRegistrySerialPort

//--------------------------------------------------------------------------------
namespace System_DeviceUtils {
/// Формирует список COM-портов.
QStringList enumerateCOMPorts();

/// Формирует список последовательных портов на основе реестра.
QStringList getSerialDeviceNames();

/// Формирует список пропертей для устройств по GUID-у.
bool enumerateSystem_Devices(const QUuid &aUuid,
                             TWinDeviceProperties &aDeviceProperties,
                             DWORD aPathProperty,
                             bool aQuick = false);

/// Формирует список пропертей для устройств поиском по реестру.
TWinDeviceProperties enumerateRegistryDevices(bool aQuick = false);

/// Вмерживает недостающие проперти девайсов, полученные из реестра, в проперти, полученные по
/// гуидам.
void mergeRegistryDeviceProperties(TWinDeviceProperties &aDeviceProperties,
                                   const TWinDeviceProperties &aDeviceRegistryProperties,
                                   TSourceDeviceData &aSourceDeviceData);

/// Получает имя порта из группы имен портов, соответствующее имени заданного порта.
QString getRelevantPortName(const QString &aPortName, const QStringList &aPortNames);

/// Получает свойство порта из группы свойств порта, соответствующее заданному свойству порта.
QString getRelevantWinProperty(const QString &aWinProperty, const QStringList &aWinProperties);

/// Получает значение свойства реестра по хэндлу ключа и id свойства.
QString getRegKeyValue(HKEY key, LPCTSTR aProperty);

/// Возвращает свойство системного устройства.
QString getProperty(const HDEVINFO &hDevInfo, SP_DEVINFO_DATA &aDeviceInfoData, DWORD aProperty);

/// Получить выходные ключи, по которым были получены данные по устройству.
QString getDeviceOutKey(const QStringList &aKeys);

/// Получить выходные данные по устройству.
QString getDeviceOutData(const TWinProperties &aWinPropertyData);

/// Получить максимальный размер данных в контейнере.
int getMaxSize(const QStringList &aBuffer);

/// Получить данные для регэкспа с экранированными символами.
QString getScreenedData(const QString &aData);
} // namespace System_DeviceUtils

bool operator!=(const COMMTIMEOUTS &aTimeouts_1, const COMMTIMEOUTS &aTimeouts_2);
bool operator==(const DCB &aMine, const DCB &aTheirs);

//--------------------------------------------------------------------------------
