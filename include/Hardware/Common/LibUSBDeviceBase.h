/* @file Базовый класс устройств на LibUSB-порту. */

#pragma once

// Project
#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/IOPorts/LibUSBPort.h"

// System
#include <iterator>

//--------------------------------------------------------------------------------
template <class T> class LibUSBDeviceBase : public T
{
    SET_INTERACTION_TYPE(LibUSB)

  public:
    LibUSBDeviceBase();
    virtual ~LibUSBDeviceBase();

#pragma region SDK::Driver::IDevice interface
    /// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
    virtual void initialize();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
    virtual bool release();

    /// Переформировывает список параметров для авто поиска и устанавливает 1-й набор параметров из этого списка.
    virtual SDK::Driver::IDevice::IDetectingIterator *getDetectingIterator();
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
    /// Переход к следующим параметрам устройства.
    virtual bool moveNext();

    /// Поиск устройства на текущих параметрах.
    virtual bool find();
#pragma endregion

  protected:
    /// Проверка возможности выполнения функционала, предполагающего связь с устройством.
    virtual bool checkConnectionAbility();

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
                                   const TStatusCollection &aOldStatusCollection);

    /// Проверить порт.
    virtual bool checkPort();

    /// Данные соединений устройств.
    typedef QMap<libusb_device *, bool> TUsageData;
    static TUsageData mUsageData;
    static QRecursiveMutex mUsageDataGuard;

    /// Порт.
    LibUSBPort mLibUSBPort;

    /// Данные устройств для авто поиска.
    CUSBDevice::PDetectingData mDetectingData;
};

//---------------------------------------------------------------------------
// Template method implementations

// // Qt
// #include <Common/QtHeadersBegin.h>
// #include <QtCore/QReadLocker>
// #include <QtCore/QWriteLocker>
// #include <Common/QtHeadersEnd.h>

// // System
// #include "Hardware/Common/PortPollingDeviceBase.h"
// #include "Hardware/Common/ProtoDevices.h"
// #include <Hardware/IOPorts/LibUSBUtils.h>

// using namespace SDK::Driver;

// //--------------------------------------------------------------------------------
// template <class T> LibUSBDeviceBase<T>::LibUSBDeviceBase()
// {
//     this->mIOPort = &mLibUSBPort;

//     mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
//     this->mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
// }

// //--------------------------------------------------------------------------------
// template <class T> LibUSBDeviceBase<T>::~LibUSBDeviceBase()
// {
//     resetUsageData();
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::release()
// {
//     bool result = T::release();
//     resetUsageData();

//     return result;
// }

// //--------------------------------------------------------------------------------
// template <class T> void LibUSBDeviceBase<T>::initialize()
// {
//     START_IN_WORKING_THREAD(initialize)

//     initializeUSBPort();
//     setFreeUsageData();

//     T::initialize();
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::setFreeUsageData()
// {
//     MutexLocker lock(&mUsageDataGuard);

//     auto it = std::find_if(mUsageData.begin(), mUsageData.end(), [&](bool aFree) -> bool { return aFree; });

//     if (it == mUsageData.end())
//     {
//         return false;
//     }

//     return setUsageData(it.key());
// }

// //--------------------------------------------------------------------------------
// template <class T> void LibUSBDeviceBase<T>::resetUsageData()
// {
//     MutexLocker lock(&mUsageDataGuard);

//     libusb_device *device = mLibUSBPort.getDevice();

//     if (mUsageData.contains(device))
//     {
//         mUsageData[device] = true;
//     }
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::setUsageData(libusb_device *aDevice)
// {
//     CLibUSB::SDeviceProperties properties = mLibUSBPort.getDevicesProperties(false)[aDevice];
//     QString logVID = properties.deviceData[CHardwareUSB::VID].toString();
//     QString logPID = properties.deviceData[CHardwareUSB::PID].toString();

//     if (!mDetectingData->data().contains(properties.VID))
//     {
//         this->toLog(LogLevel::Normal,
//                     QString("%1: Failed to set usage data due to no such VID
//                     %2").arg(this->mDeviceName).arg(logVID));
//         return false;
//     }

