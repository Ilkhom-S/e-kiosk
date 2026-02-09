/* @file Реализация шаблонных методов базового класса устройств на порту. */

// Project
#include <Hardware/Common/PortDeviceBase.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>
using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
template <class T>
PortDeviceBase<T>::PortDeviceBase()
    : m_IOPort(nullptr), m_ioMessageLogging(ELoggingType::None), m_controlRemoving(false) {
    this->m_InitializeRepeatCount = 2;
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::addPortData() {
    EPortTypes::Enum portType = m_IOPort->getType();
    QString port;

    if (portType == EPortTypes::COM) {
        port = "COM";
    }
    if (portType == EPortTypes::USB) {
        port = "USB";
    }
    if (portType == EPortTypes::TCP) {
        port = "TCP";
    }
    if (portType == EPortTypes::VirtualCOM) {
        port = "Virtual COM";
    }
    if (portType == EPortTypes::COMEmulator) {
        port = "COM Emulator";
    }

    this->setDeviceParameter(CDeviceData::Port, port);
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::finalizeInitialization() {
    addPortData();

    m_controlRemoving =
        m_IOPort->getDeviceConfiguration()[CHardware::Port::COM::ControlRemoving].toBool();

    T::finalizeInitialization();
}

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::release() {
    if (!T::release()) {
        return false;
    }

    if (m_IOPort && !m_IOPort->close()) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Failed to close device port");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    T::setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardwareSDK::RequiredDevice)) {
        m_IOPort = dynamic_cast<IIOPort *>(
            aConfiguration[CHardwareSDK::RequiredDevice].value<IDevice *>());
    }

    if (m_IOPort) {
        QVariantMap configuration;
        TVoidMethod forwardingTask =
            this->isAutoDetecting() ? TVoidMethod() : std::bind(&PortDeviceBase::initialize, this);
        configuration.insert(CHardware::Port::OpeningContext, QVariant::fromValue(forwardingTask));

        if (aConfiguration.contains(CHardwareSDK::SearchingType)) {
            configuration.insert(CHardwareSDK::SearchingType,
                                 aConfiguration[CHardwareSDK::SearchingType]);
        }

        m_IOPort->setDeviceConfiguration(configuration);
    }
}

//--------------------------------------------------------------------------------
template <class T> TResult PortDeviceBase<T>::processCommand(char aCommand, QByteArray *aAnswer) {
    return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(const QByteArray &aCommand, QByteArray *aAnswer) {
    return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(char aCommand,
                                          const QByteArray &aCommandData,
                                          QByteArray *aAnswer) {
    return processCommand(QByteArray(1, aCommand), aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(const QByteArray &aCommand,
                                          const QByteArray &aCommandData,
                                          QByteArray *aAnswer) {
    TResult result = execCommand(aCommand, aCommandData, aAnswer);

    if (!m_controlRemoving ||
        ((result != CommandResult::NoAnswer) && (result != CommandResult::Transport))) {
        return result;
    }

    m_IOPort->close();

    if (!m_IOPort->open()) {
        return CommandResult::Port;
    }

    return execCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::checkError(int aError, TBoolMethod aChecking, const QString &aErrorLog) {
    bool contains = m_ioPortStatusCodes.contains(aError);
    m_ioPortStatusCodes.remove(aError);

    if (aChecking()) {
        return true;
    }

    m_ioPortStatusCodes.insert(aError);

    if (!contains) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": " + aErrorLog);
    }

    return false;
};

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::checkExistence() {
    MutexLocker locker(&this->m_ExternalMutex);

    if (!this->checkConnectionAbility()) {
        return false;
    }

    // TODO: сделать настройку плагинов - расширенное логирование
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(m_ioMessageLogging));
    configuration.insert(CHardware::Port::DeviceModelName, this->m_DeviceName);

    m_IOPort->setDeviceConfiguration(configuration);

    if (!T::checkExistence()) {
        return false;
    }

    configuration.insert(CHardware::Port::DeviceModelName, this->m_DeviceName);
    m_IOPort->setDeviceConfiguration(configuration);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::processStatus(TStatusCodes &aStatusCodes) {
    if (!this->checkPort()) {
        if (m_ioPortStatusCodes.isEmpty()) {
            checkError(
                IOPortStatusCode::Error::Busy,
                [&]() -> bool { return m_IOPort->open(); },
                "device cannot open port before getting status");
        }

        return false;
    }

    if (this->getStatus(aStatusCodes) &&
        !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable)) {
        return true;
    }

    if (!this->checkPort()) {
        if (m_ioPortStatusCodes.isEmpty()) {
            checkError(
                IOPortStatusCode::Error::Busy,
                [&]() -> bool { return m_IOPort->open(); },
                "device cannot open port after getting status");
        }

        return false;
    }

    aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
SStatusCodeSpecification PortDeviceBase<T>::getStatusCodeSpecification(int aStatusCode) const {
    return m_ioPortStatusCodes.contains(aStatusCode) ? m_ioPortStatusCodesSpecification[aStatusCode]
                                                     : T::getStatusCodeSpecification(aStatusCode);
}

//--------------------------------------------------------------------------------
template <class T>
QString PortDeviceBase<T>::getTrOfNewProcessed(const TStatusCollection &aStatusCollection,
                                               EWarningLevel::Enum aWarningLevel) {
    TStatusCodes statusCodes = aStatusCollection[aWarningLevel];

    foreach (int statusCode, m_ioPortStatusCodes) {
        if (m_ioPortStatusCodesSpecification[statusCode].warningLevel == aWarningLevel) {
            statusCodes.insert(statusCode);
        }
    }

    return this->getStatusTranslations(statusCodes, false);
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::emitStatusCodes(TStatusCollection &aStatusCollection, int aExtendedStatus) {
    if (!m_ioPortStatusCodes.isEmpty() && aStatusCollection.contains(EWarningLevel::OK)) {
        aStatusCollection[EWarningLevel::OK].remove(DeviceStatusCode::OK::OK);
    }

    foreach (int portStatusCode, m_ioPortStatusCodes) {
        EWarningLevel::Enum warningLevel =
            m_ioPortStatusCodesSpecification[portStatusCode].warningLevel;
        aStatusCollection[warningLevel].insert(portStatusCode);
    }

    T::emitStatusCodes(aStatusCollection, aExtendedStatus);
}

//---------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::canApplyStatusBuffer() {
    bool portError =
        std::find_if(
            m_ioPortStatusCodes.begin(), m_ioPortStatusCodes.end(), [&](int aStatusCode) -> bool {
                return m_ioPortStatusCodesSpecification[aStatusCode].warningLevel ==
                       EWarningLevel::Error;
            }) != m_ioPortStatusCodes.end();

    return !portError && T::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::doPoll(TStatusCodes &aStatusCodes) {
    {
        MutexLocker locker(&this->m_ExternalMutex);

        if (this->m_LogDate.day() != QDate::currentDate().day()) {
            m_IOPort->initialize();
        }
    }

    T::doPoll(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
EWarningLevel::Enum PortDeviceBase<T>::getWarningLevel(const TStatusCollection &aStatusCollection) {
    TStatusCollection portStatusCollection =
        this->getStatusCollection(m_ioPortStatusCodes, &m_ioPortStatusCodesSpecification);
    EWarningLevel::Enum portWarningLevel = this->getWarningLevel(portStatusCollection);
    EWarningLevel::Enum deviceWarningLevel = this->getWarningLevel(aStatusCollection);

    return qMax(deviceWarningLevel, portWarningLevel);
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::setLog(ILog *aLog) {
    T::setLog(aLog);

    if (m_IOPort) {
        m_IOPort->setLog(aLog);
    }
}
