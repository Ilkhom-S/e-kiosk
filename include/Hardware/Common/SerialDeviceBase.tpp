/* @file Реализация шаблонных методов базового класса устройств на COM-порту. */

// Project
#include <Hardware/Common/SerialDeviceBase.h>

// SDK#include <SDK/Drivers/IOPort/COMParameters.h>using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
template <class T> SerialDeviceBase<T>::SerialDeviceBase() : m_PortStatusChanged(false) {
    // данные порта
    m_PortParameters[EParameters::ByteSize].append(8);
    m_PortParameters[EParameters::RTS].append(ERTSControl::Enable);
    m_PortParameters[EParameters::DTR].append(EDTRControl::Disable);
}

//--------------------------------------------------------------------------------
template <class T> QStringList SerialDeviceBase<T>::getOptionalPortSettings() {
    return QStringList() << CHardware::Port::COM::BaudRate << CHardware::Port::COM::Parity
                         << CHardware::Port::COM::ByteSize << CHardware::Port::COM::StopBits
                         << CHardware::Port::COM::RTS << CHardware::Port::COM::DTR;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::release() {
    this->removeConfigParameter(CHardwareSDK::RequiredDevice);

    return T::release();
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::checkConnectionAbility() {
    // Prefixing with 'this->' resolves the dependent base class error
    if (!this->checkError(
            IOPortStatusCode::Error::NotSet,
            [this]() { return this->m_IOPort != nullptr; },
            "IO port is not set")) {
        return false;
    }

    // Qt 5.15/6 compatible cast
    IDevice *deviceInterface = dynamic_cast<IDevice *>(this->m_IOPort);
    if (!deviceInterface) {
        deviceInterface = dynamic_cast<IDevice *>(this->m_IOPort);
    }

    this->setConfigParameter(CHardwareSDK::RequiredDevice, QVariant::fromValue(deviceInterface));

    // QVariant::toString() is safe across Qt 5 and 6
    const QString systemName =
        this->m_IOPort->getDeviceConfiguration().value(CHardwareSDK::SystemName).toString();

    // Combined checks for existence and opening
    return this->checkError(
               IOPortStatusCode::Error::NotConnected,
               [this]() { return this->m_IOPort->isExist() || this->m_IOPort->deviceConnected(); },
               "IO port is not connected") &&
           this->checkError(
               IOPortStatusCode::Error::NotConfigured,
               // Using lambda capture compatible with C++14
               [systemName]() { return !systemName.isEmpty(); },
               "IO port is not set correctly") &&
           this->checkError(
               IOPortStatusCode::Error::Busy,
               [this]() { return this->m_IOPort->open(); },
               "device cannot open port");
}

#define MAKE_SERIAL_PORT_PARAMETER(aParameters, aType)                                             \
    TSerialDevicePortParameter aParameters = this->m_PortParameters[EParameters::aType];           \
    if (aParameters.isEmpty()) {                                                                   \
        this->toLog(LogLevel::Error,                                                               \
                    this->m_DeviceName + QString(": %1 are empty").arg(#aParameters));             \
        return false;                                                                              \
    }                                                                                              \
    if (!optionalPortSettingsEnable && optionalPortSettings.contains(CHardware::Port::COM::aType)) \
        aParameters = TSerialDevicePortParameter() << aParameters[0];

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::makeSearchingList() {
    QStringList optionalPortSettings =
        this->getConfigParameter(CHardwareSDK::OptionalPortSettings).toStringList();
    bool optionalPortSettingsEnable =
        this->getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool();

    MAKE_SERIAL_PORT_PARAMETER(baudRates, BaudRate);
    MAKE_SERIAL_PORT_PARAMETER(parities, Parity);
    MAKE_SERIAL_PORT_PARAMETER(byteSizes, ByteSize);
    MAKE_SERIAL_PORT_PARAMETER(RTSs, RTS);
    MAKE_SERIAL_PORT_PARAMETER(DTRs, DTR);

    foreach (int baudrate, baudRates) {
        foreach (int parity, parities) {
            foreach (int RTS, RTSs) {
                foreach (int DTR, DTRs) {
                    foreach (int bytesize, byteSizes) {
                        SSerialPortParameters params(baudrate, parity, RTS, DTR, bytesize);
                        m_SearchingPortParameters.append(params);
                    }
                }
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::checkExistence() {
    if (this->containsConfigParameter(CHardwareSDK::SearchingType)) {
        QVariantMap configuration;
        configuration.insert(CHardwareSDK::SearchingType,
                             this->getConfigParameter(CHardwareSDK::SearchingType));
        this->m_IOPort->setDeviceConfiguration(configuration);
    }

    this->m_IOPort->initialize();

    MutexLocker locker(&this->m_ExternalMutex);

    if (!this->checkConnectionAbility()) {
        return false;
    }

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging,
                         QVariant().fromValue(this->m_ioMessageLogging));
    configuration.insert(CHardware::Port::DeviceModelName, this->m_DeviceName);
    this->m_IOPort->setDeviceConfiguration(configuration);

    TPortParameters portParameters;
    this->m_IOPort->getParameters(portParameters);
    TPortParameters mainPortParameters = portParameters;

    // TODO: убрать, когда будет реализован соответствующий функционал в ПП
    this->setConfigParameter(CHardwareSDK::OptionalPortSettingsEnable, true);

    if (!this->getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool()) {
        QStringList optionalPortSettings = getOptionalPortSettings();

        auto check = [&](const QString &aConfigParameter, int aPortParameter) {
            if (!this->m_PortParameters[aPortParameter].contains(portParameters[aPortParameter]) &&
                optionalPortSettings.contains(aConfigParameter)) {
                this->toLog(
                    LogLevel::Normal,
                    QString("Change %1: %2 -> %3")
                        .arg(EParameters::Enum_ToString(aPortParameter))
                        .arg(parameterDescription(aPortParameter, portParameters[aPortParameter]))
                        .arg(parameterDescription(aPortParameter,
                                                  this->m_PortParameters[aPortParameter][0])));
                portParameters[aPortParameter] = this->m_PortParameters[aPortParameter][0];
            }
        };

        check(CHardware::Port::COM::BaudRate, EParameters::BaudRate);
        check(CHardware::Port::COM::Parity, EParameters::Parity);
        check(CHardware::Port::COM::ByteSize, EParameters::ByteSize);
        check(CHardware::Port::COM::RTS, EParameters::RTS);
        check(CHardware::Port::COM::DTR, EParameters::DTR);
    }

    if (mainPortParameters != portParameters) {
        if (!this->m_IOPort->setParameters(portParameters)) {
            portParameters = mainPortParameters;
        } else if (!this->checkExistence()) {
            if (this->m_Connected) {
                return false;
            }

            portParameters = mainPortParameters;

            if (!this->m_IOPort->setParameters(portParameters)) {
                return false;
            }
        }
    }

    if (mainPortParameters == portParameters) {
        auto portLog = [&](int aParameter) -> QString {
            return QString(EParameters::Enum_ToString(aParameter)) + " " +
                   parameterDescription(aParameter, portParameters[aParameter]);
        };

        if (!portParameters.isEmpty()) {
            this->toLog(LogLevel::Normal,
                        QString("Port %1 with %2, %3, %4, %5, %6")
                            .arg(this->m_IOPort->getName())
                            .arg(portLog(EParameters::BaudRate))
                            .arg(portLog(EParameters::Parity))
                            .arg(portLog(EParameters::RTS))
                            .arg(portLog(EParameters::DTR))
                            .arg(portLog(EParameters::ByteSize)));

            QStringList logData;

            for (auto parameters = this->m_PortParameters.begin();
                 parameters != this->m_PortParameters.end();
                 ++parameters) {
                int parameterKey = parameters.key();
                int portValue = portParameters[parameters.key()];

                if (!parameters->contains(portValue)) {
                    logData << QString("%1 = %2")
                                   .arg(EParameters::Enum_ToString(parameterKey))
                                   .arg(parameterDescription(parameterKey, portValue));
                }
            }

            if (!logData.isEmpty()) {
                this->toLog(LogLevel::Warning,
                            QString("%1: Port parameter(s) are inadvisable: %2")
                                .arg(this->m_DeviceName)
                                .arg(logData.join(", ")));
                this->m_ioPortStatusCodes.insert(IOPortStatusCode::Warning::MismatchParameters);
            } else {
                this->m_ioPortStatusCodes.remove(IOPortStatusCode::Warning::MismatchParameters);
            }
        }

        if (!this->checkExistence()) {
            return false;
        }
    }

    if (this->m_IOPort) {
        QVariantMap portConfiguration;
        portConfiguration.insert(CHardware::Port::DeviceModelName, this->m_DeviceName);
        this->m_IOPort->setDeviceConfiguration(portConfiguration);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *SerialDeviceBase<T>::getDetectingIterator() {
    if (!this->m_AutoDetectable) {
        return nullptr;
    }

    this->m_SearchingPortParameters.clear();
    this->makeSearchingList();

    this->m_NextParameterIterator = this->m_SearchingPortParameters.begin();

    return this;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::find() {
    TPortParameters portParameters;
    portParameters.insert(EParameters::BaudRate, this->m_CurrentParameter.baudRate);
    portParameters.insert(EParameters::Parity, this->m_CurrentParameter.parity);
    portParameters.insert(EParameters::RTS, this->m_CurrentParameter.RTS);
    portParameters.insert(EParameters::DTR, this->m_CurrentParameter.DTR);
    portParameters.insert(EParameters::ByteSize, this->m_CurrentParameter.byteSize);

    if (!this->m_IOPort->setParameters(portParameters)) {
        this->toLog(
            LogLevel::Error,
            this->m_DeviceName +
                ": Failed to set port parameters, unable to perform current action therefore");
        this->m_ioPortStatusCodes.insert(IOPortStatusCode::Error::Busy);

        return false;
    }

    return this->find();
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::moveNext() {
    if (this->m_NextParameterIterator >= this->m_SearchingPortParameters.end()) {
        return false;
    }

    this->m_CurrentParameter = *this->m_NextParameterIterator;
    this->m_NextParameterIterator++;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::processStatus(TStatusCodes &aStatusCodes) {
    TStatusCodes statusCodes = this->m_ioPortStatusCodes;
    bool result = this->checkConnectionAbility();
    this->m_PortStatusChanged = statusCodes != this->m_ioPortStatusCodes;

    if (!result) {
        return false;
    }

    if (!this->getStatus(aStatusCodes) ||
        aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable)) {
        TStatusCodes otherStatusCodes = this->m_ioPortStatusCodes;
        this->checkConnectionAbility();
        this->m_PortStatusChanged =
            this->m_PortStatusChanged || (otherStatusCodes != this->m_ioPortStatusCodes);

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::environmentChanged() {
    return this->m_PortStatusChanged;
}
