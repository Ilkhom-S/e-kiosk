/* @file Базовый класс устройств на LibUSB-порту. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QVariant>

// Совместимость с Qt 6 для рекурсивного мьютекса
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static QRecursiveMutex mUsageDataGuard;
#else
static QMutex mUsageDataGuard;
#endif
#include <Common/QtHeadersEnd.h>

// Project
#include <Hardware/Common/BaseStatusTypes.h>
#include <Hardware/Common/BaseStatus.h>
#include <Hardware/Common/USBDeviceModelData.h>
#include <Hardware/IOPorts/LibUSBPort.h>
#include <Hardware/IOPorts/LibUSBUtils.h>
#include <Hardware/Common/DeviceBase.h>
#include <Hardware/IOPorts/IOPortStatusCodes.h>

// System
#include <iterator>
#include <algorithm>

template <class T> class LibUSBDeviceBase : public T
{
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
    static TUsageData mUsageData;

    // Кроссплатформенное объявление рекурсивного мьютекса
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static QRecursiveMutex mUsageDataGuard;
#else
    static QMutex mUsageDataGuard;
#endif

    LibUSBPort mLibUSBPort;
    CUSBDevice::PDetectingData mDetectingData;
    int mDetectingPosition = -1; // Позиция итератора при поиске
};

//--------------------------------------------------------------------------------
// РЕАЛИЗАЦИЯ (Методы шаблонов должны быть в заголовочном файле)
//--------------------------------------------------------------------------------

template <class T> typename LibUSBDeviceBase<T>::TUsageData LibUSBDeviceBase<T>::mUsageData;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Для Qt 6 используется кроссплатформенный QRecursiveMutex
template <class T> QRecursiveMutex LibUSBDeviceBase<T>::mUsageDataGuard;
#else
// Для Qt 5 инициализируем мьютекс в рекурсивном режиме
template <class T> QMutex LibUSBDeviceBase<T>::mUsageDataGuard(QMutex::Recursive);
#endif

template <class T> LibUSBDeviceBase<T>::LibUSBDeviceBase()
{
    // this->mIOPort необходим в шаблонах для доступа к членам базового класса на Linux/Mac
    this->mIOPort = &mLibUSBPort;
    mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

template <class T> LibUSBDeviceBase<T>::~LibUSBDeviceBase()
{
    resetUsageData();
}

template <class T> bool LibUSBDeviceBase<T>::release()
{
    bool result = T::release();
    resetUsageData();
    return result;
}

template <class T> void LibUSBDeviceBase<T>::initialize()
{
    // Используем оператор & и полное имя класса для корректного получения адреса метода в шаблоне
    START_IN_WORKING_THREAD(&LibUSBDeviceBase<T>::initialize);
    initializeUSBPort();
    setFreeUsageData();
    T::initialize();
}

template <class T> bool LibUSBDeviceBase<T>::setFreeUsageData()
{
    QMutexLocker lock(&mUsageDataGuard);

    // Поиск первого свободного устройства через итераторы (совместимо с C++14)
    for (auto it = mUsageData.begin(); it != mUsageData.end(); ++it)
    {
        if (it.value())
        { // Если устройство свободно
            return setUsageData(it.key());
        }
    }
    return false;
}

template <class T> void LibUSBDeviceBase<T>::resetUsageData()
{
    QMutexLocker lock(&mUsageDataGuard);
    libusb_device *device = mLibUSBPort.getDevice();
    if (mUsageData.contains(device))
    {
        mUsageData[device] = true;
    }
}

template <class T> bool LibUSBDeviceBase<T>::setUsageData(libusb_device *aDevice)
{
    // .value() гарантирует безопасность при отсутствии ключа (не создает пустой элемент)
    CLibUSB::SDeviceProperties properties = mLibUSBPort.getDevicesProperties(false).value(aDevice);
    QString logVID = properties.deviceData.value(CHardwareUSB::VID).toString();
    QString logPID = properties.deviceData.value(CHardwareUSB::PID).toString();

    if (!mDetectingData->data().contains(properties.VID))
    {
        this->toLog(LogLevel::Normal, QStringLiteral("%1: Нет данных для VID %2").arg(this->mDeviceName, logVID));
        return false;
    }

    const auto &PIDData = mDetectingData->value(properties.VID).constData();
    if (!PIDData.contains(properties.PID))
    {
        this->toLog(LogLevel::Normal,
                    QStringLiteral("%1: Нет PID %2 для VID %3").arg(this->mDeviceName, logPID, logVID));
        return false;
    }

    const auto &productData = PIDData[properties.PID];
    this->mDeviceName = productData.model;
    this->mVerified = productData.verified;

    mUsageData[aDevice] = false;
    mLibUSBPort.setDevice(aDevice);
    return true;
}

template <class T> void LibUSBDeviceBase<T>::initializeUSBPort()
{
    mLibUSBPort.initialize();
    CLibUSB::TDeviceProperties devicesProperties = mLibUSBPort.getDevicesProperties(true);

    QMutexLocker lock(&mUsageDataGuard);

    // Безопасный способ получения ключей QMap для Qt 6
    QSet<libusb_device *> currentKeys(mUsageData.keyBegin(), mUsageData.keyEnd());
    QSet<libusb_device *> availableKeys(devicesProperties.keyBegin(), devicesProperties.keyEnd());
    QSet<libusb_device *> deletedDevices = currentKeys - availableKeys;

    for (libusb_device *device : deletedDevices)
    {
        mUsageData.remove(device);
    }

    for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
    {
        for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
        {
            for (auto kt = jt.value().constData().begin(); kt != jt.value().constData().end(); ++kt)
            {
                if (!mUsageData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
                {
                    mUsageData.insert(it.key(), true);
                }
            }
        }
    }
}

template <class T> bool LibUSBDeviceBase<T>::checkConnectionAbility()
{
    return this->checkError(
        IOPortStatusCode::Error::Busy, [this]() { return this->mIOPort->open(); }, "device cannot open port");
}

template <class T> bool LibUSBDeviceBase<T>::checkPort()
{
    if (this->mIOPort->isExist())
        return true;
    if (!this->mIOPort->deviceConnected())
        return false;

    initializeUSBPort();
    return setFreeUsageData() && this->checkExistence();
}

template <class T>
void LibUSBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                            const TStatusCollection &aOldStatusCollection)
{
    if (this->mIOPort && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
    {
        this->mIOPort->close();
    }
    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

template <class T> SDK::Driver::IDevice::IDetectingIterator *LibUSBDeviceBase<T>::getDetectingIterator()
{
    initializeUSBPort();
    mDetectingPosition = -1;
    return !mUsageData.isEmpty() ? this : nullptr;
}

template <class T> bool LibUSBDeviceBase<T>::find()
{
    if (mDetectingPosition < 0 || mDetectingPosition >= mUsageData.size())
        return false;

    QMutexLocker lock(&mUsageDataGuard);
    auto it = mUsageData.begin();
    std::advance(it, mDetectingPosition);

    if (it == mUsageData.end() || !setUsageData(it.key()))
        return false;

    return this->checkExistence();
}

template <class T> bool LibUSBDeviceBase<T>::moveNext()
{
    mDetectingPosition++;
    return (mDetectingPosition >= 0) && (mDetectingPosition < mUsageData.size());
}
