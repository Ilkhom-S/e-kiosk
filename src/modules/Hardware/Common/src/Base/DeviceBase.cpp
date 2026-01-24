/* @file Базовый класс устройства. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <QtCore/QtAlgorithms>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/PluginConstants.h>
#include <Common/Version.h>

// System
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

// Project
#include "DeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class DeviceBase<ProtoPrinter>;
template class DeviceBase<ProtoDispenser>;
template class DeviceBase<ProtoCashAcceptor>;
template class DeviceBase<ProtoWatchdog>;
template class DeviceBase<ProtoModem>;
template class DeviceBase<ProtoFR>;
template class DeviceBase<ProtoHID>;
template class DeviceBase<ProtoMifareReader>;
template class DeviceBase<ProtoDeviceBase>;

//--------------------------------------------------------------------------------
template <class T> DeviceBase<T>::DeviceBase() : mExternalMutex(), mResourceMutex() {
    this->moveToThread(&this->mThread);

    this->mDeviceName = CDevice::DefaultName;
    this->mBadAnswerCounter = 0;
    this->mMaxBadAnswers = 0;
    this->mPostPollingAction = true, this->mVerified = true;
    this->mModelCompatibility = true;
    this->mLastWarningLevel = static_cast<EWarningLevel::Enum>(-1);
    this->mConnected = false;
    this->mInitialized = ERequestStatus::Fail;
    this->mVersion = Humo::getVersion();
    this->mOldFirmware = false;
    this->mInitializeRepeatCount = 1;
    this->mAutoDetectable = true;
    this->mNeedReboot = false;
    this->mForceStatusBufferEnabled = false;

    this->mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new DeviceStatusCode::CSpecifications());

    this->mRecoverableErrors.insert(DeviceStatusCode::Error::Initialization);

    this->mUnsafeStatusCodes =
        TStatusCodes() << DeviceStatusCode::OK::Busy << DeviceStatusCode::OK::Initialization

                       << DeviceStatusCode::Warning::ThirdPartyDriver << DeviceStatusCode::Warning::Developing
                       << DeviceStatusCode::Warning::OperationError << DeviceStatusCode::Warning::UnknownDataExchange;

    mStatusCollectionHistory.setSize(CDevice::StatusCollectionHistoryCount);
    mReplaceableStatuses << DeviceStatusCode::Error::NotAvailable << DeviceStatusCode::Error::Unknown;
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::subscribe(const char *aSignal, QObject *aReceiver, const char *aSlot) {
    return aReceiver->connect(this, aSignal, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::unsubscribe(const char *aSignal, QObject *aReceiver) {
    return this->disconnect(aSignal, aReceiver);
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::checkConnectionAbility() {
    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::updateParameters() {
    return true;
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::setInitialData() {
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::isConnected() {
    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::environmentChanged() {
    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::isPowerReboot() {
    TStatusCollection statusCollection1 = mStatusCollectionHistory.lastValue(1);
    TStatusCollection statusCollection2 = mStatusCollectionHistory.lastValue(2);

    return mStatusCollectionHistory.isEmpty() || (statusCollection2.contains(DeviceStatusCode::Error::NotAvailable) &&
                                                  !statusCollection1.contains(DeviceStatusCode::Error::NotAvailable));
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::checkExistence() {
    MutexLocker locker(&mExternalMutex);

    bool autoDetecting = this->isAutoDetecting();

    if (!this->mAutoDetectable && autoDetecting) {
        this->toLog(LogLevel::Normal, this->mDeviceName + " can not be found via autodetecting at all.");
        return false;
    }

    this->toLog(LogLevel::Normal, "Trying to identify device " + this->mDeviceName);

    this->mVerified = true;
    this->mModelCompatibility = true;
    this->mOldFirmware = false;

    bool doPostPollingAction = false;

    qSwap(this->mPostPollingAction, doPostPollingAction);
    this->mThread.setObjectName(this->mDeviceName);
    this->mConnected = this->isConnected();
    this->mThread.setObjectName(this->mDeviceName);
    qSwap(this->mPostPollingAction, doPostPollingAction);

    if (!this->mModelCompatibility && autoDetecting) {
        this->toLog(LogLevel::Error, this->mDeviceName +
                                         " can not be found via autodetecting as unsupported by plugin " +
                                         this->getConfigParameter(CHardware::PluginPath).toString());
        return false;
    } else if (!this->mConnected) {
        this->toLog(LogLevel::Error, QString("Failed to identify %1.").arg(this->mDeviceName));
        return false;
    }

    this->setConfigParameter(CHardwareSDK::ModelName, this->mDeviceName);

    this->toLog(LogLevel::Normal, QString("Device %1 is identified.").arg(this->mDeviceName));

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::initialize() {
    // 1. Исправляем передачу адреса метода в макрос для шаблонов (dependent name fix)
    START_IN_WORKING_THREAD(&DeviceBase<T>::initialize)

    // 2. Используем QStringLiteral для всех константных строк для оптимизации памяти
    QString deviceName = this->getConfigParameter(CHardwareSDK::ModelName).toString();

    if (deviceName.isEmpty()) {
        deviceName = this->mDeviceName;
    }

    this->toLog(LogLevel::Normal, QStringLiteral("**********************************************************"));
    this->toLog(LogLevel::Normal,
                QStringLiteral("Initializing device: %1. Version: %2.").arg(deviceName).arg(this->mVersion));
    this->toLog(LogLevel::Normal, QStringLiteral("**********************************************************"));

    this->mInitialized = ERequestStatus::InProcess;
    this->mInitializationError = false;

    // mResourceMutex должен вызываться через this-> если он в базовом шаблоне
    MutexLocker resourceLocker(&this->mResourceMutex);

    if (this->checkConnectionAbility()) {
        if (this->isPowerReboot() || !this->mConnected) {
            this->checkExistence();
        }

        if (this->mConnected) {
            this->emitStatusCode(DeviceStatusCode::OK::Initialization, EStatus::Interface);
            this->mStatusCollection.clear();

            int count = 0;
            bool isCriticalError = false;

            do {
                if (count > 0) {
                    this->toLog(LogLevel::Normal, QStringLiteral("Try to repeat initialization #%1.").arg(count + 1));
                }

                MutexLocker externalLocker(&this->mExternalMutex);

                this->setInitialData();

                if (this->updateParameters()) {
                    break;
                }

                int errorSize = this->mStatusCollection.size(EWarningLevel::Error);
                isCriticalError =
                    (errorSize > 1) ||
                    ((errorSize == 1) && !this->mStatusCollection.contains(DeviceStatusCode::Error::Initialization));
            } while ((++count < this->mInitializeRepeatCount) && !isCriticalError);

            this->mInitialized = (!isCriticalError && (count < this->mInitializeRepeatCount)) ? ERequestStatus::Success
                                                                                              : ERequestStatus::Fail;

            if (this->mInitialized == ERequestStatus::Fail) {
                this->toLog(LogLevel::Error,
                            isCriticalError
                                ? QStringLiteral("Initialization was broken due to critical error.")
                                : QStringLiteral("The maximum quantity of initialization attempts is exceeded."));
            }
        } else {
            this->mInitialized = ERequestStatus::Fail;
            this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
        }

        this->finalizeInitialization();
    } else {
        this->mConnected = false;
        this->mInitialized = ERequestStatus::Fail;
        this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
    }

    // Оптимизация формирования строки данных плагина
    QString pluginPath = QStringLiteral("\n%1 : %2")
                             .arg(CHardware::PluginPath)
                             .arg(this->getConfigParameter(CHardware::PluginPath).toString());

    SLogData logData = this->getDeviceData();
    this->setConfigParameter(CHardwareSDK::DeviceData,
                             pluginPath + logData.plugin + logData.device + logData.config + logData.requiredDevice);
    this->logDeviceData(logData);
    this->removeConfigParameter(CHardware::CallingType);

    if (this->mInitialized == ERequestStatus::Success) {
        this->toLog(LogLevel::Normal, QStringLiteral("Device %1 is initialized.").arg(this->mDeviceName));

        emit(this->initialized());
    } else {
        this->toLog(LogLevel::Error, QStringLiteral("Failed to initialize %1.").arg(this->mDeviceName));
    }
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::finalizeInitialization() {
    if (!this->mConnected) {
        this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
    }

    Qt::ConnectionType connectionType = !this->isWorkingThread() ? Qt::BlockingQueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "onPoll", connectionType);
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::release() {
    bool result = this->release();

    this->mConnected = false;
    this->mLastWarningLevel = static_cast<EWarningLevel::Enum>(-1);
    mStatusCollection.clear();

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::isPluginMismatch() {
    if (!this->containsConfigParameter(CPluginParameters::PPVersion)) {
        return false;
    }

    QString version = this->getConfigParameter(CPluginParameters::PPVersion).toString();
    auto trimBuild = [&](const QString &aVersion) -> QString { return aVersion.split(" build ").first().simplified(); };

    return trimBuild(version) != trimBuild(this->mVersion);
}

//---------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::isInitializationError(TStatusCodes &aStatusCodes) {
    bool powerTurnOn = !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable) &&
                       this->mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);

    return (this->mInitialized == ERequestStatus::Fail) && !powerTurnOn;
    /*
    TODO: рассмотреть вариант формирования ошибки инициализации только когда нет и не было ошибки NotAvailable, т.е. -
    return (mInitialized == ERequestStatus::Fail) && !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable) &&
    !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
    */
}

