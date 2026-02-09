/* @file Базовый класс устройств на USB-порту. */

#include <QtCore/QMutex>
#include <QtCore/QReadLocker>
#include <QtCore/QSet>
#include <QtCore/QWriteLocker>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QRecursiveMutex>
#endif

#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"
#include "USBDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
// Макрос инициализации: адаптирован под QRecursiveMutex в Qt 6 и QMutex в Qt 5
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define INSTANCE_USB_DEVICE(aClass)                                                                \
    template class aClass;                                                                         \
    template <> aClass::TPDOData aClass::m_PDOData = aClass::TPDOData();                           \
    template <> QRecursiveMutex aClass::m_PDODataGuard = QRecursiveMutex();
#else
#define INSTANCE_USB_DEVICE(aClass)                                                                \
    template class aClass;                                                                         \
    template <> aClass::TPDOData aClass::m_PDOData = aClass::TPDOData();                           \
    template <> QMutex aClass::m_PDODataGuard(QMutex::Recursive);
#endif

INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>>)
INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoHID>>)

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::USBDeviceBase() : m_PDODetecting(false), m_PortUsing(true) {
    // В шаблонах используем this-> для доступа к членам базового класса
    this->m_IOPort = &m_USBPort;
    m_DetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
    this->m_ReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T> USBDeviceBase<T>::~USBDeviceBase() {
    resetPDOName();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::release() {
    bool result = T::release();
    resetPDOName();
    return result;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::initialize() {
    START_IN_WORKING_THREAD(initialize)
    initializeUSBPort();
    setFreePDOName();
    T::initialize();
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::setFreePDOName() {
    QMutexLocker lock(&m_PDODataGuard);

    // Поиск первого свободного PDO-имени
    for (auto it = m_PDOData.begin(); it != m_PDOData.end(); ++it) {
        if (it.value()) {
            return setPDOName(it.key());
        }
    }
    return false;
}

//--------------------------------------------------------------------------------
template <class T> void USBDeviceBase<T>::resetPDOName() {
    QMutexLocker lock(&m_PDODataGuard);
    // Извлекаем системное имя из конфигурации порта
    QString pdoName =
        this->m_IOPort->getDeviceConfiguration().value(CHardwareSDK::System_Name).toString();

    if (m_PDOData.contains(pdoName)) {
        m_PDOData[PDOName] = true;
    }
}

//--------------------------------------------------------------------------------
#include <QtSerialPort/QSerialPortInfo>

template <class T> bool USBDeviceBase<T>::setPDOName(const QString &aPDOName) {
    // 1. Ищем информацию о порте по его системному имени (aPDOName)
    // Это работает на Windows (COM1), Linux (/dev/ttyUSB0) и Mac.
    QSerialPortInfo portInfo(aPDOName);

    if (portInfo.isNull()) {
        this->toLog(LogLevel::Error,
                    QStringLiteral("%1: Port %2 not found via QSerialPortInfo")
                        .arg(this->m_DeviceName, aPDOName));
        return false;
    }

    // 2. Получаем VID и PID напрямую из QSerialPortInfo
    quint16 vid = portInfo.hasVendorIdentifier() ? portInfo.vendorIdentifier() : 0;
    quint16 pid = portInfo.hasProductIdentifier() ? portInfo.productIdentifier() : 0;

    QString logVID = ProtocolUtils::toHexLog(vid);
    QString logPID = ProtocolUtils::toHexLog(pid);

    // 3. Проверка VID в данных авто поиска (CSpecification)
    if (!m_DetectingData->data().contains(VID)) {
        this->toLog(LogLevel::Normal,
                    QStringLiteral("%1: No such VID %2").arg(this->m_DeviceName, logVID));
        return false;
    }

    const auto &pidData = m_DetectingData->value(VID).constData();

    // 4. Проверка PID
    if (!PIDData.contains(pid)) {
        this->toLog(
            LogLevel::Normal,
            QStringLiteral("%1: No PID %2 for VID %3").arg(this->m_DeviceName, logPID, logVID));
        return false;
    }

    // 5. Установка данных устройства из мета-данных
    const auto &data = PIDData[pid];
    this->m_DeviceName = data.model;
    this->m_Verified = data.verified;

    // Блокируем имя порта как занятое в статическом массиве
    m_PDOData[aPDOName] = false;

    // 6. Сохранение конфигурации
    QVariantMap configuration;
    configuration.insert(CHardwareSDK::System_Name, aPDOName);

    this->toLog(LogLevel::Normal,
                QStringLiteral("%1: Set USB Device %2 (VID %3 PID %4)")
                    .arg(this->m_DeviceName, aPDOName, logVID, logPID));

    this->m_IOPort->setDeviceConfiguration(configuration);

    return true;
}

//--------------------------------------------------------------------------------
#include <QtSerialPort/QSerialPortInfo>

template <class T> void USBDeviceBase<T>::initializeUSBPort() {
    // 1. Получаем список всех доступных последовательных портов в системе.
    // Это работает на Windows, Linux и macOS.
    const auto availablePorts = QSerialPortInfo::availablePorts();

    // Подготавливаем набор имен текущих физических портов для синхронизации
    QSet<QString> systemPortNames;
    for (const QSerialPortInfo &info : availablePorts) {
        system_PortNames.insert(info.portName());
    }

    QMutexLocker lock(&m_PDODataGuard);

    // 2. Очистка m_PDOData от портов, которые больше не существуют в системе.
    QSet<QString> cachedPortNames(m_PDOData.keyBegin(), m_PDOData.keyEnd());
    QSet<QString> deletedPorts = cachedPortNames - system_PortNames;

    for (const QString &portName : deletedPorts) {
        m_PDOData.remove(portName);
    }

    // 3. Сопоставление найденных портов с данными авто поиска (VID/PID).
    for (const QSerialPortInfo &info : availablePorts) {
        // Пропускаем, если порт уже есть в базе данных или не имеет идентификаторов
        if (m_PDOData.contains(info.portName()) || !info.hasVendorIdentifier() ||
            !info.hasProductIdentifier()) {
            continue;
        }

        quint16 vID = info.vendorIdentifier();
        quint16 pID = info.productIdentifier();

        // Проходим по иерархии данных авто поиска (VID -> PID)
        // Используем constData() из вашего CSpecification для безопасности.
        for (auto jt = this->m_DetectingData->data().begin();
             jt != this->m_DetectingData->data().end();
             ++jt) {
            if (vID == jt.key()) {
                const auto &pidMap = jt.value().constData();

                // Если PID найден в списке разрешенных для этого устройства
                if (pidMap.contains(pID)) {
                    // Помечаем порт как свободный (true) для последующего использования
                    m_PDOData.insert(info.portName(), true);
                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkConnectionAbility() {
    return !m_PortUsing || this->checkError(
                               IOPortStatusCode::Error::Busy,
                               [this]() { return this->m_IOPort->open(); },
                               "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::checkPort() {
    if (this->m_IOPort->isExist()) {
        return true;
    } else if (!this->m_IOPort->deviceConnected()) {
        return false;
    }

    initializeUSBPort();
    return setFreePDOName() && this->checkExistence();
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                         const TStatusCollection &aOldStatusCollection) {
    if (this->m_IOPort && m_PortUsing &&
        aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable)) {
        this->m_IOPort->close();
    }
    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *USBDeviceBase<T>::getDetectingIterator() {
    initializeUSBPort();
    this->m_DetectingPosition = -1;
    return !m_PDOData.isEmpty() ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::find() {
    if (this->m_DetectingPosition < 0 || this->m_DetectingPosition >= m_PDOData.size()) {
        return false;
    }

    {
        QMutexLocker lock(&m_PDODataGuard);
        auto it = m_PDOData.begin();
        std::advance(it, this->m_DetectingPosition);

        if (it == m_PDOData.end() || !setPDOName(it.key())) {
            return false;
        }
    }
    return this->checkExistence();
}

//------------------------------------------------------------------------------
template <class T> bool USBDeviceBase<T>::moveNext() {
    this->m_DetectingPosition++;
    return (this->m_DetectingPosition >= 0) && (this->m_DetectingPosition < m_PDOData.size());
}
