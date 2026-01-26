/* @file Базовый класс устройств приема денег на порту. */

// STL
#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <Common/QtHeadersEnd.h>

// System
#include <Hardware/CashAcceptors/CashAcceptorData.h>
#include <Hardware/CashAcceptors/PortCashAcceptor.h>
#include <Hardware/Common/LoggingType.h>

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
    this->setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, 0);
    this->setConfigParameter(CHardware::CashAcceptor::StackedFilter, false);

    this->setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCashAcceptor::Timeout::ExitInitialize);

    // Устанавливаем начальные параметры.
    setInitialData();
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::setInitialData() {
    this->mCurrencyError = ECurrencyError::OK;
    this->mPollingInterval = getPollingInterval(false);
    this->mReady = false;

    // инициализация состояний
    this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
    this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);
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

    if (!this->checkStatus(answer) || answer.isEmpty()) {
        return false;
    }

    aData = TStatusData() << answer;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::getStatus(TStatusCodes &aStatusCodes) {
    TDeviceCodeBuffers lastDeviceCodeBuffers(this->mDeviceCodeBuffers);
    this->mDeviceCodeBuffers.clear();

    TStatusData statusData;

    if (!checkStatuses(statusData)) {
        return false;
    }

    TDeviceCodeSpecifications deviceCodeSpecifications;
    QByteArray parData;

    // 1. Заменяем внешний foreach на стандартный цикл
    for (const auto &answerData : statusData) {
        // Получаем ключи спецификаций. В Qt 6 .keys() возвращает QList.
        auto initialKeys = deviceCodeSpecifications.keys();
        QSet<QString> lastData(initialKeys.begin(), initialKeys.end());

        this->mDeviceCodeSpecification->getSpecification(answerData, deviceCodeSpecifications);

        auto updatedKeys = deviceCodeSpecifications.keys();
        QSet<QString> newData(updatedKeys.begin(), updatedKeys.end());

        // 2. В Qt 6 оператор '-' для QSet удален. Используем subtract().
        newData.subtract(lastData);

        this->mDeviceCodeBuffers << answerData;

        // 3. Заменяем внутренний foreach на стандартный цикл
        for (const auto &data : newData) {
            // Обращаемся к спецификации. Используем this-> для зависимых имен если нужно.
            int statusCode = deviceCodeSpecifications[data].statusCode;

            bool escrow = (statusCode == BillAcceptorStatusCode::BillOperation::Escrow);
            bool stacked = (statusCode == BillAcceptorStatusCode::BillOperation::Stacked);

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
        logLevel = qMin(logLevel, this->getLogLevel(warningLevel));
    }

    if (defaultDescriptionLog.isEmpty()) {
        defaultDescriptionLog << "default";
    }

    if (this->mDeviceCodeBuffers != lastDeviceCodeBuffers) {
        if (this->mDeviceCodeBuffers.isEmpty()) {
            this->toLog(
                logLevel,
                this->mDeviceName +
                    QString(": %1 -> %2").arg(UnknownDeviceCodeDescription).arg(defaultDescriptionLog.join(", ")));
        } else {
            for (TDeviceCodeSpecifications::iterator it = deviceCodeSpecifications.begin();
                 it != deviceCodeSpecifications.end(); ++it) {
                if (!it->description.isEmpty()) {
                    SStatusCodeSpecification statusCodeData = this->mStatusCodesSpecification->value(it->statusCode);
                    logLevel = this->getLogLevel(statusCodeData.warningLevel);

                    QString codeLog;

                    if (it->description == UnknownDeviceCodeDescription) {
                        codeLog = QString(" (%1)").arg(it.key());
                    }

                    this->toLog(
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

    if (!this->setLastPar(parData) && escrow) {
        this->reject();

        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);
        aStatusCodes.insert(BillAcceptorStatusCode::Normal::Enabled);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::setLastPar(const QByteArray &aAnswer) {
    QString log = this->mDeviceName + ": Failed to set last par due to ";

    if ((this->mEscrowPosition < 0) || (this->mEscrowPosition >= aAnswer.size())) {
        this->toLog(LogLevel::Error, log + QString("wrong escrow position = %1 (answer size = %2)")
                                               .arg(this->mEscrowPosition)
                                               .arg(aAnswer.size()));
        return false;
    }

    int escrowKey = uchar(aAnswer[this->mEscrowPosition]);

    if (!this->mEscrowParTable.data().contains(escrowKey)) {
        this->toLog(LogLevel::Error, log + "wrong escrow key = " + ProtocolUtils::toHexLog<uchar>(uchar(escrowKey)));
        return false;
    }

    this->mEscrowPars = TPars() << this->mEscrowParTable[aAnswer[this->mEscrowPosition]];
    SPar par = this->mEscrowPars[0];

    if (!par.nominal) {
        this->toLog(LogLevel::Error, log + "nominal == 0");
        return false;
    }

    if (!CurrencyCodes.data().values().contains(par.currencyId)) {
        this->toLog(LogLevel::Error, log + "unknown currency, id = " + QString::number(par.currencyId));
        return false;
    }

    if (!par.enabled) {
        this->toLog(LogLevel::Error, log + "par isn`t enabled");
        return false;
    }

    if (par.inhibit) {
        this->toLog(LogLevel::Error, log + "par is inhibit");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::restoreStatuses() {
    START_IN_WORKING_THREAD(restoreStatuses)

    this->toLog(LogLevel::Normal,
                this->mDeviceName +
                    QString(", restoreStatuses: post polling action is %1enabled, checking disable state is %2enabled")
                        .arg(this->mPostPollingAction ? "" : "not ")
                        .arg(this->mCheckDisable ? "" : "not "));

    auto canRestore = [&]() -> bool { return !this->mCheckDisable && this->mPostPollingAction; };
    CCashAcceptor::TStatusHistory statusHistory;

    if (this->mPollingActive && this->waitCondition(canRestore, CPortCashAcceptor::RestoreStatusesWaiting) &&
        !this->mStatusHistory.isEmpty()) {
        EWarningLevel::Enum warningLevel = this->mStatusHistory.lastValue().warningLevel;

        for (auto it = this->mStatusHistory.begin() + this->mStatusHistory.getLevel(); it < this->mStatusHistory.end();
             ++it) {
            if (it->warningLevel <= warningLevel) {
                // перепаковываем статусы
                TStatusCodes commonStatusCodes;

                foreach (const TStatusCodes &statusCodes, it->statuses) {
                    commonStatusCodes.unite(statusCodes);
                }

                this->cleanStatusCodes(commonStatusCodes);
                CCashAcceptor::SStatusSpecification resultStatuses;

                foreach (int statusCode, commonStatusCodes) {
                    ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(
                        this->mStatusCodesSpecification->value(statusCode).status);

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
        this->emitStatuses(*it, CCashAcceptor::Set::MainStatuses);
    }
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::onSendDisabled() {
    this->toLog(LogLevel::Normal, QString("Send statuses: %1 disabled").arg(this->mDeviceType));

    emit this->status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::sendEnabled() {
    this->toLog(LogLevel::Normal, QString("Send statuses: %1 enabled").arg(this->mDeviceType));

    emit this->status(EWarningLevel::OK, "Enabled", ECashAcceptorStatus::Enabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::setEnable(bool aEnabled) {
    this->toLog(LogLevel::Normal, QString("%1: %2").arg(this->mDeviceName).arg(aEnabled ? "Enable" : "Disable"));

    if (this->mInitialized != ERequestStatus::Success) {
        if (aEnabled)
            this->toLog(LogLevel::Error, this->mDeviceName + ": on set enable(true) not initialized, return false");
        else
            this->toLog(LogLevel::Normal, this->mDeviceName + ": on set enable(false) not initialized, return true");

        if (!aEnabled) {
            this->toLog(LogLevel::Normal, QString("Send statuses: %1 disabled").arg(this->mDeviceName));

            emit this->status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);
        }

        return !aEnabled;
    }

    if (!this->checkConnectionAbility()) {
        return !aEnabled;
    }

    this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
    this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);

    if ((aEnabled && this->isEnabled()) || (!aEnabled && this->isDisabled())) {
        this->toLog(LogLevel::Normal,
                    this->mDeviceName + QString(": already %1").arg(aEnabled ? "enabled" : "disabled"));

        this->setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);

        if (!aEnabled) {
            this->mCheckDisable = false;
        }

        this->setPollingInterval(this->getPollingInterval(aEnabled));
        this->startPolling();

        // TODO: откат отложенного включения купюроприёмника
        aEnabled ? /*sendEnabled()*/ this->restoreStatuses() : this->onSendDisabled();

        return true;
    }

    this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, aEnabled);
    this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, !aEnabled);

    if (!aEnabled && !this->mPostPollingAction && !this->mCheckDisable) {
        this->toLog(LogLevel::Normal, this->mDeviceName + ": The process has started already");
        return true;
    }

    this->mPostPollingAction = false;

    this->stopPolling();

    auto cancelSetEnable = [&]() -> bool {
        this->toLog(LogLevel::Normal, this->mDeviceName + (aEnabled ? ": An error is occured, Enable return false"
                                                                    : ": An error is occured, Disable return true"));

        this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
        this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);
        this->setConfigParameter(CHardware::CashAcceptor::Enabled, false);

        this->mPostPollingAction = true;

        this->setPollingInterval(this->getPollingInterval(false));
        this->startPolling(true);

        // TODO: откат отложенного включения купюроприёмника
        aEnabled ? /*sendEnabled()*/ this->restoreStatuses() : this->onSendDisabled();

        return !aEnabled;
    };

    if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
        return cancelSetEnable();
    }

    if (!aEnabled) {
        bool canReturn = this->canReturning(false);

        if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
            return cancelSetEnable();
        }

        if (canReturn) {
            PollingExpector expector;
            auto notAccepting = [&]() -> bool {
                return !this->mStatusCollection.contains(BillAcceptorStatusCode::BillOperation::Accepting);
            };
            expector.wait<void>(std::bind(&CashAcceptorBase<T>::simplePoll, this), notAccepting,
                                getPollingInterval(true), CCashAcceptor::Timeout::Escrow);

            canReturn = this->canReturning(true);

            if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
                return cancelSetEnable();
            }

            if (canReturn) {
                auto notEscrow = [&]() -> bool {
                    return !this->mStatusCollection.contains(BillAcceptorStatusCode::BillOperation::Escrow);
                };
                processAndWait(std::bind(&PortCashAcceptor<T>::reject, this), notEscrow, CCashAcceptor::Timeout::Return,
                               false);
            }
        }
    }

    if (!aEnabled && !this->canDisable() && !this->mCheckDisable) {
        // вариант с параметризованным отключением купюроприёмника
        // bool deferred = aDeferred || (containsConfigParameter(CHardware::CashAcceptor::OnlyDeferredDisable) &&
        // getConfigParameter(CHardware::CashAcceptor::OnlyDeferredDisable).toBool());
        bool deferred = true;

        if (!this->mCheckDisable) {
            if (deferred) {
                this->mCheckDisable = true;
                this->toLog(LogLevel::Normal,
                            this->mDeviceName + ": deferred disabling logic is activated, disable return true");
            } else {
                this->toLog(LogLevel::Normal,
                            this->mDeviceName + ": Failed to disable due to operation with note, disable return false");
            }
        }

        this->mPostPollingAction = true;

        this->startPolling(true);
        restoreStatuses();

        return deferred;
    }

    if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
        return cancelSetEnable();
    }

    // TODO: эмитить сигнал только если находимся не в рабочем потоке
    processEnable(aEnabled);

    // TODO: откат отложенного включения купюроприёмника
    return !aEnabled || this->isNotDisabled();
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::processEnable(bool aEnabled) {
    // TODO: откат отложенного включения купюроприёмника
    if (!aEnabled && !this->isWorkingThread()) {
        QMetaObject::invokeMethod(this, "processEnable", Qt::QueuedConnection, Q_ARG(bool, aEnabled));

        return;
    }

    auto setEnableWaitPostAction = [&]() -> bool {
        bool result = aEnabled ? this->isNotDisabled() : this->isNotEnabled();

        if (result) {
            this->mCheckDisable = false;

            this->setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);
            this->setConfigParameter(
                aEnabled ? CHardware::CashAcceptor::ProcessEnabling : CHardware::CashAcceptor::ProcessDisabling, false);

            this->setPollingInterval(this->getPollingInterval(aEnabled));
            if (aEnabled) {
                this->logEnabledPars();
            }
        }

        return result;
    };

    bool needReset = true;

    if (!this->mStatusHistory.isEmpty()) {
        TStatusCollection lastStatusCollection = this->mStatusCollectionHistory.lastValue();
        TStatusCollection beforeLastStatusCollection = this->mStatusCollectionHistory.lastValue(2);
        CCashAcceptor::TStatuses lastStatuses = this->getLastStatuses(1);
        CCashAcceptor::TStatuses beforeLastStatuses = this->getLastStatuses(2);

        needReset = (!lastStatuses.isEmpty(ECashAcceptorStatus::Disabled) &&
                     !beforeLastStatuses.isEmpty(ECashAcceptorStatus::Rejected)) ||
                    (lastStatusCollection.contains(DeviceStatusCode::OK::Initialization) &&
                     (beforeLastStatusCollection.contains(BillAcceptorStatusCode::Normal::Disabled) ||
                      this->isEnabled(beforeLastStatuses))) ||
                    (lastStatusCollection.contains(BillAcceptorStatusCode::Normal::Enabled) && !aEnabled);
    }

    if (aEnabled) {
        applyParTable();
    }

    this->mPostPollingAction = true;
    auto setEnableErrorCondition = [&]() -> bool {
        if (aEnabled)
            return false;
        return !this->getLastStatuses().isEmpty(ECashAcceptorStatus::BillOperation);
    };

    processAndWait(std::bind(&PortCashAcceptor<T>::enableMoneyAcceptingMode, this, aEnabled), setEnableWaitPostAction,
                   CCashAcceptor::Timeout::SetEnable, needReset, true, setEnableErrorCondition);

    if (aEnabled) {
        applyParTable();
    }

    MutexLocker locker(&this->mResourceMutex);

    // TODO: откат отложенного включения купюроприёмника
    /*if (aEnabled && isNotDisabled())
    {
            sendEnabled();
    }
    else */
    if (!aEnabled && this->isNotEnabled()) {
        QTimer::singleShot(this->getConfigParameter(CHardware::CashAcceptor::DisablingTimeout).toInt(), this,
                           SLOT(onSendDisabled()));
    } else {
        restoreStatuses();
    }
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::reenableMoneyAcceptingMode() {
    bool setEnable = this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool();

    if (this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool()) {
        setEnable = false;
    } else if (this->getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool()) {
        setEnable = true;
    }

    return this->enableMoneyAcceptingMode(setEnable);
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

    this->stopPolling();

    PollingExpector expector;

    do {
        if (counter) {
            this->toLog(
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
            this->simplePoll();

            if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
                result = aCondition();
            }
        }
    } while (!result && (++counter < CCashAcceptor::MaxCommandAttempt));

    if (!result) {
        this->toLog(LogLevel::Warning,
                    this->mDeviceName +
                        ((counter >= CCashAcceptor::MaxCommandAttempt)
                             ? ": max attempts to process the command were exceeded, status is not required"
                             : ": error was occured while waiting for required device state"));
    }

    if (aRestartPolling) {
        this->startPolling();
    }

    if (this->mPostPollingAction) {
        this->restoreStatuses();
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::isResetCompleted(bool aWait) {
    if (aWait || (this->mResetWaiting == EResetWaiting::Full)) {
        return this->isAvailable() && !this->isInitialize();
    } else if (this->mResetWaiting == EResetWaiting::Available) {
        return this->isAvailable();
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::reset(bool aWait) {
    if (!this->checkConnectionAbility()) {
        return false;
    }

    if (this->isPowerReboot() && this->mResetOnIdentification) {
        return true;
    }

    auto resetFunction = [&]() -> bool { return this->processReset(); };
    auto condition = std::bind(&PortCashAcceptor<T>::isResetCompleted, this, aWait);

    int timeout = this->getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();

    return this->processAndWait(resetFunction, condition, timeout, false, this->mPollingActive);
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::updateParameters() {
    // TODO: при расширении функционала делать это в базовом классе
    setInitialData();
    processDeviceData();

    if (!this->reset(false) || !this->setDefaultParameters()) {
        return false;
    }

    this->mCurrencyError = this->processParTable();
    ECurrencyError::Enum oldCurrencyError = this->mCurrencyError;

    this->setEnable(false);

    if (this->mCurrencyError == ECurrencyError::Loading) {
        this->mCurrencyError = this->processParTable();
    }

    if (!this->mPollingInterval) {
        this->mPollingInterval = this->getPollingInterval(false);
    }

    PollingExpector expector;
    int timeout = this->getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();
    bool result = expector.wait<void>(
        std::bind(&CashAcceptorBase<T>::simplePoll, this), [&]() -> bool { return !this->isInitialize(); },
        this->getPollingInterval(true), timeout);

    if (this->mCurrencyError == ECurrencyError::Loading) {
        this->mCurrencyError = this->processParTable();
    }

    if (this->mCurrencyError != oldCurrencyError) {
        this->simplePoll();
    }

    return result && (this->mCurrencyError == ECurrencyError::OK);
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::finalizeInitialization() {
    this->addPortData();

    if (this->mPollingActive && (this->mInitialized == ERequestStatus::Success)) {
        this->onPoll();
    }

    this->startPolling(!this->mConnected);
    this->mStatusHistory.checkLastUnprocessed();
    this->restoreStatuses();
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canUpdateFirmware() {
    if (!this->mUpdatable || !this->isDeviceReady()) {
        return false;
    }

    MutexLocker locker(&this->mResourceMutex);

    if (this->isEnabled() && !this->setEnable(false)) {
        this->toLog(LogLevel::Error,
                    this->mDeviceName +
                        ": Failed to disable for updating the firmware due to incorrect state of cash acceptor");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::updateFirmware(const QByteArray &aBuffer) {
    if (!this->isWorkingThread()) {
        QMetaObject::invokeMethod(this, "updateFirmware", Qt::QueuedConnection, Q_ARG(const QByteArray &, aBuffer));

        return;
    }

    bool result = performUpdateFirmware(aBuffer);

    if (result) {
        this->mForceWaitResetCompleting = true;
        reset(false);
        this->mForceWaitResetCompleting = false;
    }

    emit this->updated(result);
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::performUpdateFirmware(const QByteArray & /*aBuffer*/) {
    return false;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canApplyStatusBuffer() {
    bool processEnabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool processDisabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();

    if (processEnabling || (processDisabling && !this->mCheckDisable)) {
        return true;
    }

    return CashAcceptorBase<T>::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T>
void PortCashAcceptor<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                            const TStatusCollection &aOldStatusCollection) {
    CashAcceptorBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);

    if (this->mCheckDisable) {
        if (!this->canDisable()) {
            this->toLog(LogLevel::Normal,
                        this->mDeviceName + ": Failed to disable due to operation process conditions, waiting...");
            return;
        }

        if (!this->setEnable(false)) {
            this->toLog(
                LogLevel::Error,
                this->mDeviceName +
                    QString(": Failed to disable for updating the firmware due to incorrect state of cash acceptor"));
            return;
        }
    }

    if (!this->mStatusCollection.isEmpty(EWarningLevel::Error)) {
        this->mCheckDisable = false;
        return;
    }

    bool enabled = this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool();
    CCashAcceptor::TStatuses statuses = this->mStatusHistory.lastValue().statuses;

    auto toBool = [&](QVariant arg) -> QString { return (arg.toBool()) ? "true" : "false"; };
    auto toPred = [&](QVariant arg) -> QString { return (arg.toBool()) ? "" : "NOT "; };

    bool checkEnabled = this->isEnabled();
    bool checkDisabled = this->isDisabled();
    bool enabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool disabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();
    bool exitFromError =
        !aOldStatusCollection.isEmpty(EWarningLevel::Error) && aNewStatusCollection.isEmpty(EWarningLevel::Error);
    bool beforeRejected = this->mStatusHistory.lastValue(2).statuses.contains(ECashAcceptorStatus::Rejected);

    bool term1 = (enabled || checkEnabled) && disabling;
    bool term2 = (enabled && checkDisabled && !disabling) && !exitFromError;
    bool term3 = (!enabled || checkDisabled) && enabling &&
                 !(beforeRejected && !this->mStatuses.isEmpty(ECashAcceptorStatus::Disabled));

    bool needSetEnable = term1 || term2 || term3;

    this->toLog(LogLevel::Debug,
                this->mDeviceName + QString(": enabled = %1, enable correction is %2needed,\
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
                                        .arg(toPred(this->mStatuses.isEmpty(ECashAcceptorStatus::Disabled))));

    if (needSetEnable) {
        this->setEnable(!this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool());
    }
}

//--------------------------------------------------------------------------------