//---------------------------------------------------------------------------
template <class T> void DeviceBase<T>::cleanStatusCodes(TStatusCodes &aStatusCodes) {
    if (this->isInitializationError(aStatusCodes) || this->mInitializationError) {
        aStatusCodes.insert(DeviceStatusCode::Error::Initialization);
    }

    if (aStatusCodes.contains(DeviceStatusCode::Error::Initialization)) {
        aStatusCodes.remove(DeviceStatusCode::OK::Initialization);
        aStatusCodes.remove(DeviceStatusCode::OK::Busy);
    }

    if (aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable)) {
        if (aStatusCodes.size() > 1) {
            aStatusCodes.clear();
            aStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);
        }

        this->mNeedReboot = false;
        this->mInitializationError = false;
    }

    if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::Error::ThirdPartyDriverFail))) {
        aStatusCodes.clear();
        aStatusCodes.insert(DeviceStatusCode::Error::ThirdPartyDriverFail);
    }

    if (mOldFirmware) {
        aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
    }

    if (mNeedReboot) {
        aStatusCodes.insert(DeviceStatusCode::Warning::NeedReboot);
    }

    if (!mVerified) {
        aStatusCodes.insert(DeviceStatusCode::Warning::ModelNotVerified);
    }

    if (!mModelCompatibility) {
        aStatusCodes.insert(DeviceStatusCode::Warning::ModelNotCompatible);
    }

    if (!this->mOperatorPresence && isPluginMismatch()) {
        aStatusCodes.insert(DeviceStatusCode::Warning::Compatibility);
    }

    if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::OK::OK))) {
        aStatusCodes.remove(DeviceStatusCode::OK::OK);
    }

    TStatusCollection statusCollection = getStatusCollection(aStatusCodes);

    if ((statusCollection.size(EWarningLevel::OK) > 1) && (aStatusCodes.contains(DeviceStatusCode::OK::Unknown))) {
        aStatusCodes.remove(DeviceStatusCode::OK::Unknown);
    }

    if ((aStatusCodes.size() > 1) && (aStatusCodes.contains(DeviceStatusCode::Warning::OperationError))) {
        aStatusCodes.remove(DeviceStatusCode::Warning::OperationError);
    }

    if ((statusCollection.size(EWarningLevel::Error) > 1) &&
        (aStatusCodes.contains(DeviceStatusCode::Error::Unknown))) {
        aStatusCodes.remove(DeviceStatusCode::Error::Unknown);
    }
}

