/* @file Базовый класс устройств приема денег на порту. */

// STL
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/CashAcceptors/CashAcceptorData.h"
#include <Hardware/Common/LoggingType.h>

// Project
#include "PortCashAcceptor.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T> PortCashAcceptor<T>::PortCashAcceptor() {
    // данные устройства
    this->mDeviceName = "Port cash acceptor";
    this->mCheckDisable = false;
    this->mResetOnIdentification = false;
    this->mIOMessageLogging = ELoggingType::None;
    this->mParInStacked = false;
    this->mUpdatable = false;
    this->mEscrowPosition = -1;
    this->mMaxBadAnswers = 4;
    this->mResetWaiting = EResetWaiting::No;
    this->mPollingIntervalEnabled = CCashAcceptorsPollingInterval::Enabled;
    this->mPollingIntervalDisabled = CCashAcceptorsPollingInterval::Disabled;
    this->mForceWaitResetCompleting = false;

    // описания для кодов статусов
    setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, 0);
    setConfigParameter(CHardware::CashAcceptor::StackedFilter, false);

    setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCashAcceptor::Timeout::ExitInitialize);

    // Устанавливаем начальные параметры.
    setInitialData();
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::setInitialData() {
    this->mCurrencyError = ECurrencyError::OK;
    this->mPollingInterval = getPollingInterval(false);
    this->mReady = false;

    // инициализация состояний
    setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
    setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::processStatus(TStatusCodes &aStatusCodes) {
    if (!T::processStatus(aStatusCodes)) {
        return false;
    }

    if (this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool() &&
        !this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool()) {
        if (this->mBadAnswerCounter && this->getStatusCollection(aStatusCodes)[EWarningLevel::Error].isEmpty()) {
            this->reenableMoneyAcceptingMode();
            this->applyParTable();
        }

        // TODO: только если будет замечено, что обмен не прерывался, но внутренние регистры сбросились.
        // applyParTable();
    }

    return true;
}

//---------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::checkStatuses(TStatusData &aData) {
    QByteArray answer;

    if (!checkStatus(answer) || answer.isEmpty()) {
        return false;
    }

    aData = TStatusData() << answer;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::getStatus(TStatusCodes &aStatusCodes) {
    TDeviceCodeBuffers lastDeviceCodeBuffers(mDeviceCodeBuffers);
    this->mDeviceCodeBuffers.clear();

    TStatusData statusData;

    if (!checkStatuses(statusData)) {
        return false;
    }

    TDeviceCodeSpecifications deviceCodeSpecifications;
    QByteArray parData;

    foreach (auto answerData, statusData) {
        QSet<QString> lastData(deviceCodeSpecifications.keys().begin(), deviceCodeSpecifications.keys().end());
        this->mDeviceCodeSpecification->getSpecification(answerData, deviceCodeSpecifications);
        QSet<QString> newData(deviceCodeSpecifications.keys().begin(), deviceCodeSpecifications.keys().end()) -
            lastData;

        this->mDeviceCodeBuffers << answerData;

        foreach (auto data, newData) {
            // данные о валюте
            int statusCode = deviceCodeSpecifications[data].statusCode;
            bool escrow = statusCode == BillAcceptorStatusCode::BillOperation::Escrow;
            bool stacked = statusCode == BillAcceptorStatusCode::BillOperation::Stacked;

            if (escrow || (stacked && this->mParInStacked)) {
                parData = answerData;
            }
        }
    }

    QStringList defaultDescriptionLog;
    LogLevel::Enum logLevel = LogLevel::Normal;

    foreach (const SDeviceCodeSpecification &specification, deviceCodeSpecifications) {
        aStatusCodes.insert(specification.statusCode);
        defaultDescriptionLog << specification.description;
        EWarningLevel::Enum warningLevel =
            this->mStatusCodesSpecification->value(specification.statusCode).warningLevel;
        logLevel = qMin(logLevel, getLogLevel(warningLevel));
    }

    if (defaultDescriptionLog.isEmpty()) {
        defaultDescriptionLog << "default";
    }

    if (mDeviceCodeBuffers != lastDeviceCodeBuffers) {
        if (mDeviceCodeBuffers.isEmpty()) {
            toLog(logLevel,
                  this->mDeviceName +
                      QString(": %1 -> %2").arg(UnknownDeviceCodeDescription).arg(defaultDescriptionLog.join(", ")));
        } else {
            for (TDeviceCodeSpecifications::iterator it = deviceCodeSpecifications.begin();
                 it != deviceCodeSpecifications.end(); ++it) {
                if (!it->description.isEmpty()) {
                    SStatusCodeSpecification statusCodeData = this->mStatusCodesSpecification->value(it->statusCode);
                    logLevel = getLogLevel(statusCodeData.warningLevel);

                    QString codeLog;

                    if (it->description == UnknownDeviceCodeDescription) {
                        codeLog = QString(" (%1)").arg(it.key());
                    }

                    toLog(
                        logLevel,
                        this->mDeviceName +
                            QString(": %1%2 -> %3").arg(it->description).arg(codeLog).arg(statusCodeData.description));
                }
            }
        }
    }

    // валюта
    bool escrow = aStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow);
    bool stacked = aStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Stacked);

    if (!escrow && !(stacked && this->mParInStacked)) {
        return true;
    }

    if (!setLastPar(parData) && escrow) {
        reject();

        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);
        aStatusCodes.insert(BillAcceptorStatusCode::Normal::Enabled);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::setLastPar(const QByteArray &aAnswer) {
    QString log = this->mDeviceName + ": Failed to set last par due to ";

    if ((mEscrowPosition < 0) || (mEscrowPosition >= aAnswer.size())) {
        toLog(LogLevel::Error,
              log + QString("wrong escrow position = %1 (answer size = %2)").arg(mEscrowPosition).arg(aAnswer.size()));
        return false;
    }

    int escrowKey = uchar(aAnswer[mEscrowPosition]);

    if (!mEscrowParTable.data().contains(escrowKey)) {
        toLog(LogLevel::Error, log + "wrong escrow key = " + ProtocolUtils::toHexLog<uchar>(uchar(escrowKey)));
        return false;
    }

    this->mEscrowPars = TPars() << this->mEscrowParTable[aAnswer[mEscrowPosition]];
    SPar par = this->mEscrowPars[0];

    if (!par.nominal) {
        toLog(LogLevel::Error, log + "nominal == 0");
        return false;
    }

    if (!CurrencyCodes.data().values().contains(par.currencyId)) {
        toLog(LogLevel::Error, log + "unknown currency, id = " + QString::number(par.currencyId));
        return false;
    }

    if (!par.enabled) {
        toLog(LogLevel::Error, log + "par isn`t enabled");
        return false;
    }

    if (par.inhibit) {
        toLog(LogLevel::Error, log + "par is inhibit");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::restoreStatuses() {
    START_IN_WORKING_THREAD(restoreStatuses)

    toLog(LogLevel::Normal,
          this->mDeviceName +
              QString(", restoreStatuses: post polling action is %1enabled, checking disable state is %2enabled")
                  .arg(mPostPollingAction ? "" : "not ")
                  .arg(mCheckDisable ? "" : "not "));

    auto canRestore = [&]() -> bool { return !mCheckDisable && this->mPostPollingAction; };
    TStatusHistory statusHistory;

    if (mPollingActive && waitCondition(canRestore, CPortCashAcceptor::RestoreStatusesWaiting) &&
        !mStatusHistory.isEmpty()) {
        EWarningLevel::Enum warningLevel = this->mStatusHistory.lastValue().warningLevel;

        for (auto it = this->mStatusHistory.begin() + this->mStatusHistory.getLevel(); it < this->mStatusHistory.end();
             ++it) {
            if (it->warningLevel <= warningLevel) {
                // перепаковываем статусы
                TStatusCodes commonStatusCodes;

                foreach (const TStatusCodes &statusCodes, it->statuses) {
                    commonStatusCodes.unite(statusCodes);
                }

                cleanStatusCodes(commonStatusCodes);
                CCashAcceptor::SStatusSpecification resultStatuses;

                foreach (int statusCode, commonStatusCodes) {
                    ECashAcceptorStatus::Enum status =
                        static_cast<ECashAcceptorStatus::Enum>(mStatusCodesSpecification->value(statusCode).status);

                    if (CCashAcceptor::Set::MainStatuses.contains(status) && (status != ECashAcceptorStatus::Escrow) &&
                        (status != ECashAcceptorStatus::Stacked)) {
                        resultStatuses.statuses[status].insert(statusCode);
                        resultStatuses.warningLevel = qMax(
                            resultStatuses.warningLevel, this->mStatusCodesSpecification->warningLevelByStatus(status));
                    }
                }

                if (statusHistory.isEmpty() || (statusHistory.lastValue() != resultStatuses)) {
                    statusHistory.append(resultStatuses);
                }
            }
        }
    }

    for (auto it = statusHistory.begin(); it < statusHistory.end(); ++it) {
        emitStatuses(*it, CCashAcceptor::Set::MainStatuses);
    }
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::onSendDisabled() {
    toLog(LogLevel::Normal, QString("Send statuses: %1 disabled").arg(mDeviceType));

    emit status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::sendEnabled() {
    toLog(LogLevel::Normal, QString("Send statuses: %1 enabled").arg(mDeviceType));

    emit status(EWarningLevel::OK, "Enabled", ECashAcceptorStatus::Enabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::setEnable(bool aEnabled) {
    toLog(LogLevel::Normal, QString("%1: %2").arg(mDeviceName).arg(aEnabled ? "Enable" : "Disable"));

    if (mInitialized != ERequestStatus::Success) {
        if (aEnabled)
            toLog(LogLevel::Error, this->mDeviceName + ": on set enable(true) not initialized, return false");
        else
            toLog(LogLevel::Normal, this->mDeviceName + ": on set enable(false) not initialized, return true");

        if (!aEnabled) {
            toLog(LogLevel::Normal, QString("Send statuses: %1 disabled").arg(mDeviceName));

            emit status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);
        }

        return !aEnabled;
    }

    if (!checkConnectionAbility()) {
        return !aEnabled;
    }

    setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
    setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);

    if ((aEnabled && isEnabled()) || (!aEnabled && isDisabled())) {
        toLog(LogLevel::Normal, this->mDeviceName + QString(": already %1").arg(aEnabled ? "enabled" : "disabled"));

        setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);

        if (!aEnabled) {
            this->mCheckDisable = false;
        }

        setPollingInterval(getPollingInterval(aEnabled));
        startPolling();

        // TODO: откат отложенного включения купюрника
        aEnabled ? /*sendEnabled()*/ restoreStatuses() : onSendDisabled();

        return true;
    }

    setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, aEnabled);
    setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, !aEnabled);

    if (!aEnabled && !mPostPollingAction && !mCheckDisable) {
        toLog(LogLevel::Normal, this->mDeviceName + ": The process has started already");
        return true;
    }

    this->mPostPollingAction = false;

    stopPolling();

    auto cancelSetEnable = [&]() -> bool {
        toLog(LogLevel::Normal, this->mDeviceName + (aEnabled ? ": An error is occured, Enable return false"
                                                              : ": An error is occured, Disable return true"));

        setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
        setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);
        setConfigParameter(CHardware::CashAcceptor::Enabled, false);

        this->mPostPollingAction = true;

        setPollingInterval(getPollingInterval(false));
        startPolling(true);

        // TODO: откат отложенного включения купюрника
        aEnabled ? /*sendEnabled()*/ restoreStatuses() : onSendDisabled();

        return !aEnabled;
    };

    if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
        return cancelSetEnable();
    }

    if (!aEnabled) {
        bool canReturn = canReturning(false);

        if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
            return cancelSetEnable();
        }

        if (canReturn) {
            PollingExpector expector;
            auto notAccepting = [&]() -> bool {
                return !mStatusCollection.contains(BillAcceptorStatusCode::BillOperation::Accepting);
            };
            expector.wait<void>(std::bind(&CashAcceptorBase<T>::simplePoll, this), notAccepting,
                                getPollingInterval(true), CCashAcceptor::Timeout::Escrow);

            canReturn = canReturning(true);

            if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
                return cancelSetEnable();
            }

            if (canReturn) {
                auto notEscrow = [&]() -> bool {
                    return !mStatusCollection.contains(BillAcceptorStatusCode::BillOperation::Escrow);
                };
                processAndWait(std::bind(&PortCashAcceptor<T>::reject, this), notEscrow, CCashAcceptor::Timeout::Return,
                               false);
            }
        }
    }

    if (!aEnabled && !canDisable() && !mCheckDisable) {
        // вариант с параметризованным отключением купюрника
        // bool deferred = aDeferred || (containsConfigParameter(CHardware::CashAcceptor::OnlyDefferedDisable) &&
        // getConfigParameter(CHardware::CashAcceptor::OnlyDefferedDisable).toBool());
        bool deferred = true;

        if (!mCheckDisable) {
            if (deferred) {
                this->mCheckDisable = true;
                toLog(LogLevel::Normal,
                      this->mDeviceName + ": deferred disabling logic is activated, disable return true");
            } else {
                toLog(LogLevel::Normal,
                      this->mDeviceName + ": Failed to disable due to operation with note, disable return false");
            }
        }

        this->mPostPollingAction = true;

        startPolling(true);
        restoreStatuses();

        return deferred;
    }

    if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
        return cancelSetEnable();
    }

    // TODO: эмитить сигнал только если нахождимся не в рабочем потоке
    processEnable(aEnabled);

    // TODO: откат отложенного включения купюрника
    return !aEnabled || isNotDisabled();
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::processEnable(bool aEnabled) {
    // TODO: откат отложенного включения купюрника
    if (!aEnabled && !isWorkingThread()) {
        QMetaObject::invokeMethod(this, "processEnable", Qt::QueuedConnection, Q_ARG(bool, aEnabled));

        return;
    }

    auto setEnableWaitPostAction = [&]() -> bool {
        bool result = aEnabled ? isNotDisabled() : isNotEnabled();

        if (result) {
            this->mCheckDisable = false;

            setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);
            setConfigParameter(
                aEnabled ? CHardware::CashAcceptor::ProcessEnabling : CHardware::CashAcceptor::ProcessDisabling, false);

            setPollingInterval(getPollingInterval(aEnabled));

            if (aEnabled) {
                logEnabledPars();
            }
        }

        return result;
    };

    bool needReset = true;

    if (!mStatusHistory.isEmpty()) {
        TStatusCollection lastStatusCollection = this->mStatusCollectionHistory.lastValue();
        TStatusCollection beforeLastStatusCollection = this->mStatusCollectionHistory.lastValue(2);
        CCashAcceptor::TStatuses lastStatuses = getLastStatuses(1);
        CCashAcceptor::TStatuses beforeLastStatuses = getLastStatuses(2);

        needReset = (!lastStatuses.isEmpty(ECashAcceptorStatus::Disabled) &&
                     !beforeLastStatuses.isEmpty(ECashAcceptorStatus::Rejected)) ||
                    (lastStatusCollection.contains(DeviceStatusCode::OK::Initialization) &&
                     (beforeLastStatusCollection.contains(BillAcceptorStatusCode::Normal::Disabled) ||
                      isEnabled(beforeLastStatuses))) ||
                    (lastStatusCollection.contains(BillAcceptorStatusCode::Normal::Enabled) && !aEnabled);
    }

    if (aEnabled) {
        applyParTable();
    }

    this->mPostPollingAction = true;
    auto setEnableErrorCondition = [&]() -> bool {
        if (aEnabled)
            return false;
        return !getLastStatuses().isEmpty(ECashAcceptorStatus::BillOperation);
    };

    processAndWait(std::bind(&PortCashAcceptor<T>::enableMoneyAcceptingMode, this, aEnabled), setEnableWaitPostAction,
                   CCashAcceptor::Timeout::SetEnable, needReset, true, setEnableErrorCondition);

    if (aEnabled) {
        applyParTable();
    }

    MutexLocker locker(&mResourceMutex);

    // TODO: откат отложенного включения купюрника
    /*if (aEnabled && isNotDisabled())
    {
            sendEnabled();
    }
    else */
    if (!aEnabled && isNotEnabled()) {
        QTimer::singleShot(getConfigParameter(CHardware::CashAcceptor::DisablingTimeout).toInt(), this,
                           SLOT(onSendDisabled()));
    } else {
        restoreStatuses();
    }
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::reenableMoneyAcceptingMode() {
    bool setEnable = getConfigParameter(CHardware::CashAcceptor::Enabled).toBool();

    if (getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool()) {
        setEnable = false;
    } else if (getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool()) {
        setEnable = true;
    }

    return enableMoneyAcceptingMode(setEnable);
}

