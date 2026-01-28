/* @file Определение статических членов LibUSBDeviceBase. */

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"

// Project
#include "LibUSBDeviceBase.h"

template <class T> typename LibUSBDeviceBase<T>::TUsageData LibUSBDeviceBase<T>::mUsageData;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Для Qt 6 используется кроссплатформенный QRecursiveMutex
template <class T> QRecursiveMutex LibUSBDeviceBase<T>::mUsageDataGuard;
#else
// Для Qt 5 инициализируем мьютекс в рекурсивном режиме
template <class T> QMutex LibUSBDeviceBase<T>::mUsageDataGuard(QMutex::Recursive);
#endif

// Явное инстанцирование для необходимых типов
template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
