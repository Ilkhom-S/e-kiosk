/* @file Реализация шаблонных методов базового класса устройств на порту. */

// Project
#include <Hardware/Common/PortDeviceBase.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>
using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
template <class T>
PortDeviceBase<T>::PortDeviceBase() : mIOPort(nullptr), mIOMessageLogging(ELoggingType::None), mControlRemoving(false)
{
    this->mInitializeRepeatCount = 2;
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::addPortData()
{
    EPortTypes::Enum portType = mIOPort->getType();
    QString port;

    if (portType == EPortTypes::COM)
        port = "COM";
    if (portType == EPortTypes::USB)
        port = "USB";
    if (portType == EPortTypes::TCP)
        port = "TCP";
    if (portType == EPortTypes::VirtualCOM)
        port = "Virtual COM";
    if (portType == EPortTypes::COMEmulator)
        port = "COM Emulator";

    this->setDeviceParameter(CDeviceData::Port, port);
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::finalizeInitialization()
{
    addPortData();

    mControlRemoving = mIOPort->getDeviceConfiguration()[CHardware::Port::COM::ControlRemoving].toBool();

    T::finalizeInitialization();
}

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::release()
{
    if (!T::release())
    {
        return false;
    }

    if (mIOPort && !mIOPort->close())
    {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to close device port");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::setDeviceConfiguration(const QVariantMap &aConfiguration)
{
    T::setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardwareSDK::RequiredDevice))
    {
        mIOPort = dynamic_cast<IIOPort *>(aConfiguration[CHardwareSDK::RequiredDevice].value<IDevice *>());
    }

    if (mIOPort)
    {
        QVariantMap configuration;
        TVoidMethod forwardingTask =
            this->isAutoDetecting() ? TVoidMethod() : std::bind(&PortDeviceBase::initialize, this);
        configuration.insert(CHardware::Port::OpeningContext, QVariant::fromValue(forwardingTask));

        if (aConfiguration.contains(CHardwareSDK::SearchingType))
        {
            configuration.insert(CHardwareSDK::SearchingType, aConfiguration[CHardwareSDK::SearchingType]);
        }

        mIOPort->setDeviceConfiguration(configuration);
    }
}

//--------------------------------------------------------------------------------
template <class T> TResult PortDeviceBase<T>::processCommand(char aCommand, QByteArray *aAnswer)
{
    return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T> TResult PortDeviceBase<T>::processCommand(const QByteArray &aCommand, QByteArray *aAnswer)
{
    return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(char aCommand, const QByteArray &aCommandData, QByteArray *aAnswer)
{
    return processCommand(QByteArray(1, aCommand), aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(const QByteArray &aCommand, const QByteArray &aCommandData,
                                          QByteArray *aAnswer)
{
    TResult result = execCommand(aCommand, aCommandData, aAnswer);

    if (!mControlRemoving || ((result != CommandResult::NoAnswer) && (result != CommandResult::Transport)))
    {
        return result;
    }

    mIOPort->close();

    if (!mIOPort->open())
    {
        return CommandResult::Port;
    }

    return execCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::checkError(int aError, TBoolMethod aChecking, const QString &aErrorLog)
{
    bool contains = mIOPortStatusCodes.contains(aError);
    mIOPortStatusCodes.remove(aError);

    if (aChecking())
    {
        return true;
    }

    mIOPortStatusCodes.insert(aError);

    if (!contains)
    {
        this->toLog(LogLevel::Error, this->mDeviceName + ": " + aErrorLog);
    }

    return false;
};

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::checkExistence()
{
    MutexLocker locker(&this->mExternalMutex);

    if (!this->checkConnectionAbility())
    {
        return false;
    }

    // TODO: сделать настройку плагинов - расширенное логирование
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(mIOMessageLogging));
    configuration.insert(CHardware::Port::DeviceModelName, this->mDeviceName);

    mIOPort->setDeviceConfiguration(configuration);

    if (!T::checkExistence())
    {
        return false;
    }

    configuration.insert(CHardware::Port::DeviceModelName, this->mDeviceName);
    mIOPort->setDeviceConfiguration(configuration);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::processStatus(TStatusCodes &aStatusCodes)
{
    if (!this->checkPort())
    {
        if (mIOPortStatusCodes.isEmpty())
        {
            checkError(
                IOPortStatusCode::Error::Busy, [&]() -> bool { return mIOPort->open(); },
                "device cannot open port before getting status");
        }

        return false;
    }

    if (this->getStatus(aStatusCodes) && !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
    {
        return true;
    }

    if (!this->checkPort())
    {
        if (mIOPortStatusCodes.isEmpty())
        {
            checkError(
                IOPortStatusCode::Error::Busy, [&]() -> bool { return mIOPort->open(); },
                "device cannot open port after getting status");
        }

        return false;
    }

    aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> SStatusCodeSpecification PortDeviceBase<T>::getStatusCodeSpecification(int aStatusCode) const
{
    return mIOPortStatusCodes.contains(aStatusCode) ? mIOPortStatusCodesSpecification[aStatusCode]
                                                    : T::getStatusCodeSpecification(aStatusCode);
}

//--------------------------------------------------------------------------------
template <class T>
QString PortDeviceBase<T>::getTrOfNewProcessed(const TStatusCollection &aStatusCollection,
                                               EWarningLevel::Enum aWarningLevel)
{
    TStatusCodes statusCodes = aStatusCollection[aWarningLevel];

    foreach (int statusCode, mIOPortStatusCodes)
    {
        if (mIOPortStatusCodesSpecification[statusCode].warningLevel == aWarningLevel)
        {
            statusCodes.insert(statusCode);
        }
    }

    return this->getStatusTranslations(statusCodes, false);
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::emitStatusCodes(TStatusCollection &aStatusCollection, int aExtendedStatus)
{
    if (!mIOPortStatusCodes.isEmpty() && aStatusCollection.contains(EWarningLevel::OK))
    {
        aStatusCollection[EWarningLevel::OK].remove(DeviceStatusCode::OK::OK);
    }

    foreach (int portStatusCode, mIOPortStatusCodes)
    {
        EWarningLevel::Enum warningLevel = mIOPortStatusCodesSpecification[portStatusCode].warningLevel;
        aStatusCollection[warningLevel].insert(portStatusCode);
    }

    T::emitStatusCodes(aStatusCollection, aExtendedStatus);
}

//---------------------------------------------------------------------------
template <class T> bool PortDeviceBase<T>::canApplyStatusBuffer()
{
    bool portError =
        std::find_if(mIOPortStatusCodes.begin(), mIOPortStatusCodes.end(), [&](int aStatusCode) -> bool
                     { return mIOPortStatusCodesSpecification[aStatusCode].warningLevel == EWarningLevel::Error; }) !=
        mIOPortStatusCodes.end();

    return !portError && T::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::doPoll(TStatusCodes &aStatusCodes)
{
    {
        MutexLocker locker(&this->mExternalMutex);

        if (this->mLogDate.day() != QDate::currentDate().day())
        {
            mIOPort->initialize();
        }
    }

    T::doPoll(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T> EWarningLevel::Enum PortDeviceBase<T>::getWarningLevel(const TStatusCollection &aStatusCollection)
{
    TStatusCollection portStatusCollection =
        this->getStatusCollection(mIOPortStatusCodes, &mIOPortStatusCodesSpecification);
    EWarningLevel::Enum portWarningLevel = this->getWarningLevel(portStatusCollection);
    EWarningLevel::Enum deviceWarningLevel = this->getWarningLevel(aStatusCollection);

    return qMax(deviceWarningLevel, portWarningLevel);
}

//--------------------------------------------------------------------------------
template <class T> void PortDeviceBase<T>::setLog(ILog *aLog)
{
    T::setLog(aLog);

    if (mIOPort)
    {
        mIOPort->setLog(aLog);
    }
}