//---------------------------------------------------------------------------
template <class T> int PortCashAcceptor<T>::getPollingInterval(bool aEnabled) {
    return aEnabled ? this->mPollingIntervalEnabled : this->mPollingIntervalDisabled;
}

//---------------------------------------------------------------------------
template <class T>
bool PortCashAcceptor<T>::processAndWait(const TBoolMethod &aCommand, TBoolMethod aCondition, int aTimeout,
                                         bool aNeedReset, bool aRestartPolling, TBoolMethod aErrorCondition) {
    int counter = 0;
    bool result = false;

    stopPolling();

    PollingExpector expector;

    do {
        if (counter) {
            toLog(
                LogLevel::Warning,
                this->mDeviceName +
                    QString(": waiting timeout for status change has expired, status is not required, try iteration %1")
                        .arg(counter + 1));

            if (aNeedReset) {
                reset(true);
            }
        }

        if (aCommand()) {
            result = expector.wait<void>(std::bind(&CashAcceptorBase<T>::simplePoll, this), aCondition, aErrorCondition,
                                         getPollingInterval(true), aTimeout);
        } else {
            simplePoll();

            if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
                result = aCondition();
            }
        }
    } while (!result && (++counter < CCashAcceptor::MaxCommandAttempt));

    if (!result) {
        toLog(LogLevel::Warning,
              this->mDeviceName + ((counter >= CCashAcceptor::MaxCommandAttempt)
                                       ? ": max attempts to process the command were exceeded, status is not required"
                                       : ": error was occured while waiting for required device state"));
    }

    if (aRestartPolling) {
        startPolling();
    }

    if (mPostPollingAction) {
        restoreStatuses();
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::isResetCompleted(bool aWait) {
    if (aWait || (mResetWaiting == EResetWaiting::Full)) {
        return isAvailable() && !isInitialize();
    } else if (mResetWaiting == EResetWaiting::Available) {
        return isAvailable();
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::reset(bool aWait) {
    if (!checkConnectionAbility()) {
        return false;
    }

    if (isPowerReboot() && this->mResetOnIdentification) {
        return true;
    }

    auto resetFunction = [&]() -> bool { return processReset(); };
    auto condition = std::bind(&PortCashAcceptor<T>::isResetCompleted, this, aWait);

    int timeout = getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();

    return processAndWait(resetFunction, condition, timeout, false, this->mPollingActive);
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::updateParameters() {
    // TODO: при расширении функционала делать это в базовом классе
    setInitialData();
    processDeviceData();

    if (!reset(false) || !setDefaultParameters()) {
        return false;
    }

    this->mCurrencyError = processParTable();
    ECurrencyError::Enum oldCurrencyError = this->mCurrencyError;

    setEnable(false);

    if (mCurrencyError == ECurrencyError::Loading) {
        this->mCurrencyError = processParTable();
    }

    if (!mPollingInterval) {
        this->mPollingInterval = getPollingInterval(false);
    }

    PollingExpector expector;
    int timeout = getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();
    bool result = expector.wait<void>(
        std::bind(&CashAcceptorBase<T>::simplePoll, this), [&]() -> bool { return !isInitialize(); },
        getPollingInterval(true), timeout);

    if (mCurrencyError == ECurrencyError::Loading) {
        this->mCurrencyError = processParTable();
    }

    if (mCurrencyError != oldCurrencyError) {
        simplePoll();
    }

    return result && (mCurrencyError == ECurrencyError::OK);
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::finalizeInitialization() {
    addPortData();

    if (mPollingActive && (mInitialized == ERequestStatus::Success)) {
        onPoll();
    }

    startPolling(!mConnected);
    this->mStatusHistory.checkLastUnprocessed();
    restoreStatuses();
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canUpdateFirmware() {
    if (!mUpdatable || !isDeviceReady()) {
        return false;
    }

    MutexLocker locker(&mResourceMutex);

    if (isEnabled() && !setEnable(false)) {
        toLog(LogLevel::Error,
              this->mDeviceName +
                  ": Failed to disable for updating the firmware due to incorrect state of cash acceptor");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::updateFirmware(const QByteArray &aBuffer) {
    if (!isWorkingThread()) {
        QMetaObject::invokeMethod(this, "updateFirmware", Qt::QueuedConnection, Q_ARG(const QByteArray &, aBuffer));

        return;
    }

    bool result = performUpdateFirmware(aBuffer);

    if (result) {
        this->mForceWaitResetCompleting = true;
        reset(false);
        this->mForceWaitResetCompleting = false;
    }

    emit updated(result);
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::performUpdateFirmware(const QByteArray & /*aBuffer*/) {
    return false;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canApplyStatusBuffer() {
    bool processEnabling = getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool processDisabling = getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();

    if (processEnabling || (processDisabling && !mCheckDisable)) {
        return true;
    }

    return CashAcceptorBase<T>::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T>
void PortCashAcceptor<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                            const TStatusCollection &aOldStatusCollection) {
    CashAcceptorBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);

    if (mCheckDisable) {
        if (!canDisable()) {
            toLog(LogLevel::Normal,
                  this->mDeviceName + ": Failed to disable due to operation process conditions, waiting...");
            return;
        }

        if (!setEnable(false)) {
            toLog(LogLevel::Error,
                  this->mDeviceName +
                      QString(": Failed to disable due to operation error, checking disable state is %2enabled")
                          .arg(mCheckDisable ? "" : "not "));
            this->mCheckDisable = false;

            return;
        }
    }

    if (!mStatusCollection.isEmpty(EWarningLevel::Error)) {
        this->mCheckDisable = false;
        return;
    }

    bool enabled = getConfigParameter(CHardware::CashAcceptor::Enabled).toBool();
    CCashAcceptor::TStatuses statuses = this->mStatusHistory.lastValue().statuses;

    auto toBool = [&](QVariant arg) -> QString { return (arg.toBool()) ? "true" : "false"; };
    auto toPred = [&](QVariant arg) -> QString { return (arg.toBool()) ? "" : "NOT "; };

    bool checkEnabled = isEnabled();
    bool checkDisabled = isDisabled();
    bool enabling = getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool disabling = getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();
    bool exitFromError =
        !aOldStatusCollection.isEmpty(EWarningLevel::Error) && aNewStatusCollection.isEmpty(EWarningLevel::Error);
    bool beforeRejected = this->mStatusHistory.lastValue(2).statuses.contains(ECashAcceptorStatus::Rejected);

    bool term1 = (enabled || checkEnabled) && disabling;
    bool term2 = (enabled && checkDisabled && !disabling) && !exitFromError;
    bool term3 = (!enabled || checkDisabled) && enabling &&
                 !(beforeRejected && !mStatuses.isEmpty(ECashAcceptorStatus::Disabled));

    bool needSetEnable = term1 || term2 || term3;

    toLog(LogLevel::Debug, this->mDeviceName + QString(": enabled = %1, enable correction is %2needed,\
		\n(1) = %3, isEnabled = %4, enabling = %5, isDisabled = %6, disabling = %7,\
		\n(2) = %8, exit from error = %9,\
		\n(3) = %10, beforeRejected = %11, disabled statuses is %12empty")
                                                   .arg(toBool(enabled))
                                                   .arg(toPred(needSetEnable))
                                                   .arg(toBool(term1))
                                                   .arg(toBool(checkEnabled))
                                                   .arg(toBool(enabling))
                                                   .arg(toBool(checkDisabled))
                                                   .arg(toBool(disabling))
                                                   .arg(toBool(term2))
                                                   .arg(toBool(exitFromError))
                                                   .arg(toBool(term3))
                                                   .arg(toBool(beforeRejected))
                                                   .arg(toPred(mStatuses.isEmpty(ECashAcceptorStatus::Disabled))));

    if (needSetEnable) {
        setEnable(!getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool());
    }
}

//--------------------------------------------------------------------------------
