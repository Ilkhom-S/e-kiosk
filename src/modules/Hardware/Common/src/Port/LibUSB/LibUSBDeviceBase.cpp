/* @file Базовый класс устройств на LibUSB-порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <Common/QtHeadersEnd.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QRecursiveMutex>
#endif
#include <Common/QtHeadersEnd.h>

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/IOPorts/LibUSBUtils.h"
#include "LibUSBDeviceBase.h"

using namespace SDK::Driver;

// Определение статических членов
template <class T> typename LibUSBDeviceBase<T>::TUsageData LibUSBDeviceBase<T>::mUsageData;

// Инициализация мьютекса с учетом версии Qt
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
template <class T> QRecursiveMutex LibUSBDeviceBase<T>::mUsageDataGuard;
#else
template <class T> QMutex LibUSBDeviceBase<T>::mUsageDataGuard(QMutex::Recursive);
#endif

//--------------------------------------------------------------------------------
template <class T> LibUSBDeviceBase<T>::LibUSBDeviceBase()
{
    // this->mIOPort необходим в шаблонах для доступа к членам базового класса
    this->mIOPort = &mLibUSBPort;

    mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T> LibUSBDeviceBase<T>::~LibUSBDeviceBase()
{
    resetUsageData();
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::release()
{
    bool result = T::release();
    resetUsageData();
    return result;
}

//--------------------------------------------------------------------------------
template <class T> void LibUSBDeviceBase<T>::initialize()
{
    // Макрос должен поддерживать актуальный контекст потока
    START_IN_WORKING_THREAD(initialize)

    initializeUSBPort();
    setFreeUsageData();

    T::initialize();
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::setFreeUsageData()
{
    QMutexLocker lock(&mUsageDataGuard);

    // std::find_if работает с итераторами QMap. В Qt 6 предпочтительнее явные циклы или итераторы.
    for (auto it = mUsageData.begin(); it != mUsageData.end(); ++it)
    {
        if (it.value())
        { // Если устройство свободно
            return setUsageData(it.key());
        }
    }

    return false;
}

//--------------------------------------------------------------------------------
template <class T> void LibUSBDeviceBase<T>::resetUsageData()
{
    QMutexLocker lock(&mUsageDataGuard);

    libusb_device *device = mLibUSBPort.getDevice();
    if (mUsageData.contains(device))
    {
        mUsageData[device] = true;
    }
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::setUsageData(libusb_device *aDevice)
{
    // Используем .value() для безопасного доступа к свойствам без изменения QMap
    CLibUSB::SDeviceProperties properties = mLibUSBPort.getDevicesProperties(false).value(aDevice);
    QString logVID = properties.deviceData.value(CHardwareUSB::VID).toString();
    QString logPID = properties.deviceData.value(CHardwareUSB::PID).toString();

    if (!mDetectingData->data().contains(properties.VID))
    {
        this->toLog(LogLevel::Normal, QStringLiteral("%1: No such VID %2").arg(this->mDeviceName, logVID));
        return false;
    }

    const auto &PIDData = mDetectingData->value(properties.VID).constData();
    if (!PIDData.contains(properties.PID))
    {
        this->toLog(LogLevel::Normal,
                    QStringLiteral("%1: No PID %2 for VID %3").arg(this->mDeviceName, logPID, logVID));
        return false;
    }

    const auto &data = PIDData[properties.PID];
    this->mDeviceName = data.model;
    this->mVerified = data.verified;

    this->toLog(LogLevel::Normal,
                QStringLiteral("%1: Set usage for VID %2 PID %3").arg(this->mDeviceName, logVID, logPID));

    mUsageData[aDevice] = false;
    mLibUSBPort.setDevice(aDevice);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void LibUSBDeviceBase<T>::initializeUSBPort()
{
    mLibUSBPort.initialize();
    CLibUSB::TDeviceProperties devicesProperties = mLibUSBPort.getDevicesProperties(true);

    QMutexLocker lock(&mUsageDataGuard);

    // В Qt 6 метод keys().toSet() неэффективен. Используем конструктор QSet с итераторами.
    QSet<libusb_device *> currentKeys(mUsageData.keyBegin(), mUsageData.keyEnd());
    QSet<libusb_device *> newKeys(devicesProperties.keyBegin(), devicesProperties.keyEnd());

    QSet<libusb_device *> deletedDevices = currentKeys - newKeys;

    for (libusb_device *device : deletedDevices)
    {
        mUsageData.remove(device);
    }

    // Тройной цикл поиска соответствия VID/PID
    for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
    {
        for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
        {
            for (auto kt = jt.value().data().begin(); kt != jt.value().data().end(); ++kt)
            {
                if (!mUsageData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
                {
                    mUsageData.insert(it.key(), true);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::checkConnectionAbility()
{
    return this->checkError(
        IOPortStatusCode::Error::Busy, [this]() { return this->mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::checkPort()
{
    if (this->mIOPort->isExist())
    {
        return true;
    }
    else if (!this->mIOPort->deviceConnected())
    {
        return false;
    }

    initializeUSBPort();
    return setFreeUsageData() && this->checkExistence();
}

//--------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *LibUSBDeviceBase<T>::getDetectingIterator()
{
    initializeUSBPort();
    mDetectingPosition = -1;
    return !mUsageData.isEmpty() ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::find()
{
    if (mDetectingPosition < 0 || mDetectingPosition >= mUsageData.size())
    {
        return false;
    }

    {
        QMutexLocker lock(&mUsageDataGuard);
        // Безопасный переход к элементу по индексу
        auto it = mUsageData.begin();
        std::advance(it, mDetectingPosition);

        if (it == mUsageData.end() || !setUsageData(it.key()))
        {
            return false;
        }
    }

    return this->checkExistence();
}

//------------------------------------------------------------------------------
template <class T> bool LibUSBDeviceBase<T>::moveNext()
{
    mDetectingPosition++;
    return (mDetectingPosition >= 0) && (mDetectingPosition < mUsageData.size());
}
