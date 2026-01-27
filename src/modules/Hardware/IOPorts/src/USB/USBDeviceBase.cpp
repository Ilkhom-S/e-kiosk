/* @file Базовый класс устройств на USB-порту. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QReadLocker>
#include <QtCore/QSet>
#include <QtCore/QWriteLocker>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include <Hardware/Common/USBDeviceBase.h>
#include "Hardware/HID/ProtoHID.h"

#ifdef Q_OS_WIN32
#include "Hardware/IOPorts/COM/windows/SystemDeviceUtils.h"
#endif

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
// Макрос для инстанцирования USB устройств с учетом совместимости Qt5/Qt6
// Объединяем мьютексы: в Qt6 QMutex::Recursive удален, используем QRecursiveMutex
// Qt5: QMutex::Recursive - рекурсивный мьютекс для потокобезопасности
// Qt6: QRecursiveMutex - аналогичная функциональность после удаления QMutex::Recursive
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define INSTANCE_USB_DEVICE(aClass)                                                                                    \
    template class aClass;                                                                                             \
    aClass::TPDOData aClass::mPDOData;                                                                                 \
    QRecursiveMutex aClass::mPDODataGuard;
#else
#define INSTANCE_USB_DEVICE(aClass)                                                                                    \
    template class aClass;                                                                                             \
    aClass::TPDOData aClass::mPDOData;                                                                                 \
    QMutex aClass::mPDODataGuard(QMutex::Recursive);
#endif

INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>>)
INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoHID>>)

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::USBDeviceBase() : mPDODetecting(false), mPortUsing(true) {
    this->mIOPort = &this->mUSBPort;

    this->mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::~USBDeviceBase() {
    this->resetPDOName();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::release() {
    bool result = T::release();
    this->resetPDOName();

    return result;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::initialize() {
    // В шаблонах передаем полный адрес метода
    START_IN_WORKING_THREAD(&USBDeviceBase<T>::initialize)

    this->initializeUSBPort();
    this->setFreePDOName();

    T::initialize();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::setFreePDOName() {
    MutexLocker lock(&this->mPDODataGuard);

    auto it = std::find_if(mPDOData.begin(), mPDOData.end(), [](bool aFree) { return aFree; });

    if (it == mPDOData.end()) {
        return false;
    }

    return this->setPDOName(it.key());
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::resetPDOName() {
    MutexLocker lock(&this->mPDODataGuard);

    QString PDOName = this->mIOPort->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();

    if (mPDOData.contains(PDOName)) {
        mPDOData[PDOName] = true;
    }
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::setPDOName(const QString &aPDOName) {
    auto properties = mUSBPort.getDevicesProperties(false, true)[aPDOName];
    QString logVID = ProtocolUtils::toHexLog(properties.VID);
    QString logPID = ProtocolUtils::toHexLog(properties.PID);

    if (!this->mDetectingData->data().contains(properties.VID)) {
        this->toLog(
            LogLevel::Normal,
            QStringLiteral("%1: Failed to set PDO name due to no such VID %2").arg(this->mDeviceName).arg(logVID));
        return false;
    }

    auto &PIDData = this->mDetectingData->value(properties.VID).data();

    if (!PIDData.contains(properties.PID)) {
        this->toLog(LogLevel::Normal, QStringLiteral("%1: Failed to set PDO name due to no such PID %2 for VID %3")
                                          .arg(this->mDeviceName)
                                          .arg(logPID)
                                          .arg(logVID));
        return false;
    }

    CUSBDevice::SProductData data = PIDData[properties.PID];
    this->mDeviceName = data.model;
    this->mVerified = data.verified;

    mPDOData[aPDOName] = false;

    QVariantMap configuration;
    configuration.insert(CHardwareSDK::SystemName, aPDOName);
    this->toLog(LogLevel::Normal, QStringLiteral("%1: Set USB PDO name %2, VID %3, PID %4")
                                      .arg(this->mDeviceName)
                                      .arg(aPDOName)
                                      .arg(logVID)
                                      .arg(logPID));

    this->mIOPort->setDeviceConfiguration(configuration);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::initializeUSBPort() {
    this->initialize();
    auto devicesProperties = mUSBPort.getDevicesProperties(true, this->mPDODetecting);

    MutexLocker lock(&this->mPDODataGuard);

    // Qt 6: замена .toSet() и оператора '-'
    auto mDataKeys = mPDOData.keys();
    QSet<QString> deletedPDONames(mDataKeys.begin(), mDataKeys.end());

    auto devKeys = devicesProperties.keys();
    QSet<QString> existingNames(devKeys.begin(), devKeys.end());

    deletedPDONames.subtract(existingNames);

    for (const QString &aPDOName : deletedPDONames) {
        mPDOData.remove(aPDOName);
    }

    for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it) {
        auto &detectData = this->mDetectingData->data();
        for (auto jt = detectData.begin(); jt != detectData.end(); ++jt) {
            auto &productData = jt.value().data();
            for (auto kt = productData.begin(); kt != productData.end(); ++kt) {
                if (!mPDOData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key())) {
                    mPDOData.insert(it.key(), true);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkConnectionAbility() {
    return !mPortUsing ||
           this->checkError(
               IOPortStatusCode::Error::Busy, [&]() { return this->mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkPort() {
    if (this->mIOPort->isExist()) {
        return true;
    } else if (!this->mIOPort->deviceConnected()) {
        return false;
    }

    this->initializeUSBPort();

    return this->setFreePDOName() && this->checkExistence();
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                         const TStatusCollection &aOldStatusCollection) {
    if (this->mIOPort && this->mPortUsing && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable)) {
        this->mIOPort->close();
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *USBDeviceBase<T>::getDetectingIterator() {
    this->initializeUSBPort();

    this->mDetectingPosition = -1;

    return mPDOData.size() > 0 ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::find() {
    if ((this->mDetectingPosition < 0) || (this->mDetectingPosition >= mPDOData.size())) {
        return false;
    }

    {
        MutexLocker lock(&this->mPDODataGuard);

        auto it = mPDOData.begin() + this->mDetectingPosition;

        if (!this->setPDOName(it.key())) {
            return false;
        }
    }

    return this->checkExistence();
}

//------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::moveNext() {
    this->mDetectingPosition++;

    return (this->mDetectingPosition >= 0) && (this->mDetectingPosition < mPDOData.size());
}
