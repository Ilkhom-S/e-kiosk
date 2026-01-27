/* @file Базовый класс устройств на COM-порту. */

// System
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/IOPorts/IOPortStatusCodes.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

// Project
#include "SerialDeviceBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//-------------------------------------------------------------------------------
template class SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoWatchdog>>;
template class SerialDeviceBase<PortDeviceBase<DeviceBase<ProtoModem>>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>;

//--------------------------------------------------------------------------------
template <class T> SerialDeviceBase<T>::SerialDeviceBase() : mPortStatusChanged(false) {
    // данные порта
    mPortParameters[EParameters::ByteSize].append(8);
    mPortParameters[EParameters::RTS].append(ERTSControl::Enable);
    mPortParameters[EParameters::DTR].append(EDTRControl::Disable);
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
            IOPortStatusCode::Error::NotSet, [this]() { return this->mIOPort != nullptr; }, "IO port is not set")) {
        return false;
    }

    // Qt 5.15/6 compatible cast
    IDevice *deviceInterface = dynamic_cast<IDevice *>(this->mIOPort);
    if (!deviceInterface) {
        deviceInterface = dynamic_cast<IDevice *>(this->mIOPort);
    }

    this->setConfigParameter(CHardwareSDK::RequiredDevice, QVariant::fromValue(deviceInterface));

    // QVariant::toString() is safe across Qt 5 and 6
    const QString systemName = this->mIOPort->getDeviceConfiguration().value(CHardwareSDK::SystemName).toString();

    // Combined checks for existence and opening
    return this->checkError(
               IOPortStatusCode::Error::NotConnected,
               [this]() { return this->mIOPort->isExist() || this->mIOPort->deviceConnected(); },
               "IO port is not connected") &&
           this->checkError(
               IOPortStatusCode::Error::NotConfigured,
               // Using lambda capture compatible with C++14
               [systemName]() { return !systemName.isEmpty(); }, "IO port is not set correctly") &&
           this->checkError(
               IOPortStatusCode::Error::Busy, [this]() { return this->mIOPort->open(); }, "device cannot open port");
}