//---------------------------------------------------------------------------
template <class T> void DeviceBase<T>::recoverErrors(TStatusCodes &aStatusCodes) {
    TStatusCodes recoverableErrors = aStatusCodes & mRecoverableErrors;

    if ((mUnsafeStatusCodes & aStatusCodes).isEmpty() && mStatusCollection.contains(EWarningLevel::Error) &&
        !recoverableErrors.isEmpty()) {
        TStatusCodes oldErrors = mStatusCollection.value(EWarningLevel::Error);
        TStatusCodes oldRecoverableErrors = oldErrors & mRecoverableErrors;
        TStatusCodes otherStatusCodes = aStatusCodes - mRecoverableErrors;
        bool effectiveErrors = std::find_if(otherStatusCodes.begin(), otherStatusCodes.end(), [&](int aCode) -> bool {
                                   return mStatusCodesSpecification->value(aCode).warningLevel == EWarningLevel::Error;
                               }) != otherStatusCodes.end();
        TStatusCodes oldUnsafeStatusCodes = getStatusCodes(mStatusCollection) & mUnsafeStatusCodes;

        if (((oldErrors + oldUnsafeStatusCodes).size() > oldRecoverableErrors.size()) && !effectiveErrors) {
            foreach (int statusCode, mRecoverableErrors) {
                if (mStatusCodesSpecification->value(statusCode).warningLevel == EWarningLevel::Error) {
                    aStatusCodes.remove(statusCode);
                }
            }
        }
    }

    if (aStatusCodes.isEmpty()) {
        aStatusCodes.insert(DeviceStatusCode::OK::OK);
    }
}

