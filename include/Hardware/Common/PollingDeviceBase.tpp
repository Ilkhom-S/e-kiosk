/* @file Реализация базового класса устройств с поллингом. */

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

//--------------------------------------------------------------------------------
template <class T>
PollingDeviceBase<T>::PollingDeviceBase()
    : m_PollingInterval(0), m_PollingActive(false), m_ForceNotWaitFirst(false) {
    // Таймер переносится в поток устройства, чтобы обработка сигналов
    // происходила в контексте mThread, а не главного (GUI) потока.
    this->m_Polling.moveToThread(&this->m_Thread);

    // В шаблонных классах C++14/17 использование макросов SIGNAL/SLOT часто приводит
    // к ошибкам поиска имен. Синтаксис на указателях проверяется при компиляции.
    // static_cast необходим для явного указания типа в контексте шаблона.
    QObject::connect(&this->m_Polling,
                     &QTimer::timeout,
                     static_cast<PollingDeviceBase *>(this),
                     &PollingDeviceBase::onPoll);
}

//--------------------------------------------------------------------------------
template <class T> bool PollingDeviceBase<T>::release() {
    this->releasePolling();

    return this->DeviceBase<T>::release();
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::finalizeInitialization() {
    if (!this->m_Connected) {
        this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
    }

    bool notWaitFirst = this->m_ForceNotWaitFirst || !this->m_Connected;

    this->startPolling(notWaitFirst);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::setPollingActive(bool aActive) {
    if (!this->m_OperatorPresence) {
        this->toLog(LogLevel::Normal, aActive ? "Start polling." : "Stop polling.");

        aActive ? this->m_Polling.start(this->m_PollingInterval) : this->m_Polling.stop();

        this->m_PollingActive = aActive;
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::startPolling(bool aNotWaitFirst) {
    if (this->m_PollingActive) {
        return;
    }

    Qt::ConnectionType connectionType =
        !this->isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;

    if (this->m_PollingInterval) {
        QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, true));
    }

    if (!aNotWaitFirst) {
        QMetaObject::invokeMethod(this, "onPoll", connectionType);
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::stopPolling(bool aWait) {
    if (this->m_PollingActive) {
        Qt::ConnectionType connectionType =
            !this->isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;
        QMetaObject::invokeMethod(this, "setPollingActive", connectionType, Q_ARG(bool, false));

        if (aWait) {
            this->waitCondition([&]() -> bool { return !this->m_PollingActive; },
                                CPollingDeviceBase::StopWaiting);
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::setPollingInterval(int aPollingInterval) {
    Q_ASSERT(aPollingInterval != 0);

    int delta = this->m_PollingInterval - aPollingInterval;
    this->m_PollingInterval = aPollingInterval;

    if (this->m_PollingActive && delta) {
        this->stopPolling();
        this->startPolling(false);
    }
}

//--------------------------------------------------------------------------------
template <class T>
void PollingDeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                             const TStatusCollection &aOldStatusCollection) {
    if (this->m_Connected) {
        for (int i = 0; i < this->m_PPTaskList.size(); ++i) {
            this->m_PPTaskList[i]();

            this->m_PPTaskList.removeAt(i--);
        }
    }

    this->DeviceBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingDeviceBase<T>::waitCondition(TBoolMethod aCondition, const SWaitingData &aWaitingData) {
    if (this->DeviceBase<T>::isWorkingThread()) {
        return aCondition();
    }

    PollingExpector expector;

    return expector.wait(aCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::reInitialize() {
    this->stopPolling(false);

    this->DeviceBase<T>::reInitialize();
}

//---------------------------------------------------------------------------
template <class T> bool PollingDeviceBase<T>::isInitializationError(TStatusCodes &aStatusCodes) {
    return this->m_PollingActive && this->DeviceBase<T>::isInitializationError(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T> void PollingDeviceBase<T>::releasePolling() {
    if (!this->isAutoDetecting() &&
        (!this->m_StatusCollection.isEmpty() || (this->m_Initialized == ERequestStatus::InProcess))) {
        this->waitCondition([&]() -> bool { return this->m_PollingActive; },
                            CPollingDeviceBase::StopWaiting);
    }

    this->stopPolling();
}
