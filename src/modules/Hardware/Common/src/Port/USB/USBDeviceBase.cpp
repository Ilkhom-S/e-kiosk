/* @file Базовый класс устройств на USB-порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QReadLocker>
#include <QtCore/QSet>
#include <QtCore/QWriteLocker>
#include <Common/QtHeadersEnd.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QRecursiveMutex>
#endif
#include <Common/QtHeadersEnd.h>

#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"
#include "USBDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
// Макрос инициализации: адаптирован под QRecursiveMutex в Qt 6 и QMutex в Qt 5
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define INSTANCE_USB_DEVICE(aClass)                                                                                    \
    template class aClass;                                                                                             \
    template <> aClass::TPDOData aClass::mPDOData = aClass::TPDOData();                                                \
    template <> QRecursiveMutex aClass::mPDODataGuard = QRecursiveMutex();
#else
#define INSTANCE_USB_DEVICE(aClass)                                                                                    \
    template class aClass;                                                                                             \
    template <> aClass::TPDOData aClass::mPDOData = aClass::TPDOData();                                                \
    template <> QMutex aClass::mPDODataGuard(QMutex::Recursive);
#endif

INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>>)
INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoHID>>)

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::USBDeviceBase() : mPDODetecting(false), mPortUsing(true)
{
    // В шаблонах используем this-> для доступа к членам базового класса
    this->mIOPort = &mUSBPort;
    mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::~USBDeviceBase()
{
    resetPDOName();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::release()
{
    bool result = T::release();
    resetPDOName();
    return result;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::initialize()
{
    START_IN_WORKING_THREAD(initialize)
    initializeUSBPort();
    setFreePDOName();
    T::initialize();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::setFreePDOName()
{
    QMutexLocker lock(&mPDODataGuard);

    // Поиск первого свободного PDO-имени
    for (auto it = mPDOData.begin(); it != mPDOData.end(); ++it)
    {
        if (it.value())
        {
            return setPDOName(it.key());
        }
    }
    return false;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::resetPDOName()
{
    QMutexLocker lock(&mPDODataGuard);
    // Извлекаем системное имя из конфигурации порта
    QString PDOName = this->mIOPort->getDeviceConfiguration().value(CHardwareSDK::SystemName).toString();

    if (mPDOData.contains(PDOName))
    {
        mPDOData[PDOName] = true;
    }
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::setPDOName(const QString &aPDOName)
{
    // Безопасное получение свойств устройства через .value()
    SWinDeviceProperties properties = mUSBPort.getDevicesProperties(false, true).value(aPDOName);
    QString logVID = ProtocolUtils::toHexLog(properties.VID);
    QString logPID = ProtocolUtils::toHexLog(properties.PID);

    if (!mDetectingData->data().contains(properties.VID))
    {
        this->toLog(LogLevel::Normal, QStringLiteral("%1: No such VID %2").arg(this->mDeviceName, logVID));
        return false;
    }

    // Доступ через constData() для соответствия новому интерфейсу CSpecification
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

    mPDOData[aPDOName] = false;

    QVariantMap configuration;
    configuration.insert(CHardwareSDK::SystemName, aPDOName);
    this->toLog(LogLevel::Normal,
                QStringLiteral("%1: Set USB PDO %2 (VID %3 PID %4)").arg(this->mDeviceName, aPDOName, logVID, logPID));

    this->mIOPort->setDeviceConfiguration(configuration);
    return true;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::initializeUSBPort()
{
    mUSBPort.initialize();
    TWinDeviceProperties devicesProperties = mUSBPort.getDevicesProperties(true, mPDODetecting);

    QMutexLocker lock(&mPDODataGuard);

    // Замена keys().toSet() на конструктор QSet (совместимо с Qt 6)
    QSet<QString> currentKeys(mPDOData.keyBegin(), mPDOData.keyEnd());
    QSet<QString> newKeys(devicesProperties.keyBegin(), devicesProperties.keyEnd());
    QSet<QString> deletedPDONames = currentKeys - newKeys;

    for (const QString &aPDOName : deletedPDONames)
    {
        mPDOData.remove(aPDOName);
    }

    for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
    {
        for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
        {
            for (auto kt = jt.value().constData().begin(); kt != jt.value().constData().end(); ++kt)
            {
                if (!mPDOData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
                {
                    mPDOData.insert(it.key(), true);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkConnectionAbility()
{
    return !mPortUsing ||
           this->checkError(
               IOPortStatusCode::Error::Busy, [this]() { return this->mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkPort()
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
    return setFreePDOName() && this->checkExistence();
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                         const TStatusCollection &aOldStatusCollection)
{
    if (this->mIOPort && mPortUsing && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
    {
        this->mIOPort->close();
    }
    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *USBDeviceBase<T>::getDetectingIterator()
{
    initializeUSBPort();
    this->mDetectingPosition = -1;
    return !mPDOData.isEmpty() ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::find()
{
    if (this->mDetectingPosition < 0 || this->mDetectingPosition >= mPDOData.size())
    {
        return false;
    }

    {
        QMutexLocker lock(&mPDODataGuard);
        auto it = mPDOData.begin();
        std::advance(it, this->mDetectingPosition);

        if (it == mPDOData.end() || !setPDOName(it.key()))
        {
            return false;
        }
    }
    return this->checkExistence();
}

//------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::moveNext()
{
    this->mDetectingPosition++;
    return (this->mDetectingPosition >= 0) && (this->mDetectingPosition < mPDOData.size());
}