//---------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::isStatusesReplaceable(TStatusCodes &aStatusCodes) {
    TStatusCodes errors = mStatusCollection.value(EWarningLevel::Error);

    return std::find_if(mReplaceableStatuses.begin(), mReplaceableStatuses.end(), [&](int aStatusCode) -> bool {
               return aStatusCodes.contains(aStatusCode) && !errors.contains(aStatusCode);
           }) != mReplaceableStatuses.end();
}

//---------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::canApplyStatusBuffer() {
    return mMaxBadAnswers && (mForceStatusBufferEnabled || (!this->mOperatorPresence && this->mPostPollingAction)) &&
           !getStatusCodes(mStatusCollection).isEmpty() &&
           !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T> void DeviceBase<T>::applyStatusBuffer(TStatusCodes &aStatusCodes) {
    // задействуем буфер статусов
    if (canApplyStatusBuffer() && isStatusesReplaceable(aStatusCodes)) {
        if (mBadAnswerCounter <= mMaxBadAnswers) {
            ++mBadAnswerCounter;
        }

        if (mBadAnswerCounter <= mMaxBadAnswers) {
            aStatusCodes = getStatusCodes(mStatusCollection);
            QStringList descriptions;

            foreach (int statusCode, aStatusCodes) {
                descriptions << mStatusCodesSpecification->value(statusCode).description;
            }

            this->toLog(LogLevel::Error, QString("%1: bad answer counter = %2 of %3, return previous statuses: %4")
                                             .arg(this->mDeviceName)
                                             .arg(this->mBadAnswerCounter)
                                             .arg(this->mMaxBadAnswers)
                                             .arg(descriptions.join(", ")));
        }
    } else {
        this->mBadAnswerCounter = 0;
    }
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::waitReady(const SWaitingData &aWaitingData, bool aReady) {
    TStatusCodes statusCodes;
    auto poll = [&]() -> bool {
        statusCodes.clear();
        return aReady ==
               (getStatus(std::ref(statusCodes)) && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable));
    };

    return PollingExpector().wait(poll, aWaitingData);
}