#define MAKE_SERIAL_PORT_PARAMETER(aParameters, aType)                                                                 \
    TSerialDevicePortParameter aParameters = this->mPortParameters[EParameters::aType];                                \
    if (aParameters.isEmpty()) {                                                                                       \
        this->toLog(LogLevel::Error, this->mDeviceName + QString(": %1 are empty").arg(#aParameters));                 \
        return false;                                                                                                  \
    }                                                                                                                  \
    if (!optionalPortSettingsEnable && optionalPortSettings.contains(CHardware::Port::COM::aType))                     \
        aParameters = TSerialDevicePortParameter() << aParameters[0];

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::makeSearchingList() {
    QStringList optionalPortSettings = this->getConfigParameter(CHardwareSDK::OptionalPortSettings).toStringList();
    bool optionalPortSettingsEnable = this->getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool();

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
                        mSearchingPortParameters.append(params);
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
        configuration.insert(CHardwareSDK::SearchingType, this->getConfigParameter(CHardwareSDK::SearchingType));
        this->mIOPort->setDeviceConfiguration(configuration);
    }

    this->mIOPort->initialize();

    MutexLocker locker(&this->mExternalMutex);

    if (!this->checkConnectionAbility()) {
        return false;
    }

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(this->mIOMessageLogging));
    configuration.insert(CHardware::Port::DeviceModelName, this->mDeviceName);
    this->mIOPort->setDeviceConfiguration(configuration);

    TPortParameters portParameters;
    this->mIOPort->getParameters(portParameters);
    TPortParameters mainPortParameters = portParameters;

    // TODO: убрать, когда будет реализован соответствующий функционал в ПП
    this->setConfigParameter(CHardwareSDK::OptionalPortSettingsEnable, true);

    if (!this->getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool()) {
        QStringList optionalPortSettings = getOptionalPortSettings();

        auto check = [&](const QString &aConfigParameter, int aPortParameter) {
            if (!this->mPortParameters[aPortParameter].contains(portParameters[aPortParameter]) &&
                optionalPortSettings.contains(aConfigParameter)) {
                this->toLog(LogLevel::Normal,
                            QString("Change %1: %2 -> %3")
                                .arg(EParameters::EnumToString(aPortParameter))
                                .arg(parameterDescription(aPortParameter, portParameters[aPortParameter]))
                                .arg(parameterDescription(aPortParameter, this->mPortParameters[aPortParameter][0])));
                portParameters[aPortParameter] = this->mPortParameters[aPortParameter][0];
            }
        };

        check(CHardware::Port::COM::BaudRate, EParameters::BaudRate);
        check(CHardware::Port::COM::Parity, EParameters::Parity);
        check(CHardware::Port::COM::ByteSize, EParameters::ByteSize);
        check(CHardware::Port::COM::RTS, EParameters::RTS);
        check(CHardware::Port::COM::DTR, EParameters::DTR);
    }

    if (mainPortParameters != portParameters) {
        if (!this->mIOPort->setParameters(portParameters)) {
            portParameters = mainPortParameters;
        } else if (!this->checkExistence()) {
            if (this->mConnected) {
                return false;
            }

            portParameters = mainPortParameters;

            if (!this->mIOPort->setParameters(portParameters)) {
                return false;
            }
        }
    }

    if (mainPortParameters == portParameters) {
        auto portLog = [&](int aParameter) -> QString {
            return QString(EParameters::EnumToString(aParameter)) + " " +
                   parameterDescription(aParameter, portParameters[aParameter]);
        };

        if (!portParameters.isEmpty()) {
            this->toLog(LogLevel::Normal, QString("Port %1 with %2, %3, %4, %5, %6")
                                              .arg(this->mIOPort->getName())
                                              .arg(portLog(EParameters::BaudRate))
                                              .arg(portLog(EParameters::Parity))
                                              .arg(portLog(EParameters::RTS))
                                              .arg(portLog(EParameters::DTR))
                                              .arg(portLog(EParameters::ByteSize)));

            QStringList logData;

            for (auto parameters = this->mPortParameters.begin(); parameters != this->mPortParameters.end();
                 ++parameters) {
                int parameterKey = parameters.key();
                int portValue = portParameters[parameters.key()];

                if (!parameters->contains(portValue)) {
                    logData << QString("%1 = %2")
                                   .arg(EParameters::EnumToString(parameterKey))
                                   .arg(parameterDescription(parameterKey, portValue));
                }
            }

            if (!logData.isEmpty()) {
                this->toLog(LogLevel::Warning, QString("%1: Port parameter(s) are inadvisable: %2")
                                                   .arg(this->mDeviceName)
                                                   .arg(logData.join(", ")));
                this->mIOPortStatusCodes.insert(IOPortStatusCode::Warning::MismatchParameters);
            } else {
                this->mIOPortStatusCodes.remove(IOPortStatusCode::Warning::MismatchParameters);
            }
        }

        if (!this->checkExistence()) {
            return false;
        }
    }

    if (this->mIOPort) {
        QVariantMap portConfiguration;
        portConfiguration.insert(CHardware::Port::DeviceModelName, this->mDeviceName);
        this->mIOPort->setDeviceConfiguration(portConfiguration);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *SerialDeviceBase<T>::getDetectingIterator() {
    if (!this->mAutoDetectable) {
        return nullptr;
    }

    this->mSearchingPortParameters.clear();
    this->makeSearchingList();

    this->mNextParameterIterator = this->mSearchingPortParameters.begin();

    return this;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::find() {
    TPortParameters portParameters;
    portParameters.insert(EParameters::BaudRate, this->mCurrentParameter.baudRate);
    portParameters.insert(EParameters::Parity, this->mCurrentParameter.parity);
    portParameters.insert(EParameters::RTS, this->mCurrentParameter.RTS);
    portParameters.insert(EParameters::DTR, this->mCurrentParameter.DTR);
    portParameters.insert(EParameters::ByteSize, this->mCurrentParameter.byteSize);

    if (!this->mIOPort->setParameters(portParameters)) {
        this->toLog(LogLevel::Error,
                    this->mDeviceName + ": Failed to set port parameters, unable to perform current action therefore");
        this->mIOPortStatusCodes.insert(IOPortStatusCode::Error::Busy);

        return false;
    }

    return this->find();
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::moveNext() {
    if (this->mNextParameterIterator >= this->mSearchingPortParameters.end()) {
        return false;
    }

    this->mCurrentParameter = *this->mNextParameterIterator;
    this->mNextParameterIterator++;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::processStatus(TStatusCodes &aStatusCodes) {
    TStatusCodes statusCodes = this->mIOPortStatusCodes;
    bool result = this->checkConnectionAbility();
    this->mPortStatusChanged = statusCodes != this->mIOPortStatusCodes;

    if (!result) {
        return false;
    }

    if (!this->getStatus(aStatusCodes) || aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable)) {
        TStatusCodes otherStatusCodes = this->mIOPortStatusCodes;
        this->checkConnectionAbility();
        this->mPortStatusChanged = this->mPortStatusChanged || (otherStatusCodes != this->mIOPortStatusCodes);

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool SerialDeviceBase<T>::environmentChanged() {
    return this->mPortStatusChanged;
}

//--------------------------------------------------------------------------------
double getFrameSize(const TPortParameters &aPortParameters) {
    int parity = aPortParameters[EParameters::Parity];
    int bytesize = aPortParameters[EParameters::ByteSize];
    int stop = aPortParameters[EParameters::StopBits];

    int parityBits = (parity == EParity::No) ? 0 : 1;
    double stopBits = (stop == EStopBits::One) ? 1 : ((stop == EStopBits::Two) ? 2 : 1.5);

    return 1 + bytesize + parityBits + stopBits;
}

//--------------------------------------------------------------------------------
// Explicit template instantiations for SerialDeviceBase constructor
template SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>::SerialDeviceBase();

// Explicit template instantiation for SerialDeviceBase constructor (ProtoDispenser)
template SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>::SerialDeviceBase();
