/* @file Базовый класс устройств на LibUSB-порту. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Совместимость с Qt 6 для рекурсивного мьютекса
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static QRecursiveMutex m_UsageDataGuard;
#else
static QMutex m_UsageDataGuard;
#endif

#include <Hardware/Common/BaseStatus.h>
#include <Hardware/Common/BaseStatusTypes.h>
#include <Hardware/Common/DeviceBase.h>
#include <Hardware/Common/USBDeviceModelData.h>
#include <Hardware/IOPorts/IOPortStatusCodes.h>
#include <Hardware/IOPorts/LibUSBPort.h>
#include <Hardware/IOPorts/LibUSBUtils.h>
#include <algorithm>
#include <iterator>

template <class T> class LibUSBDeviceBase : public T {
    SET_INTERACTION_TYPE(LibUSB)

public:
    LibUSBDeviceBase();
    virtual ~LibUSBDeviceBase();

    // Интерфейс SDK::Driver::IDevice
    virtual void initialize() override;
    virtual bool release() override;
    virtual SDK::Driver::IDevice::IDetectingIterator *getDetectingIterator() override;

    // Интерфейс SDK::Driver::IDetectingIterator
    virtual bool moveNext() override;
    virtual bool find() override;

protected:
    virtual bool checkConnectionAbility() override;
    void initializeUSBPort();
    bool setUsageData(libusb_device *aDevice);
    bool setFreeUsageData();
    void resetUsageData();
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection) override;
    virtual bool checkPort() override;

    typedef QMap<libusb_device *, bool> TUsageData;
    static TUsageData m_UsageData;

    // Кроссплатформенное объявление рекурсивного мьютекса
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRecursiveMutex m_UsageDataGuard;
#else
    static QMutex m_UsageDataGuard;
#endif

    LibUSBPort m_LibUSBPort;
    CUSBDevice::PDetectingData m_DetectingData;
    int m_DetectingPosition = -1; // Позиция итератора при поиске
};

//--------------------------------------------------------------------------------
// РЕАЛИЗАЦИЯ (Методы шаблонов должны быть в заголовочном файле)
//--------------------------------------------------------------------------------

template <class T> typename LibUSBDeviceBase<T>::TUsageData LibUSBDeviceBase<T>::m_UsageData;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Для Qt 6 используется кроссплатформенный QRecursiveMutex
template <class T> QRecursiveMutex LibUSBDeviceBase<T>::m_UsageDataGuard;
#else
// Для Qt 5 инициализируем мьютекс в рекурсивном режиме
template <class T> QMutex LibUSBDeviceBase<T>::m_UsageDataGuard(QMutex::Recursive);
#endif

template <class T> LibUSBDeviceBase<T>::LibUSBDeviceBase() {
    // this->m_IOPort необходим в шаблонах для доступа к членам базового класса на Linux/Mac
    this->m_IOPort = &m_LibUSBPort;
    m_DetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->m_ReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

template <class T> LibUSBDeviceBase<T>::~LibUSBDeviceBase() {
    resetUsageData();
}

template <class T> bool LibUSBDeviceBase<T>::release() {
    bool result = T::release();
    resetUsageData();
    return result;
}

template <class T> void LibUSBDeviceBase<T>::initialize() {
    // Используем оператор & и полное имя класса для корректного получения адреса метода в шаблоне
    START_IN_WORKING_THREAD(&LibUSBDeviceBase<T>::initialize);
    initializeUSBPort();
    setFreeUsageData();
    T::initialize();
}

template <class T> bool LibUSBDeviceBase<T>::setFreeUsageData() {
    QMutexLocker lock(&m_UsageDataGuard);

    // Поиск первого свободного устройства через итераторы (совместимо с C++14)
    for (auto it = m_UsageData.begin(); it != m_UsageData.end(); ++it) {
        if (it.value()) { // Если устройство свободно
            return setUsageData(it.key());
        }
    }
    return false;
}

template <class T> void LibUSBDeviceBase<T>::resetUsageData() {
    QMutexLocker lock(&m_UsageDataGuard);
    libusb_device *device = m_LibUSBPort.getDevice();
    if (m_UsageData.contains(device)) {
        m_UsageData[device] = true;
    }
}

template <class T> bool LibUSBDeviceBase<T>::setUsageData(libusb_device *aDevice) {
    // .value() гарантирует безопасность при отсутствии ключа (не создает пустой элемент)
    CLibUSB::SDeviceProperties properties = m_LibUSBPort.getDevicesProperties(false).value(aDevice);
    QString logVID = properties.deviceData.value(CHardwareUSB::VID).toString();
    QString logPID = properties.deviceData.value(CHardwareUSB::PID).toString();

    if (!m_DetectingData->data().contains(properties.VID)) {
        this->toLog(LogLevel::Normal,
                    QStringLiteral("%1: Нет данных для VID %2").arg(this->m_DeviceName, logVID));
        return false;
    }

    const auto &PIDData = m_DetectingData->value(properties.VID).constData();
    if (!PIDData.contains(properties.PID)) {
        this->toLog(
            LogLevel::Normal,
            QStringLiteral("%1: Нет PID %2 для VID %3").arg(this->m_DeviceName, logPID, logVID));
        return false;
    }

    const auto &productData = PIDData[properties.PID];
    this->m_DeviceName = productData.model;
    this->m_Verified = productData.verified;

    m_UsageData[aDevice] = false;
    m_LibUSBPort.setDevice(aDevice);
    return true;
}

template <class T> void LibUSBDeviceBase<T>::initializeUSBPort() {
    m_LibUSBPort.initialize();
    CLibUSB::TDeviceProperties devicesProperties = m_LibUSBPort.getDevicesProperties(true);

    QMutexLocker lock(&m_UsageDataGuard);

    // Безопасный способ получения ключей QMap для Qt 6
    QSet<libusb_device *> currentKeys(m_UsageData.keyBegin(), m_UsageData.keyEnd());
    QSet<libusb_device *> availableKeys(devicesProperties.keyBegin(), devicesProperties.keyEnd());
    QSet<libusb_device *> deletedDevices = currentKeys - availableKeys;

    for (libusb_device *device : deletedDevices) {
        m_UsageData.remove(device);
    }

    for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it) {
        for (auto jt = m_DetectingData->data().begin(); jt != m_DetectingData->data().end(); ++jt) {
            for (auto kt = jt.value().constData().begin(); kt != jt.value().constData().end();
                 ++kt) {
                if (!m_UsageData.contains(it.key()) && (it->VID == jt.key()) &&
                    (it->PID == kt.key())) {
                    m_UsageData.insert(it.key(), true);
                }
            }
        }
    }
}

template <class T> bool LibUSBDeviceBase<T>::checkConnectionAbility() {
    return this->checkError(
        IOPortStatusCode::Error::Busy,
        [this]() { return this->m_IOPort->open(); },
        "device cannot open port");
}

template <class T> bool LibUSBDeviceBase<T>::checkPort() {
    if (this->m_IOPort->isExist())
        return true;
    if (!this->m_IOPort->deviceConnected())
        return false;

    initializeUSBPort();
    return setFreeUsageData() && this->checkExistence();
}

template <class T>
void LibUSBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                            const TStatusCollection &aOldStatusCollection) {
    if (this->m_IOPort && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable)) {
        this->m_IOPort->close();
    }
    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

template <class T>
SDK::Driver::IDevice::IDetectingIterator *LibUSBDeviceBase<T>::getDetectingIterator() {
    initializeUSBPort();
    m_DetectingPosition = -1;
    return !m_UsageData.isEmpty() ? this : nullptr;
}

template <class T> bool LibUSBDeviceBase<T>::find() {
    if (m_DetectingPosition < 0 || m_DetectingPosition >= m_UsageData.size())
        return false;

    QMutexLocker lock(&m_UsageDataGuard);
    auto it = m_UsageData.begin();
    std::advance(it, m_DetectingPosition);

    if (it == m_UsageData.end() || !setUsageData(it.key()))
        return false;

    return this->checkExistence();
}

template <class T> bool LibUSBDeviceBase<T>::moveNext() {
    m_DetectingPosition++;
    return (m_DetectingPosition >= 0) && (m_DetectingPosition < m_UsageData.size());
}