//     const QMap<quint16, CUSBDevice::SProductData> &PIDData = mDetectingData->value(properties.VID).constData();

//     if (!PIDData.contains(properties.PID))
//     {
//         this->toLog(LogLevel::Normal, QString("%1: Failed to set usage data due to no such PID %2 for VID %3")
//                                           .arg(this->mDeviceName)
//                                           .arg(logPID)
//                                           .arg(logVID));
//         return false;
//     }

//     CUSBDevice::SProductData data = PIDData[properties.PID];
//     this->mDeviceName = data.model;
//     this->mVerified = data.verified;

//     this->toLog(LogLevel::Normal, QString("%1: Set usage data for device with VID %2 and PID %3, %4")
//                                       .arg(this->mDeviceName)
//                                       .arg(logVID)
//                                       .arg(logPID)
//                                       .arg(properties.portData));

//     mUsageData[aDevice] = false;
//     mLibUSBPort.setDevice(aDevice);

//     return true;
// }

// //--------------------------------------------------------------------------------
// template <class T> void LibUSBDeviceBase<T>::initializeUSBPort()
// {
//     mLibUSBPort.initialize();
//     CLibUSB::TDeviceProperties devicesProperties = mLibUSBPort.getDevicesProperties(true);

//     MutexLocker lock(&mUsageDataGuard);

//     QSet<libusb_device *> currentDevices(mUsageData.keys().begin(), mUsageData.keys().end());
//     QSet<libusb_device *> availableDevices(devicesProperties.keys().begin(), devicesProperties.keys().end());
//     QSet<libusb_device *> deletedDevices = currentDevices - availableDevices;

//     foreach (libusb_device *device, deletedDevices)
//     {
//         mUsageData.remove(device);
//     }

//     for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
//     {
//         for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
//         {
//             for (auto kt = jt.value().data().begin(); kt != jt.value().data().end(); ++kt)
//             {
//                 if (!mUsageData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
//                 {
//                     mUsageData.insert(it.key(), true);
//                 }
//             }
//         }
//     }
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::checkConnectionAbility()
// {
//     return this->checkError(
//         IOPortStatusCode::Error::Busy, [&]() -> bool { return this->mIOPort->open(); }, "device cannot open port");
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::checkPort()
// {
//     if (this->mIOPort->isExist())
//     {
//         return true;
//     }
//     else if (!this->mIOPort->deviceConnected())
//     {
//         return false;
//     }

//     initializeUSBPort();

//     return setFreeUsageData() && this->checkExistence();
// }

// //--------------------------------------------------------------------------------
// template <class T>
// void LibUSBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
//                                             const TStatusCollection &aOldStatusCollection)
// {
//     if (this->mIOPort && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
//     {
//         this->mIOPort->close();
//     }

//     T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
// }

// //------------------------------------------------------------------------------
// template <class T> IDevice::IDetectingIterator *LibUSBDeviceBase<T>::getDetectingIterator()
// {
//     initializeUSBPort();

//     this->mDetectingPosition = -1;

//     return mUsageData.size() > 0 ? this : nullptr;
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::moveNext()
// {
//     return T::moveNext();
// }

// //--------------------------------------------------------------------------------
// template <class T> bool LibUSBDeviceBase<T>::find()
// {
//     if ((this->mDetectingPosition < 0) || (this->mDetectingPosition >= mUsageData.size()))
//     {
//         return false;
//     }

//     {
//         MutexLocker lock(&mUsageDataGuard);

//         auto it = std::next(mUsageData.begin(), this->mDetectingPosition);

//         if (!setUsageData(it.key()))
//         {
//             return false;
//         }
//     }

//     return this->checkExistence();
// }

// //---------------------------------------------------------------------------

// template <class T> typename LibUSBDeviceBase<T>::TUsageData LibUSBDeviceBase<T>::mUsageData = TUsageData();

// template <class T> QRecursiveMutex LibUSBDeviceBase<T>::mUsageDataGuard;