//---------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::getStatus(TStatusCodes & /*aStatuses*/) {
    return this->isConnected();
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::simplePoll() {
    bool doPostPollingAction = false;

    qSwap(this->mPostPollingAction, doPostPollingAction);
    this->onPoll();
    qSwap(this->mPostPollingAction, doPostPollingAction);
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::onPoll() {
    TStatusCodes statusCodes;
    this->doPoll(statusCodes);

    MutexLocker locker(&this->mResourceMutex);

    this->processStatusCodes(statusCodes);
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::processStatus(TStatusCodes &aStatusCodes) {
    return this->getStatus(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::doPoll(TStatusCodes &aStatusCodes) {
    {
        MutexLocker locker(&this->mExternalMutex);

        QDate currentDate = QDate::currentDate();

        if (this->mLogDate.day() != currentDate.day()) {
            this->mLogDate = currentDate;
            this->logDeviceData(this->getDeviceData());
        }

        aStatusCodes.clear();
        int resultStatus =
            this->processStatus(aStatusCodes) ? DeviceStatusCode::OK::OK : DeviceStatusCode::Error::NotAvailable;
        aStatusCodes.insert(resultStatus);
    }

    MutexLocker locker(&this->mResourceMutex);

    this->cleanStatusCodes(aStatusCodes);
    this->recoverErrors(aStatusCodes);
    this->applyStatusBuffer(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T> SStatusCodeSpecification DeviceBase<T>::getStatusCodeSpecification(int aStatusCode) const {
    return this->mStatusCodesSpecification->value(aStatusCode);
}

//--------------------------------------------------------------------------------
template <class T> QString DeviceBase<T>::getStatusTranslations(const TStatusCodes &aStatusCodes, bool aLocale) const {
    // 1. В Qt 6 метод .toList() у QSet/QSet-подобных контейнеров удален.
    // Используем конструктор от итераторов для создания списка.
    QStringList translations;
    TStatusCodesBuffer statusCodesBuffer(aStatusCodes.begin(), aStatusCodes.end());

    // 2. В Qt 6 макрос qSort удален. Используем стандартный std::sort.
    std::sort(statusCodesBuffer.begin(), statusCodesBuffer.end());

    // 3. Заменяем устаревший foreach на стандартный range-based for.
    for (int statusCode : statusCodesBuffer) {
        // 4. Используем this-> для обращения к методу базового шаблонного класса (обязательно для Clang/macOS).
        SStatusCodeSpecification codeSpecification = this->getStatusCodeSpecification(statusCode);

        translations << (aLocale ? codeSpecification.translation : codeSpecification.description);
    }

    // 5. Используем QStringLiteral для разделителя (оптимизация 2026 года).
    return translations.join(CDevice::StatusSeparator);
    // Примечание: если CDevice::StatusSeparator - статическая константа,
    // можно оставить CDevice::StatusSeparator.
}

//--------------------------------------------------------------------------------
template <class T> TStatusCodes DeviceBase<T>::getStatusCodes(const TStatusCollection &aStatusCollection) {
    return aStatusCollection[EWarningLevel::Error] + aStatusCollection[EWarningLevel::Warning] +
           aStatusCollection[EWarningLevel::OK];
}

//--------------------------------------------------------------------------------
template <class T>
TStatusCollection DeviceBase<T>::getStatusCollection(const TStatusCodes &aStatusCodes,
                                                     TStatusCodeSpecification *aStatusCodeSpecification) {
    TStatusCodeSpecification *statusCodeSpecification =
        aStatusCodeSpecification ? aStatusCodeSpecification : this->mStatusCodesSpecification.data();
    TStatusCollection result;

    foreach (int statusCode, aStatusCodes) {
        EWarningLevel::Enum warningLevel = statusCodeSpecification->value(statusCode).warningLevel;
        result[warningLevel].insert(statusCode);
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> LogLevel::Enum DeviceBase<T>::getLogLevel(EWarningLevel::Enum aLevel) {
    return (aLevel == EWarningLevel::OK) ? LogLevel::Normal
                                         : ((aLevel == EWarningLevel::Error) ? LogLevel::Error : LogLevel::Warning);
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::sendStatuses(const TStatusCollection &aNewStatusCollection,
                                 const TStatusCollection &aOldStatusCollection) {
    TStatusCollection newStatusCollection(aNewStatusCollection);

    TStatusCollection oldStatusCollection(aOldStatusCollection);
    oldStatusCollection[EWarningLevel::OK];
    oldStatusCollection[EWarningLevel::Warning];
    oldStatusCollection[EWarningLevel::Error];

    auto removeExcessStatusCodes = [&](TStatusCollection &aStatusCollection) {
        for (int level = EWarningLevel::OK; level <= EWarningLevel::Error; ++level) {
            EWarningLevel::Enum warningLevel = static_cast<EWarningLevel::Enum>(level);
            aStatusCollection[warningLevel] =
                aStatusCollection[warningLevel] - this->mExcessStatusCollection[warningLevel];
        }
    };

    EWarningLevel::Enum warningLevel = this->getWarningLevel(newStatusCollection);
    removeExcessStatusCodes(newStatusCollection);

    TStatusCollection allOldStatusCollection(oldStatusCollection);
    removeExcessStatusCodes(oldStatusCollection);

    if (this->getStatusCodes(newStatusCollection).isEmpty()) {
        newStatusCollection[EWarningLevel::OK].insert(DeviceStatusCode::OK::OK);
    }

    if (this->getStatusCodes(oldStatusCollection).isEmpty() &&
        !this->getStatusCodes(allOldStatusCollection).isEmpty()) {
        oldStatusCollection[EWarningLevel::OK].insert(DeviceStatusCode::OK::OK);
    }

    if (this->environmentChanged() ||
        ((newStatusCollection != oldStatusCollection) && (aNewStatusCollection != aOldStatusCollection)) ||
        (warningLevel != this->mLastWarningLevel)) {
        this->emitStatusCodes(newStatusCollection);
    }
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::emitStatusCode(int aStatusCode, int aExtendedStatus) {
    EWarningLevel::Enum warningLevel = this->mStatusCodesSpecification->value(aStatusCode).warningLevel;

    if (aExtendedStatus < EStatus::Service) {
        this->mLastWarningLevel = warningLevel;
    }

    QString translation = this->getStatusTranslations(TStatusCodes() << aStatusCode, true);
    this->toLog(this->getLogLevel(warningLevel),
                QString("Send statuses: %1, extended status %2").arg(translation).arg(aExtendedStatus));

    emit(this->status(warningLevel, translation, aExtendedStatus));
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::emitStatusCodes(TStatusCollection &aStatusCollection, int aExtendedStatus) {
    TStatusCodes statusCodes = this->getStatusCodes(aStatusCollection);
    EWarningLevel::Enum warningLevel = this->getWarningLevel(aStatusCollection);

    if (aExtendedStatus < EStatus::Service) {
        mLastWarningLevel = warningLevel;
    }

    QString translation = this->getStatusTranslations(statusCodes, true);
    this->toLog(this->getLogLevel(warningLevel),
                QString("Send statuses: %1, extended status %2").arg(translation).arg(aExtendedStatus));

    emit this->status(warningLevel, translation, aExtendedStatus);
}

//--------------------------------------------------------------------------------
template <class T> EWarningLevel::Enum DeviceBase<T>::getWarningLevel(const TStatusCollection &aStatusCollection) {
    return aStatusCollection[EWarningLevel::Error].size()
               ? EWarningLevel::Error
               : (aStatusCollection[EWarningLevel::Warning].size() ? EWarningLevel::Warning : EWarningLevel::OK);
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::reInitialize() {
    // 1. Используем this-> для обращения к членам базового шаблонного класса (обязательно для Clang/macOS).
    // 2. Используем QStringLiteral для оптимизации памяти.
    this->setConfigParameter(CHardware::CallingType, CHardware::CallingTypes::Internal);

    if (this->mOperatorPresence) {
        this->initialize();
    } else {
        // 3. В Qt 6 рекомендуется использовать синтаксис указателей на методы для QMetaObject::invokeMethod.
        // Это предотвращает ошибки в рантайме и работает быстрее, чем строковый вызов.
        QMetaObject::invokeMethod(this, &DeviceBase<T>::initialize, Qt::QueuedConnection);
    }
}

//--------------------------------------------------------------------------------
template <class T>
void DeviceBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                      const TStatusCollection &aOldStatusCollection) {
    bool powerTurnOn = aOldStatusCollection.contains(DeviceStatusCode::Error::NotAvailable) &&
                       !aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
    bool containsNewErrors = !aNewStatusCollection.isEmpty(EWarningLevel::Error);
    bool containsOldErrors = !aOldStatusCollection.isEmpty(EWarningLevel::Error);

    if ((powerTurnOn && containsNewErrors) || (containsOldErrors && !containsNewErrors)) {
        QString log = QString("Exit from %1, trying to re-identify").arg(powerTurnOn ? "turning off" : "error");

        if (!containsNewErrors) {
            log += " and to re-initialize";
        }

        this->toLog(LogLevel::Normal, log);

        this->mOperatorPresence ? this->checkExistence()
                                : QMetaObject::invokeMethod(this, "checkExistence", Qt::QueuedConnection);

        if (!containsNewErrors) {
            this->reInitialize();
        }
    }
}

//--------------------------------------------------------------------------------
template <class T>
QString DeviceBase<T>::getTrOfNewProcessed(const TStatusCollection &aStatusCollection,
                                           EWarningLevel::Enum aWarningLevel) {
    return this->getStatusTranslations(aStatusCollection[aWarningLevel], false);
}

//--------------------------------------------------------------------------------
template <class T> void DeviceBase<T>::processStatusCodes(const TStatusCodes &aStatusCodes) {
    TStatusCollection newStatusCollection = this->getStatusCollection(aStatusCodes);
    newStatusCollection[EWarningLevel::OK];
    newStatusCollection[EWarningLevel::Warning];
    newStatusCollection[EWarningLevel::Error];

    if ((newStatusCollection != this->mStatusCollection) || this->environmentChanged()) {
        QString errorsLog = this->getTrOfNewProcessed(newStatusCollection, EWarningLevel::Error);
        QString warningsLog = this->getTrOfNewProcessed(newStatusCollection, EWarningLevel::Warning);
        QString normalsLog = this->getTrOfNewProcessed(newStatusCollection, EWarningLevel::OK);

        if ((errorsLog + warningsLog + normalsLog).size()) {
            this->toLog(LogLevel::Normal, "Status changed:");
        }

        this->mLog->adjustPadding(1);

        if (!errorsLog.isEmpty())
            this->toLog(LogLevel::Error, "Errors   : " + errorsLog);
        if (!warningsLog.isEmpty())
            this->toLog(LogLevel::Warning, "Warnings : " + warningsLog);
        if (!normalsLog.isEmpty())
            this->toLog(LogLevel::Normal, "Normal   : " + normalsLog);

        this->mLog->adjustPadding(-1);
        TStatusCollection lastStatusCollection = this->mStatusCollectionHistory.lastValue();

        if (lastStatusCollection != newStatusCollection) {
            this->mStatusCollectionHistory.append(newStatusCollection);
        }

        QString debugLog = "Status codes history :";

        for (int i = 0; i < this->mStatusCollectionHistory.size(); ++i) {
            TStatusCollection statusCollection = this->mStatusCollectionHistory[i];
            QString statusLog;

#define DEBUG_STATUS(aWarningLevel)                                                                                    \
    QString debug##aWarningLevel##Log =                                                                                \
        this->getStatusTranslations(statusCollection[EWarningLevel::aWarningLevel], false);                            \
    QString name##aWarningLevel = #aWarningLevel;                                                                      \
    name##aWarningLevel += QString(8 - name##aWarningLevel.size(), QChar(' '));                                        \
    if (!debug##aWarningLevel##Log.isEmpty())                                                                          \
        statusLog += QString("%1%2 : %3")                                                                              \
                         .arg(statusLog.isEmpty() ? "" : "\n       ")                                                  \
                         .arg(name##aWarningLevel)                                                                     \
                         .arg(debug##aWarningLevel##Log);

            DEBUG_STATUS(Error);
            DEBUG_STATUS(Warning);
            DEBUG_STATUS(OK);

            debugLog += QString("\n [%1] : %2").arg(i).arg(statusLog);
        }

        this->toLog(LogLevel::Debug, debugLog);
    }

    TStatusCollection oldStatusCollection(this->mStatusCollection);
    this->mStatusCollection = newStatusCollection;
    this->sendStatuses(newStatusCollection, oldStatusCollection);

    if (this->mPostPollingAction) {
        this->postPollingAction(newStatusCollection, oldStatusCollection);
    }

    this->mConnected = !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);

    if (this->mPostPollingAction && (this->mInitialized != ERequestStatus::InProcess)) {
        this->mInitialized = (this->mConnected && (this->mInitialized == ERequestStatus::Success))
                                 ? ERequestStatus::Success
                                 : ERequestStatus::Fail;
    }
}

//--------------------------------------------------------------------------------
template <class T> bool DeviceBase<T>::find() {
    if (this->checkExistence()) {
        return true;
    }

    this->release();

    return false;
}

//--------------------------------------------------------------------------------
