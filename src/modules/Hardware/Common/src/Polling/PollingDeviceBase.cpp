/* @file Базовый класс устройств с поллингом. */

// System
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

// Project
#include "PollingDeviceBase.h"

template class PollingDeviceBase<ProtoPrinter>;
template class PollingDeviceBase<ProtoDispenser>;
template class PollingDeviceBase<ProtoCashAcceptor>;
template class PollingDeviceBase<ProtoFR>;
template class PollingDeviceBase<ProtoHID>;
template class PollingDeviceBase<ProtoMifareReader>;
template class PollingDeviceBase<ProtoDeviceBase>;
template class PollingDeviceBase<ProtoWatchdog>;

//--------------------------------------------------------------------------------
template <class T>
PollingDeviceBase<T>::PollingDeviceBase() : mPollingInterval(0), mPollingActive(false), mForceNotWaitFirst(false)
{
    // Таймер переносится в поток устройства, чтобы обработка сигналов
    // происходила в контексте mThread, а не главного (GUI) потока.
    this->mPolling.moveToThread(&this->mThread);

    // В шаблонных классах C++14/17 использование макросов SIGNAL/SLOT часто приводит
    // к ошибкам поиска имен. Синтаксис на указателях проверяется при компиляции.
    // static_cast необходим для явного указания типа в контексте шаблона.
    QObject::connect(&this->mPolling, &QTimer::timeout, static_cast<PollingDeviceBase *>(this),
                     &PollingDeviceBase::onPoll);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::releasePolling()
{
    if (!this->isAutoDetecting() &&
        (!this->mStatusCollection.isEmpty() || (this->mInitialized == ERequestStatus::InProcess)))
    {
        this->waitCondition([&]() -> bool { return this->mPollingActive; }, CPollingDeviceBase::StopWaiting);
    }

    this->stopPolling();
}

//--------------------------------------------------------------------------------
template <class T> bool PollingDeviceBase<T>::release()
{
    this->releasePolling();

    return this->DeviceBase<T>::release();
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::finalizeInitialization()
{
    if (!this->mConnected)
    {
        this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
    }

    bool notWaitFirst = this->mForceNotWaitFirst || !this->mConnected;

    this->startPolling(notWaitFirst);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::setPollingActive(bool aActive)
{
    if (!this->mOperatorPresence)
    {
        this->toLog(LogLevel::Normal, aActive ? "Start polling." : "Stop polling.");

        aActive ? this->mPolling.start(this->mPollingInterval) : this->mPolling.stop();

        this->mPollingActive = aActive;
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::startPolling(bool aNotWaitFirst)
{
    if (this->mPollingActive)
    {
        return;
    }

    Qt::ConnectionType connectionType = !this->isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;

    if (this->mPollingInterval)
    {
        QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, true));
    }

    if (!aNotWaitFirst)
    {
        QMetaObject::invokeMethod(this, "onPoll", connectionType);
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::stopPolling(bool aWait)
{
    if (this->mPollingActive)
    {
        Qt::ConnectionType connectionType =
            !this->isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;
        QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, false));

        if (aWait)
        {
            this->waitCondition([&]() -> bool { return !this->mPollingActive; }, CPollingDeviceBase::StopWaiting);
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::setPollingInterval(int aPollingInterval)
{
    Q_ASSERT(aPollingInterval != 0);

    int delta = this->mPollingInterval - aPollingInterval;
    this->mPollingInterval = aPollingInterval;

    if (this->mPollingActive && delta)
    {
        this->stopPolling();
        this->startPolling(false);
    }
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                             const TStatusCollection &aOldStatusCollection)
{
    if (this->mConnected)
    {
        for (int i = 0; i < this->mPPTaskList.size(); ++i)
        {
            this->mPPTaskList[i]();

            this->mPPTaskList.removeAt(i--);
        }
    }

    this->DeviceBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T> bool PollingDeviceBase<T>::waitCondition(TBoolMethod aCondition, const SWaitingData &aWaitingData)
{
    if (this->DeviceBase<T>::isWorkingThread())
    {
        return aCondition();
    }

    PollingExpector expector;

    return expector.wait(aCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::reInitialize()
{
    this->stopPolling(false);

    this->DeviceBase<T>::reInitialize();
}

//---------------------------------------------------------------------------
template <class T> bool PollingDeviceBase<T>::isInitializationError(TStatusCodes &aStatusCodes)
{
    return this->mPollingActive && this->DeviceBase<T>::isInitializationError(aStatusCodes);
}

//--------------------------------------------------------------------------------
// Explicit template instantiations for setPollingInterval method
template void PollingDeviceBase<ProtoCashAcceptor>::setPollingInterval(int);

// Explicit template instantiation for PollingDeviceBase constructor
template PollingDeviceBase<ProtoCashAcceptor>::PollingDeviceBase();

// Explicit template instantiation for PollingDeviceBase constructor (ProtoDispenser)
template PollingDeviceBase<ProtoDispenser>::PollingDeviceBase();
// Explicit template instantiation for PollingDeviceBase constructor (ProtoPrinter)
template PollingDeviceBase<ProtoPrinter>::PollingDeviceBase();
