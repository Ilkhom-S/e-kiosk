/* @file Базовый класс устройств на USB-порту. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QMutex>
// В Qt 6 QRecursiveMutex выделен в отдельный класс, в Qt 5 его нет
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QRecursiveMutex>
#endif
#include <Common/QtHeadersEnd.h>

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/Common/MetaDevice.h"
#include "Hardware/IOPorts/USBPort.h"

//--------------------------------------------------------------------------------
template <class T> class USBDeviceBase : public T
{
    SET_INTERACTION_TYPE(USB)

  public:
    USBDeviceBase();
    virtual ~USBDeviceBase();

#pragma region SDK::Driver::IDevice interface
    /// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
    virtual void initialize() override;

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
    virtual bool release() override;

    /// Переформировывает список параметров для авто поиска и устанавливает 1-й набор параметров из этого списка.
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

    /// Установить PDO-имя для работы порта.
    bool setPDOName(const QString &aPDOName);

    /// Установить любое свободное PDO-имя для работы порта.
    bool setFreePDOName();

    /// Сбросить доступность PDO-имени.
    void resetPDOName();

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection) override;

    /// Проверить порт.
    virtual bool checkPort() override;

    /// PDO-имена USB устройств.
    typedef QMap<QString, bool> TPDOData;
    static TPDOData mPDOData;

    // Кроссплатформенное объявление рекурсивного мьютекса для 2026 года
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRecursiveMutex mPDODataGuard;
#else
    static QMutex mPDODataGuard;
#endif

    /// Порт.
    USBPort mUSBPort;

    /// Данные устройств для авто поиска.
    CUSBDevice::PDetectingData mDetectingData;

    /// Учитывать при авто поиске PDO-имена.
    bool mPDODetecting;

    /// Порт используется.
    bool mPortUsing;

    /// Текущая позиция при авто поиске.
    int mDetectingPosition = -1;
};
