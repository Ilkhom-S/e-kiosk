/* @file Базовый класс устройств на LibUSB-порту. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QtGlobal>

// В Qt 6 рекурсивный мьютекс выделен в отдельный класс QRecursiveMutex
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QRecursiveMutex>
#endif

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/IOPorts/LibUSBPort.h"

//--------------------------------------------------------------------------------
template <class T> class LibUSBDeviceBase : public T {
    // Макрос должен быть совместим с иерархией шаблонов.
    // Если он использует мета-данные, класс должен быть известен moc-компилятору.
    SET_INTERACTION_TYPE(LibUSB)

public:
    LibUSBDeviceBase();
    virtual ~LibUSBDeviceBase();

#pragma region SDK::Driver::IDevice interface
    /// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
    virtual void initialize() override;

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
    virtual bool release() override;

    /// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из
    /// этого списка.
    virtual SDK::Driver::IDevice::IDetectingIterator *getDetectingIterator() override;
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
    /// Переход к следующим параметрам устройства.
    virtual bool moveNext() override;

    /// Поиск устройства на текущих параметрах.
    virtual bool find() override;
#pragma endregion

protected:
    /// Проверка возможности выполнения функционала, предполагающего связь с устройством.
    virtual bool checkConnectionAbility() override;

    /// Инициализация USB порта.
    void initializeUSBPort();

    /// Установить данные соединения для работы порта.
    bool setUsageData(libusb_device *aDevice);

    /// Установить любое свободное соединение для работы порта.
    bool setFreeUsageData();

    /// Сбросить доступность данных соединений.
    void resetUsageData();

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection) override;

    /// Проверить порт.
    virtual bool checkPort() override;

    /// Данные соединений устройств. libusb_device — это указатель,
    /// поэтому QMap работает с ним корректно как с ключом.
    typedef QMap<libusb_device *, bool> TUsageData;
    static TUsageData m_UsageData;

    // В Qt 6 используем QRecursiveMutex, в Qt 5 — QMutex с флагом Recursive.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRecursiveMutex m_UsageDataGuard;
#else
    static QMutex m_UsageDataGuard;
#endif

    /// Порт.
    LibUSBPort m_LibUSBPort;

    /// Данные устройств для авто поиска.
    CUSBDevice::PDetectingData m_DetectingData;

    /// Позиция итератора при поиске.
    int m_DetectingPosition = -1;
};
