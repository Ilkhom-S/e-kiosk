/* @file Базовый класс устройств приема денег на порту. */

#include <QtCore/QtAlgorithms>

#include <Hardware/CashAcceptors/CashAcceptorData.h>
#include <Hardware/CashAcceptors/PortCashAcceptor.h>
#include <Hardware/Common/LoggingType.h>
#include <numeric>

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T>
PortCashAcceptor<T>::PortCashAcceptor()
    : m_CheckDisable(false), m_ResetOnIdentification(false), m_ParInStacked(false),
      m_Updatable(false), m_EscrowPosition(-1), m_ResetWaiting(EResetWaiting::No),
      m_PollingIntervalEnabled(CCashAcceptorsPollingInterval::Enabled),
      m_PollingIntervalDisabled(CCashAcceptorsPollingInterval::Disabled),
      m_ForceWaitResetCompleting(false) {
    // данные устройства
    this->m_DeviceName = "Port cash acceptor";

    this->m_ioMessageLogging = ELoggingType::None;

    this->m_MaxBadAnswers = 4;

    // описания для кодов статусов
    this->setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, 0);
    this->setConfigParameter(CHardware::CashAcceptor::StackedFilter, false);

    this->setConfigParameter(CHardware::CashAcceptor::InitializeTimeout,
                             CCashAcceptor::Timeout::ExitInitialize);

    // Устанавливаем начальные параметры.
    setInitialData();
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::setInitialData() {
    this->m_CurrencyError = ECurrencyError::OK;
    this->m_PollingInterval = getPollingInterval(false);
    this->m_Ready = false;

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
        if (this->m_BadAnswerCounter &&
            this->getStatusCollection(aStatusCodes)[EWarningLevel::Error].isEmpty()) {
            this->reenableMoneyAcceptingMode();
            this->applyParTable();
        }

        // TODO: только если будет замечено, что обмен не прерывался, но внутренние регистры
        // сбросились. applyParTable();
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
    TDeviceCodeBuffers lastDeviceCodeBuffers(this->m_DeviceCodeBuffers);
    this->m_DeviceCodeBuffers.clear();

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

        this->m_DeviceCodeSpecification->getSpecification(answerData, deviceCodeSpecifications);

        auto updatedKeys = deviceCodeSpecifications.keys();
        QSet<QString> newData(updatedKeys.begin(), updatedKeys.end());

        // 2. В Qt 6 оператор '-' для QSet удален. Используем subtract().
        newData.subtract(lastData);

        this->m_DeviceCodeBuffers << answerData;

        // 3. Заменяем внутренний foreach на стандартный цикл
        for (const auto &data : newData) {
            // Обращаемся к спецификации. Используем this-> для зависимых имен если нужно.
            int statusCode = deviceCodeSpecifications[data].statusCode;

            bool escrow = (statusCode == BillAcceptorStatusCode::BillOperation::Escrow);
            bool stacked = (statusCode == BillAcceptorStatusCode::BillOperation::Stacked);

            if (escrow || (stacked && this->m_ParInStacked)) {
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
            this->m_StatusCodesSpecification->value(specification.statusCode).warningLevel;
        logLevel = qMin(logLevel, this->getLogLevel(warningLevel));
    }

    if (defaultDescriptionLog.isEmpty()) {
        defaultDescriptionLog << "default";
    }

    if (this->m_DeviceCodeBuffers != lastDeviceCodeBuffers) {
        if (this->m_DeviceCodeBuffers.isEmpty()) {
            this->toLog(logLevel,
                        this->m_DeviceName + QString(": %1 -> %2")
                                                 .arg(UnknownDeviceCodeDescription)
                                                 .arg(defaultDescriptionLog.join(", ")));
        } else {
            for (TDeviceCodeSpecifications::iterator it = deviceCodeSpecifications.begin();
                 it != deviceCodeSpecifications.end();
                 ++it) {
                if (!it->description.isEmpty()) {
                    SStatusCodeSpecification statusCodeData =
                        this->m_StatusCodesSpecification->value(it->statusCode);
                    logLevel = this->getLogLevel(statusCodeData.warningLevel);

                    QString codeLog;

                    if (it->description == UnknownDeviceCodeDescription) {
                        codeLog = QString(" (%1)").arg(it.key());
                    }

                    this->toLog(logLevel,
                                this->m_DeviceName + QString(": %1%2 -> %3")
                                                         .arg(it->description)
                                                         .arg(codeLog)
                                                         .arg(statusCodeData.description));
                }
            }
        }
    }

    // валюта
    bool escrow = aStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow);
    bool stacked = aStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Stacked);

    if (!escrow && !(stacked && this->m_ParInStacked)) {
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
    QString log = this->m_DeviceName + ": Failed to set last par due to ";

    if ((this->m_EscrowPosition < 0) || (this->m_EscrowPosition >= aAnswer.size())) {
        this->toLog(LogLevel::Error,
                    log + QString("wrong escrow position = %1 (answer size = %2)")
                              .arg(this->m_EscrowPosition)
                              .arg(aAnswer.size()));
        return false;
    }

    int escrowKey = uchar(aAnswer[this->m_EscrowPosition]);

    if (!this->m_EscrowParTable.data().contains(escrowKey)) {
        this->toLog(LogLevel::Error,
                    log + "wrong escrow key = " + ProtocolUtils::toHexLog<uchar>(uchar(escrowKey)));
        return false;
    }

    this->m_EscrowPars = TPars() << this->m_EscrowParTable[aAnswer[this->m_EscrowPosition]];
    SPar par = this->m_EscrowPars[0];

    if (par.nominal == 0.0) {
        this->toLog(LogLevel::Error, log + "nominal == 0");
        return false;
    }

    if (!CurrencyCodes.data().values().contains(par.currencyId)) {
        this->toLog(LogLevel::Error,
                    log + "unknown currency, id = " + QString::number(par.currencyId));
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
                this->m_DeviceName + QString(", restoreStatuses: post polling action is %1enabled, "
                                             "checking disable state is %2enabled")
                                         .arg(this->m_PostPollingAction ? "" : "not ")
                                         .arg(this->m_CheckDisable ? "" : "not "));

    auto canRestore = [&]() -> bool { return !this->m_CheckDisable && this->m_PostPollingAction; };
    CCashAcceptor::TStatusHistory statusHistory;

    if (this->m_PollingActive &&
        this->waitCondition(canRestore, CPortCashAcceptor::RestoreStatusesWaiting) &&
        !this->m_StatusHistory.isEmpty()) {
        EWarningLevel::Enum warningLevel = this->m_StatusHistory.lastValue().warningLevel;

        for (auto it = this->m_StatusHistory.begin() + this->m_StatusHistory.getLevel();
             it < this->m_StatusHistory.end();
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
                    auto status = static_cast<ECashAcceptorStatus::Enum>(
                        this->m_StatusCodesSpecification->value(statusCode).status);

                    if (CCashAcceptor::Set::MainStatuses.contains(status) &&
                        (status != ECashAcceptorStatus::Escrow) &&
                        (status != ECashAcceptorStatus::Stacked)) {
                        resultStatuses.statuses[status].insert(statusCode);
                        resultStatuses.warningLevel =
                            qMax(resultStatuses.warningLevel,
                                 this->m_StatusCodesSpecification->warningLevelByStatus(status));
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
    this->toLog(LogLevel::Normal, QString("Send statuses: %1 disabled").arg(this->m_DeviceType));

    emit this->status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::sendEnabled() {
    this->toLog(LogLevel::Normal, QString("Send statuses: %1 enabled").arg(this->m_DeviceType));

    emit this->status(EWarningLevel::OK, "Enabled", ECashAcceptorStatus::Enabled);

    restoreStatuses();
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::setEnable(bool aEnabled) {
    this->toLog(LogLevel::Normal,
                QString("%1: %2").arg(this->m_DeviceName).arg(aEnabled ? "Enable" : "Disable"));

    if (this->m_Initialized != ERequestStatus::Success) {
        if (aEnabled) {
            this->toLog(LogLevel::Error,
                        this->m_DeviceName + ": on set enable(true) not initialized, return false");
        } else {
            this->toLog(LogLevel::Normal,
                        this->m_DeviceName + ": on set enable(false) not initialized, return true");
        }

        if (!aEnabled) {
            this->toLog(LogLevel::Normal,
                        QString("Send statuses: %1 disabled").arg(this->m_DeviceName));

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
                    this->m_DeviceName +
                        QString(": already %1").arg(aEnabled ? "enabled" : "disabled"));

        this->setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);

        if (!aEnabled) {
            this->m_CheckDisable = false;
        }

        this->setPollingInterval(this->getPollingInterval(aEnabled));
        this->startPolling();

        // TODO: откат отложенного включения купюроприёмника
        aEnabled ? /*sendEnabled()*/ this->restoreStatuses() : this->onSendDisabled();

        return true;
    }

    this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, aEnabled);
    this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, !aEnabled);

    if (!aEnabled && !this->m_PostPollingAction && !this->m_CheckDisable) {
        this->toLog(LogLevel::Normal, this->m_DeviceName + ": The process has started already");
        return true;
    }

    this->m_PostPollingAction = false;

    this->stopPolling();

    auto cancelSetEnable = [&]() -> bool {
        this->toLog(LogLevel::Normal,
                    this->m_DeviceName + (aEnabled ? ": An error is occured, Enable return false"
                                                   : ": An error is occured, Disable return true"));

        this->setConfigParameter(CHardware::CashAcceptor::ProcessEnabling, false);
        this->setConfigParameter(CHardware::CashAcceptor::ProcessDisabling, false);
        this->setConfigParameter(CHardware::CashAcceptor::Enabled, false);

        this->m_PostPollingAction = true;

        this->setPollingInterval(this->getPollingInterval(false));
        this->startPolling(true);

        // TODO: откат отложенного включения купюроприёмника
        aEnabled ? /*sendEnabled()*/ this->restoreStatuses() : this->onSendDisabled();

        return !aEnabled;
    };

    if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
        return cancelSetEnable();
    }

    if (!aEnabled) {
        bool canReturn = this->canReturning(false);

        if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
            return cancelSetEnable();
        }

        if (canReturn) {
            PollingExpector expector;
            auto notAccepting = [&]() -> bool {
                return !this->m_StatusCollection.contains(
                    BillAcceptorStatusCode::BillOperation::Accepting);
            };
            expector.wait<void>(std::bind(&CashAcceptorBase<T>::simplePoll, this),
                                notAccepting,
                                getPollingInterval(true),
                                CCashAcceptor::Timeout::Escrow);

            canReturn = this->canReturning(true);

            if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
                return cancelSetEnable();
            }

            if (canReturn) {
                auto notEscrow = [&]() -> bool {
                    return !this->m_StatusCollection.contains(
                        BillAcceptorStatusCode::BillOperation::Escrow);
                };
                processAndWait(std::bind(&PortCashAcceptor<T>::reject, this),
                               notEscrow,
                               CCashAcceptor::Timeout::Return,
                               false);
            }
        }
    }

    if (!aEnabled && !this->canDisable() && !this->m_CheckDisable) {
        // вариант с параметризованным отключением купюроприёмника
        // bool deferred = aDeferred ||
        // (containsConfigParameter(CHardware::CashAcceptor::OnlyDeferredDisable) &&
        // getConfigParameter(CHardware::CashAcceptor::OnlyDeferredDisable).toBool());
        bool deferred = true;

        if (!this->m_CheckDisable) {
            if (deferred) {
                this->m_CheckDisable = true;
                this->toLog(LogLevel::Normal,
                            this->m_DeviceName +
                                ": deferred disabling logic is activated, disable return true");
            } else {
                this->toLog(
                    LogLevel::Normal,
                    this->m_DeviceName +
                        ": Failed to disable due to operation with note, disable return false");
            }
        }

        this->m_PostPollingAction = true;

        this->startPolling(true);
        restoreStatuses();

        return deferred;
    }

    if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
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
        QMetaObject::invokeMethod(
            this, "processEnable", Qt::QueuedConnection, Q_ARG(bool, aEnabled));

        return;
    }

    auto setEnableWaitPostAction = [&]() -> bool {
        bool result = aEnabled ? this->isNotDisabled() : this->isNotEnabled();

        if (result) {
            this->m_CheckDisable = false;

            this->setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);
            this->setConfigParameter(aEnabled ? CHardware::CashAcceptor::ProcessEnabling
                                              : CHardware::CashAcceptor::ProcessDisabling,
                                     false);

            this->setPollingInterval(this->getPollingInterval(aEnabled));
            if (aEnabled) {
                this->logEnabledPars();
            }
        }

        return result;
    };

    bool needReset = true;

    if (!this->m_StatusHistory.isEmpty()) {
        TStatusCollection lastStatusCollection = this->m_StatusCollectionHistory.lastValue();
        TStatusCollection beforeLastStatusCollection = this->m_StatusCollectionHistory.lastValue(2);
        CCashAcceptor::TStatuses lastStatuses = this->getLastStatuses(1);
        CCashAcceptor::TStatuses beforeLastStatuses = this->getLastStatuses(2);

        needReset =
            ((lastStatuses.isEmpty(ECashAcceptorStatus::Disabled) == 0) &&
             (beforeLastStatuses.isEmpty(ECashAcceptorStatus::Rejected) == 0)) ||
            (lastStatusCollection.contains(DeviceStatusCode::OK::Initialization) &&
             (beforeLastStatusCollection.contains(BillAcceptorStatusCode::Normal::Disabled) ||
              this->isEnabled(beforeLastStatuses))) ||
            (lastStatusCollection.contains(BillAcceptorStatusCode::Normal::Enabled) && !aEnabled);
    }

    if (aEnabled) {
        applyParTable();
    }

    this->m_PostPollingAction = true;
    auto setEnableErrorCondition = [&]() -> bool {
        if (aEnabled) {
            return false;
        }
        return !this->getLastStatuses().isEmpty(ECashAcceptorStatus::BillOperation);
    };

    processAndWait(std::bind(&PortCashAcceptor<T>::enableMoneyAcceptingMode, this, aEnabled),
                   setEnableWaitPostAction,
                   CCashAcceptor::Timeout::SetEnable,
                   needReset,
                   true,
                   setEnableErrorCondition);

    if (aEnabled) {
        applyParTable();
    }

    MutexLocker locker(&this->m_ResourceMutex);

    // TODO: откат отложенного включения купюроприёмника
    /*if (aEnabled && isNotDisabled())
    {
            sendEnabled();
    }
    else */
    if (!aEnabled && this->isNotEnabled()) {
        QTimer::singleShot(
            this->getConfigParameter(CHardware::CashAcceptor::DisablingTimeout).toInt(),
            this,
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
    return aEnabled ? this->m_PollingIntervalEnabled : this->m_PollingIntervalDisabled;
}

//---------------------------------------------------------------------------
template <class T>
bool PortCashAcceptor<T>::processAndWait(const TBoolMethod &aCommand,
                                         TBoolMethod aCondition,
                                         int aTimeout,
                                         bool aNeedReset,
                                         bool aRestartPolling,
                                         TBoolMethod aErrorCondition) {
    int counter = 0;
    bool result = false;

    this->stopPolling();

    PollingExpector expector;

    do {
        if (counter != 0) {
            this->toLog(LogLevel::Warning,
                        this->m_DeviceName +
                            QString(": waiting timeout for status change has expired, status is "
                                    "not required, try iteration %1")
                                .arg(counter + 1));

            if (aNeedReset) {
                reset(true);
            }
        }

        if (aCommand()) {
            result = expector.wait<void>(std::bind(&CashAcceptorBase<T>::simplePoll, this),
                                         aCondition,
                                         aErrorCondition,
                                         getPollingInterval(true),
                                         aTimeout);
        } else {
            this->simplePoll();

            if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
                result = aCondition();
            }
        }
    } while (!result && (++counter < CCashAcceptor::MaxCommandAttempt));

    if (!result) {
        this->toLog(
            LogLevel::Warning,
            this->m_DeviceName +
                ((counter >= CCashAcceptor::MaxCommandAttempt)
                     ? ": max attempts to process the command were exceeded, status is not required"
                     : ": error was occured while waiting for required device state"));
    }

    if (aRestartPolling) {
        this->startPolling();
    }

    if (this->m_PostPollingAction) {
        this->restoreStatuses();
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::isResetCompleted(bool aWait) {
    if (aWait || (this->m_ResetWaiting == EResetWaiting::Full)) {
        return this->isAvailable() && !this->isInitialize();
    }
    if (this->m_ResetWaiting == EResetWaiting::Available) {
        return this->isAvailable();
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::reset(bool aWait) {
    if (!this->checkConnectionAbility()) {
        return false;
    }

    if (this->isPowerReboot() && this->m_ResetOnIdentification) {
        return true;
    }

    auto resetFunction = [&]() -> bool { return this->processReset(); };
    auto condition = std::bind(&PortCashAcceptor<T>::isResetCompleted, this, aWait);

    int timeout = this->getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();

    return this->processAndWait(resetFunction, condition, timeout, false, this->m_PollingActive);
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::updateParameters() {
    // TODO: при расширении функционала делать это в базовом классе
    setInitialData();
    processDeviceData();

    if (!this->reset(false) || !this->setDefaultParameters()) {
        return false;
    }

    this->m_CurrencyError = this->processParTable();
    ECurrencyError::Enum oldCurrencyError = this->m_CurrencyError;

    this->setEnable(false);

    if (this->m_CurrencyError == ECurrencyError::Loading) {
        this->m_CurrencyError = this->processParTable();
    }

    if (!this->m_PollingInterval) {
        this->m_PollingInterval = this->getPollingInterval(false);
    }

    PollingExpector expector;
    int timeout = this->getConfigParameter(CHardware::CashAcceptor::InitializeTimeout).toInt();
    bool result = expector.wait<void>(
        std::bind(&CashAcceptorBase<T>::simplePoll, this),
        [&]() -> bool { return !this->isInitialize(); },
        this->getPollingInterval(true),
        timeout);

    if (this->m_CurrencyError == ECurrencyError::Loading) {
        this->m_CurrencyError = this->processParTable();
    }

    if (this->m_CurrencyError != oldCurrencyError) {
        this->simplePoll();
    }

    return result && (this->m_CurrencyError == ECurrencyError::OK);
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::finalizeInitialization() {
    this->addPortData();

    if (this->m_PollingActive && (this->m_Initialized == ERequestStatus::Success)) {
        this->onPoll();
    }

    this->startPolling(!this->m_Connected);
    this->m_StatusHistory.checkLastUnprocessed();
    this->restoreStatuses();
}

//--------------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canUpdateFirmware() {
    if (!this->m_Updatable || !this->isDeviceReady()) {
        return false;
    }

    MutexLocker locker(&this->m_ResourceMutex);

    if (this->isEnabled() && !this->setEnable(false)) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName + ": Failed to disable for updating the firmware due to "
                                         "incorrect state of cash acceptor");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortCashAcceptor<T>::updateFirmware(const QByteArray &aBuffer) {
    if (!this->isWorkingThread()) {
        QMetaObject::invokeMethod(
            this, "updateFirmware", Qt::QueuedConnection, Q_ARG(const QByteArray &, aBuffer));

        return;
    }

    bool result = perform_UpdateFirmware(aBuffer);

    if (result) {
        this->m_ForceWaitResetCompleting = true;
        reset(false);
        this->m_ForceWaitResetCompleting = false;
    }

    emit this->updated(result);
}

//---------------------------------------------------------------------------
template <class T>
bool PortCashAcceptor<T>::perform_UpdateFirmware(const QByteArray & /*aBuffer*/) {
    return false;
}

//---------------------------------------------------------------------------
template <class T> bool PortCashAcceptor<T>::canApplyStatusBuffer() {
    bool processEnabling =
        this->getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool processDisabling =
        this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();

    if (processEnabling || (processDisabling && !this->m_CheckDisable)) {
        return true;
    }

    return CashAcceptorBase<T>::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T>
void PortCashAcceptor<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                            const TStatusCollection &aOldStatusCollection) {
    CashAcceptorBase<T>::postPollingAction(aNewStatusCollection, aOldStatusCollection);

    if (this->m_CheckDisable) {
        if (!this->canDisable()) {
            this->toLog(LogLevel::Normal,
                        this->m_DeviceName +
                            ": Failed to disable due to operation process conditions, waiting...");
            return;
        }

        if (!this->setEnable(false)) {
            this->toLog(LogLevel::Error,
                        this->m_DeviceName +
                            QString(": Failed to disable for updating the firmware "
                                    "due to incorrect state of cash acceptor"));
            return;
        }
    }

    if (!this->m_StatusCollection.isEmpty(EWarningLevel::Error)) {
        this->m_CheckDisable = false;
        return;
    }

    bool enabled = this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool();
    CCashAcceptor::TStatuses statuses = this->m_StatusHistory.lastValue().statuses;

    auto toBool = [&](QVariant arg) -> QString { return (arg.toBool()) ? "true" : "false"; };
    auto toPred = [&](QVariant arg) -> QString { return (arg.toBool()) ? "" : "NOT "; };

    bool checkEnabled = this->isEnabled();
    bool checkDisabled = this->isDisabled();
    bool enabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessEnabling).toBool();
    bool disabling = this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool();
    bool exitFromError = (aOldStatusCollection.isEmpty(EWarningLevel::Error) == 0) &&
                         (aNewStatusCollection.isEmpty(EWarningLevel::Error) != 0);
    bool beforeRejected =
        this->m_StatusHistory.lastValue(2).statuses.contains(ECashAcceptorStatus::Rejected);

    bool term1 = (enabled || checkEnabled) && disabling;
    bool term2 = (enabled && checkDisabled && !disabling) && !exitFromError;
    bool term3 = (!enabled || checkDisabled) && enabling &&
                 !(beforeRejected && !this->m_Statuses.isEmpty(ECashAcceptorStatus::Disabled));

    bool needSetEnable = term1 || term2 || term3;

    this->toLog(LogLevel::Debug,
                this->m_DeviceName +
                    QString(": enabled = %1, enable correction is %2needed,\
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
                        .arg(toPred(this->m_Statuses.isEmpty(ECashAcceptorStatus::Disabled))));

    if (needSetEnable) {
        this->setEnable(
            !this->getConfigParameter(CHardware::CashAcceptor::ProcessDisabling).toBool());
    }
}

//--------------------------------------------------------------------------------
