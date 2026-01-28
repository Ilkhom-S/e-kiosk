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
#include <QtSerialPort/QSerialPortInfo>

template <class T> bool USBDeviceBase<T>::setPDOName(const QString &aPDOName)
{
    // 1. Ищем информацию о порте по его системному имени (aPDOName)
    // Это работает на Windows (COM1), Linux (/dev/ttyUSB0) и Mac.
    QSerialPortInfo portInfo(aPDOName);

    if (portInfo.isNull())
    {
        this->toLog(LogLevel::Error,
                    QStringLiteral("%1: Port %2 not found via QSerialPortInfo").arg(this->mDeviceName, aPDOName));
        return false;
    }

    // 2. Получаем VID и PID напрямую из QSerialPortInfo
    quint16 VID = portInfo.hasVendorIdentifier() ? portInfo.vendorIdentifier() : 0;
    quint16 PID = portInfo.hasProductIdentifier() ? portInfo.productIdentifier() : 0;

    QString logVID = ProtocolUtils::toHexLog(VID);
    QString logPID = ProtocolUtils::toHexLog(PID);

    // 3. Проверка VID в данных авто поиска (CSpecification)
    if (!mDetectingData->data().contains(VID))
    {
        this->toLog(LogLevel::Normal, QStringLiteral("%1: No such VID %2").arg(this->mDeviceName, logVID));
        return false;
    }

    const auto &PIDData = mDetectingData->value(VID).constData();

    // 4. Проверка PID
    if (!PIDData.contains(PID))
    {
        this->toLog(LogLevel::Normal,
                    QStringLiteral("%1: No PID %2 for VID %3").arg(this->mDeviceName, logPID, logVID));
        return false;
    }

    // 5. Установка данных устройства из мета-данных
    const auto &data = PIDData[PID];
    this->mDeviceName = data.model;
    this->mVerified = data.verified;

    // Блокируем имя порта как занятое в статическом массиве
    mPDOData[aPDOName] = false;

    // 6. Сохранение конфигурации
    QVariantMap configuration;
    configuration.insert(CHardwareSDK::SystemName, aPDOName);

    this->toLog(
        LogLevel::Normal,
        QStringLiteral("%1: Set USB Device %2 (VID %3 PID %4)").arg(this->mDeviceName, aPDOName, logVID, logPID));

    this->mIOPort->setDeviceConfiguration(configuration);

    return true;
}

//--------------------------------------------------------------------------------
#include <QtSerialPort/QSerialPortInfo>

template <class T> void USBDeviceBase<T>::initializeUSBPort()
{
    // 1. Получаем список всех доступных последовательных портов в системе.
    // Это работает на Windows, Linux и macOS.
    const auto availablePorts = QSerialPortInfo::availablePorts();

    // Подготавливаем набор имен текущих физических портов для синхронизации
    QSet<QString> systemPortNames;
    for (const QSerialPortInfo &info : availablePorts)
    {
        systemPortNames.insert(info.portName());
    }

    QMutexLocker lock(&mPDODataGuard);

    // 2. Очистка mPDOData от портов, которые больше не существуют в системе.
    QSet<QString> cachedPortNames(mPDOData.keyBegin(), mPDOData.keyEnd());
    QSet<QString> deletedPorts = cachedPortNames - systemPortNames;

    for (const QString &portName : deletedPorts)
    {
        mPDOData.remove(portName);
    }

    // 3. Сопоставление найденных портов с данными авто поиска (VID/PID).
    for (const QSerialPortInfo &info : availablePorts)
    {
        // Пропускаем, если порт уже есть в базе данных или не имеет идентификаторов
        if (mPDOData.contains(info.portName()) || !info.hasVendorIdentifier() || !info.hasProductIdentifier())
        {
            continue;
        }

        quint16 vID = info.vendorIdentifier();
        quint16 pID = info.productIdentifier();

        // Проходим по иерархии данных авто поиска (VID -> PID)
        // Используем constData() из вашего CSpecification для безопасности.
        for (auto jt = this->mDetectingData->data().begin(); jt != this->mDetectingData->data().end(); ++jt)
        {
            if (vID == jt.key())
            {
                const auto &pidMap = jt.value().constData();

                // Если PID найден в списке разрешенных для этого устройства
                if (pidMap.contains(pID))
                {
                    // Помечаем порт как свободный (true) для последующего использования
                    mPDOData.insert(info.portName(), true);
                    break;
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
