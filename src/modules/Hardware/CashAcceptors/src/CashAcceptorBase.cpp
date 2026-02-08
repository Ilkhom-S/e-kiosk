/* @file Базовый класс устройств приема денег. */

#include <QtCore/QtAlgorithms>

#include <Hardware/CashAcceptors/CashAcceptorBase.h>
#include <numeric>

#include "Hardware/CashAcceptors/CashAcceptorStatusesDescriptions.h"

namespace Currency {
const char NoCurrencyCode[] = "XXX";
} // namespace Currency

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T> CashAcceptorBase<T>::CashAcceptorBase() {
    // данные устройства
    this->m_DeviceType = "Base cash acceptor";
    this->m_CurrencyError = ECurrencyError::OK;
    this->m_Ready = false;

    // описания для кодов статусов
    this->m_StatusCodesSpecification =
        DeviceStatusCode::PSpecifications(new BillAcceptorStatusCode::CSpecifications());
    this->m_DeviceType = CHardware::Types::CashAcceptor;

    // параметры истории статусов
    this->m_StatusHistory.setSize(5);

    // восстановимые ошибки
    this->m_RecoverableErrors.insert(BillAcceptorStatusCode::Error::ParTableLoading);

    // Неустойчивые пограничные состояния
    for (auto it = this->m_StatusCodesSpecification->data().begin();
         it != this->m_StatusCodesSpecification->data().end();
         ++it) {
        if ((it->status == ECashAcceptorStatus::Cheated) ||
            (it->status == ECashAcceptorStatus::Busy) ||
            (it->status == ECashAcceptorStatus::BillOperation) ||
            (it->status == ECashAcceptorStatus::Rejected)) {
            this->m_UnsafeStatusCodes.insert(it.key());
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::release() {
    bool result = T::release();

    MutexLocker locker(&this->m_ResourceMutex);

    this->m_StatusHistory.clear();
    this->m_StatusHistory.append(CCashAcceptor::SStatusSpecification());
    this->m_StatusHistory.updateLevel();

    this->m_Statuses.clear();
    this->m_CurrencyError = ECurrencyError::OK;

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::isDeviceReady() {
    return this->m_Ready;
}

//---------------------------------------------------------------------------
template <class T> CCashAcceptor::TStatuses CashAcceptorBase<T>::getLastStatuses(int aLevel) const {
    if (this->m_StatusCollectionHistory.isEmpty()) {
        return CCashAcceptor::TStatuses();
    }

    HistoryList<TStatusCollection>::const_iterator lastStatusCollectionIt =
        this->m_StatusCollectionHistory.end() - qMin(this->m_StatusCollectionHistory.size(), aLevel);
    TStatusCodes lastStatusCodes;

    foreach (const TStatusCodes &statusCodes, *lastStatusCollectionIt) {
        lastStatusCodes += statusCodes;
    }

    CCashAcceptor::TStatuses statuses;

    foreach (int statusCode, lastStatusCodes) {
        ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(
            this->m_StatusCodesSpecification->value(statusCode).status);
        statuses[status].insert(statusCode);
    }

    return statuses;
}

//---------------------------------------------------------------------------
template <class T> TStatusCodes CashAcceptorBase<T>::getLongStatusCodes() const {
    TStatusCodes result;

    for (auto it = this->m_StatusCodesSpecification->data().begin();
         it != this->m_StatusCodesSpecification->data().end();
         ++it) {
        ECashAcceptorStatus::Enum status = ECashAcceptorStatus::Enum(it->status);

        if (CCashAcceptor::Set::LongStatuses.contains(status)) {
            result << it.key();
        }
    }

    result -= TStatusCodes() << BillAcceptorStatusCode::BillOperation::Unloaded
                             << BillAcceptorStatusCode::BillOperation::Dispensed
                             << BillAcceptorStatusCode::Busy::SetStackerType
                             << BillAcceptorStatusCode::Busy::Returned;

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::canDisable() const {
    CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

    return lastStatuses[ECashAcceptorStatus::BillOperation].isEmpty() &&
           lastStatuses[ECashAcceptorStatus::Escrow].isEmpty() &&
           lastStatuses[ECashAcceptorStatus::Stacked].isEmpty() &&
           lastStatuses[ECashAcceptorStatus::Busy].isEmpty() &&
           !lastStatuses[ECashAcceptorStatus::Warning].contains(
               BillAcceptorStatusCode::Warning::Cheated);
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::isNotEnabled() const {
    CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

    return !isEnabled() || isDisabled() ||
           lastStatuses[ECashAcceptorStatus::OK].contains(DeviceStatusCode::OK::OK);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isEnabled(const CCashAcceptor::TStatuses &aStatuses) const {
    CCashAcceptor::TStatuses lastStatuses = !aStatuses.isEmpty() ? aStatuses : getLastStatuses();

    if (!std::accumulate(lastStatuses.begin(),
                         lastStatuses.end(),
                         0,
                         [](int aStatusAmount, const TStatusCodes &aStatusCodes) -> int {
                             return aStatusAmount + aStatusCodes.size();
                         })) {
        this->toLog(
            LogLevel::Normal,
            QString("No actual last statuses, it is impossible to know the device is turned on"));
        return false;
    }

    return !lastStatuses[ECashAcceptorStatus::Enabled].isEmpty() ||
           !lastStatuses[ECashAcceptorStatus::BillOperation].isEmpty() ||
           !lastStatuses[ECashAcceptorStatus::Escrow].isEmpty() ||
           !lastStatuses[ECashAcceptorStatus::Stacked].isEmpty() ||
           !lastStatuses[ECashAcceptorStatus::Cheated].isEmpty() ||
           lastStatuses[ECashAcceptorStatus::OperationError].contains(
               BillAcceptorStatusCode::OperationError::Accept) ||
           lastStatuses[ECashAcceptorStatus::OperationError].contains(
               BillAcceptorStatusCode::OperationError::Escrow) ||
           lastStatuses[ECashAcceptorStatus::OperationError].contains(
               BillAcceptorStatusCode::OperationError::Stack) ||
           lastStatuses[ECashAcceptorStatus::Warning].contains(
               BillAcceptorStatusCode::Warning::Cheated) ||
           lastStatuses[ECashAcceptorStatus::Busy].contains(BillAcceptorStatusCode::Busy::Pause);
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::isNotDisabled() const {
    CCashAcceptor::TStatuses lastStatuses = getLastStatuses();

    return !isDisabled() || isEnabled() ||
           lastStatuses[ECashAcceptorStatus::OK].contains(DeviceStatusCode::OK::OK);
}

//---------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isDisabled(const CCashAcceptor::TStatuses &aStatuses) const {
    CCashAcceptor::TStatuses lastStatuses = !aStatuses.isEmpty() ? aStatuses : getLastStatuses();

    if (!std::accumulate(lastStatuses.begin(),
                         lastStatuses.end(),
                         0,
                         [](int aStatusAmount, const TStatusCodes &aStatusCodes) -> int {
                             return aStatusAmount + aStatusCodes.size();
                         })) {
        this->toLog(
            LogLevel::Normal,
            QString("No actual last statuses, it is impossible to know the device is turned off"));
        return false;
    }

    return !isEnabled() &&
           (isInitialize() || !lastStatuses[ECashAcceptorStatus::Disabled].isEmpty() ||
            !lastStatuses[ECashAcceptorStatus::Inhibit].isEmpty() ||
            !lastStatuses[ECashAcceptorStatus::MechanicFailure].isEmpty() ||
            !lastStatuses[ECashAcceptorStatus::StackerOpen].isEmpty() ||
            !lastStatuses[ECashAcceptorStatus::StackerFull].isEmpty() ||
            !lastStatuses[ECashAcceptorStatus::Error].isEmpty());
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::isInitialize() const {
    TStatusCodes lastStatusCodes = getLastStatuses()[ECashAcceptorStatus::Busy];

    return lastStatusCodes.contains(DeviceStatusCode::OK::Initialization) ||
           lastStatusCodes.contains(DeviceStatusCode::OK::Busy) ||
           lastStatusCodes.contains(BillAcceptorStatusCode::Busy::Calibration);
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::isAvailable() {
    if (this->m_StatusCollection.isEmpty()) {
        this->simplePoll();
    }

    return !this->m_StatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T> bool CashAcceptorBase<T>::canReturning(bool aOnline) {
    if (this->m_DeviceType != CHardware::Types::BillAcceptor) {
        return false;
    }

    this->simplePoll();

    TStatusCodes lastStatusCodes = this->m_StatusCollection.value(EWarningLevel::OK);
    bool result =
        lastStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow) ||
        (lastStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Accepting) && !aOnline);

    return result;
}

//---------------------------------------------------------------------------
template <class T> void CashAcceptorBase<T>::logEnabledPars() {
    MutexLocker locker(&this->m_ResourceMutex);

    bool onlyBillAcceptor = true;
    bool enable = false;

    for (auto it = this->m_EscrowParTable.data().begin(); it != this->m_EscrowParTable.data().end();
         ++it) {
        SPar &par = it.value();

        if (!par.inhibit && par.enabled && par.nominal) {
            enable = true;
            onlyBillAcceptor =
                onlyBillAcceptor && (it.value().cashReceiver == ECashReceiver::BillAcceptor);
        }
    }

    if (enable) {
        QString log = this->m_DeviceName + ": successfully enable nominals - ";

        for (auto it = this->m_EscrowParTable.data().begin();
             it != this->m_EscrowParTable.data().end();
             ++it) {
            if (!it.value().inhibit && it.value().enabled && it.value().nominal) {
                log +=
                    QString("\nnominal %1: currency %2(\"%3\")%4")
                        .arg(it->nominal, 5)
                        .arg(it->currencyId)
                        .arg(it->currency)
                        .arg(onlyBillAcceptor ? ""
                                              : ((it->cashReceiver == ECashReceiver::BillAcceptor)
                                                     ? " in bill acceptor"
                                                     : " in coin acceptor"));
            }
        }

        this->toLog(LogLevel::Normal, log);
    } else {
        this->toLog(LogLevel::Warning, this->m_DeviceName + ": no nominals for enabling!");
    }
}

//---------------------------------------------------------------------------
template <class T> void CashAcceptorBase<T>::setParList(const TParList &aParList) {
    MutexLocker locker(&this->m_ParListMutex);

    this->m_ParList = aParList;

    START_IN_WORKING_THREAD(employParList)
}

//---------------------------------------------------------------------------
template <class T> TParList CashAcceptorBase<T>::getParList() {
    MutexLocker parListLocker(&this->m_ParListMutex);
    MutexLocker resourceLocker(&this->m_ResourceMutex);

    TParList parList = this->m_EscrowParTable.data().values();

    foreach (const SPar &par, this->m_ParList) {
        if (!parList.contains(par)) {
            parList << par;
        }
    }

    return parList;
}

//---------------------------------------------------------------------------
template <class T> void CashAcceptorBase<T>::employParList() {
    bool noValidParTable = (this->m_CurrencyError == ECurrencyError::Loading) ||
                           (this->m_CurrencyError == ECurrencyError::Config) ||
                           (this->m_CurrencyError == ECurrencyError::Billset) ||
                           (this->m_CurrencyError == ECurrencyError::Compatibility);

    if (!this->m_Connected || (this->m_Initialized == ERequestStatus::Fail) || noValidParTable) {
        return;
    }

    MutexLocker resourceLocker(&this->m_ResourceMutex);

    CCashAcceptor::TStatuses lastStatuses = this->m_StatusHistory.lastValue().statuses;
    TStatusCollection lastStatusCollection = this->m_StatusCollectionHistory.lastValue();

    if (!lastStatuses.isEmpty(ECashAcceptorStatus::Rejected) ||
        !lastStatusCollection.isEmpty(EWarningLevel::Error)) {
        return;
    }

    TParTable oldTable = this->m_EscrowParTable.data();

    {
        MutexLocker parListLocker(&this->m_ParListMutex);

        if (this->m_ParList.isEmpty()) {
            return;
        }

        for (auto it = this->m_EscrowParTable.data().begin();
             it != this->m_EscrowParTable.data().end();
             ++it) {
            int index = this->m_ParList.indexOf(*it);
            it.value().enabled = (index != -1) && this->m_ParList[index].enabled;
        }

        if (std::find_if(this->m_EscrowParTable.data().begin(),
                         this->m_EscrowParTable.data().end(),
                         [&](const SPar &aPar) -> bool { return aPar.enabled; }) ==
            this->m_EscrowParTable.data().end()) {
            this->m_CurrencyError = ECurrencyError::NoAvailable;

            return;
        }
    }

    QList<int> positions = this->m_EscrowParTable.data().keys();

    if (std::find_if(positions.begin(), positions.end(), [&](int aPosition) -> bool {
            return this->m_EscrowParTable[aPosition].isEqual(oldTable[aPosition]);
        }) != positions.end()) {
        ECashReceiver::Enum cashReceiver;
        bool complexDevice = (std::find_if(this->m_EscrowParTable.data().begin(),
                                           this->m_EscrowParTable.data().end(),
                                           [&](const SPar &aPar) -> bool {
                                               bool result = bool(aPar.nominal);
                                               cashReceiver = aPar.cashReceiver;
                                               return result;
                                           }) != this->m_EscrowParTable.data().end()) &&
                             (std::find_if(this->m_EscrowParTable.data().begin(),
                                           this->m_EscrowParTable.data().end(),
                                           [&](const SPar &aPar) -> bool {
                                               return cashReceiver != aPar.cashReceiver;
                                           }) != this->m_EscrowParTable.data().end());

        if (std::find_if(this->m_EscrowParTable.data().begin(),
                         this->m_EscrowParTable.data().end(),
                         [&](const SPar &aPar) -> bool {
                             bool result = aPar.nominal && !aPar.enabled;
                             return result;
                         }) != this->m_EscrowParTable.data().end()) {
            QString log = this->m_DeviceName + ": Nominal(s) disabled:";

            for (auto it = this->m_EscrowParTable.data().begin();
                 it != this->m_EscrowParTable.data().end();
                 ++it) {
                if (it->nominal && !it->enabled) {
                    log +=
                        QString("\nnominal %1: currency %2(\"%3\")%4")
                            .arg(it->nominal, 5)
                            .arg(it->currencyId)
                            .arg(it->currency)
                            .arg(!complexDevice ? ""
                                                : ((it->cashReceiver == ECashReceiver::BillAcceptor)
                                                       ? " in bill acceptor"
                                                       : " in coin acceptor"));
                }
            }

            this->toLog(LogLevel::Normal, log);
        }
    }

    this->m_CurrencyError = ECurrencyError::OK;
}

//--------------------------------------------------------------------------------
template <class T> ECurrencyError::Enum CashAcceptorBase<T>::processParTable() {
    if (!this->containsConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId)) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": No system currency id in parameters!");
        return ECurrencyError::Config;
    }

    int systemCurrencyId =
        this->getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt();

    if (!CurrencyCodes.data().values().contains(systemCurrencyId)) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName +
                        ": Unknown system currency id = " + QString::number(systemCurrencyId));
        return ECurrencyError::Config;
    }

    QString log = this->m_DeviceName + ": successfully loaded nominal table -";

    MutexLocker locker(&this->m_ResourceMutex);

    this->m_EscrowParTable.data().clear();

    if (!loadParTable()) {
        return ECurrencyError::Loading;
    }

    if (this->m_EscrowParTable.data().isEmpty()) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": par table is empty!");
        return ECurrencyError::Loading;
    }

    bool billset = false;
    bool compatibility = false;

    for (auto it = this->m_EscrowParTable.data().begin(); it != this->m_EscrowParTable.data().end();
         ++it) {
        ECashReceiver::Enum cashReceiver = it.value().cashReceiver;

        if (this->m_DeviceType == CHardware::Types::CashAcceptor) {
            this->m_DeviceType = (cashReceiver == ECashReceiver::BillAcceptor)
                                    ? CHardware::Types::BillAcceptor
                                    : CHardware::Types::CoinAcceptor;
        } else if (((this->m_DeviceType == CHardware::Types::BillAcceptor) &&
                    (cashReceiver == ECashReceiver::CoinAcceptor)) ||
                   ((this->m_DeviceType == CHardware::Types::CoinAcceptor) &&
                    (cashReceiver == ECashReceiver::BillAcceptor))) {
            this->m_DeviceType = CHardware::Types::DualCashAcceptor;
        }

        it.value().inhibit = true;
        int currencyId = it.value().currencyId;

        if (it.value().currency.isEmpty() && currencyId && (currencyId != Currency::NoCurrency)) {
            it.value().currency = CurrencyCodes.key(currencyId);
        }

        if (CurrencyCodes.data().keys().contains(it.value().currency)) {
            billset = true;

            if (CurrencyCodes.isAccorded(it.value().currency, systemCurrencyId)) {
                compatibility = true;

                if (it.value().nominal > 0) {
                    it.value().inhibit = false;
                    it.value().currencyId = CurrencyCodes[it.value().currency];
                }
            } else {
                this->toLog(LogLevel::Error,
                            QString("%1: nominal %2 - currency \"%3\" is not accorded with system "
                                    "currency id %4")
                                .arg(this->m_DeviceName)
                                .arg(it.value().nominal, 5)
                                .arg(it.value().currency)
                                .arg(systemCurrencyId));
            }
        } else if (it.value().nominal) {
            this->toLog(LogLevel::Error,
                        QString("%1: nominal %2 - unknown currency \"%3\"")
                            .arg(this->m_DeviceName)
                            .arg(it.value().nominal, 5)
                            .arg(it.value().currency));
        }
    }

    if (!billset) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Unknown billset");
        return ECurrencyError::Billset;
    }

    if (!compatibility) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName +
                        ": Wrong compatibility billset currency and config currency");
        return ECurrencyError::Config;
    }

    for (auto it = this->m_EscrowParTable.data().begin(); it != this->m_EscrowParTable.data().end();
         ++it) {
        if (!it.value().inhibit) {
            log += QString("\nnominal %1: currency %2(\"%3\")%4%5")
                       .arg(it->nominal, 5)
                       .arg(it->currencyId)
                       .arg(it->currency)
                       .arg((this->m_DeviceType != CHardware::Types::DualCashAcceptor)
                                ? ""
                                : ((it->cashReceiver == ECashReceiver::BillAcceptor)
                                       ? " in bill acceptor"
                                       : " in coin acceptor"))
                       .arg(it->inhibit ? ", inhibited" : "");
        }
    }

    this->toLog(LogLevel::Normal, log);

    if (this->getConfigParameter(CHardware::CallingType).toString() ==
        CHardware::CallingTypes::Internal) {
        this->employParList();

        return this->m_CurrencyError;
    }

    return ECurrencyError::OK;
}

//--------------------------------------------------------------------------------
template <class T>
bool CashAcceptorBase<T>::isStatusCollectionConformed(const TStatusCodesHistory &aHistory) {
    if (this->m_StatusCollectionHistory.size() < aHistory.size()) {
        return false;
    }

    for (int i = 0; i < aHistory.size(); ++i) {
        TStatusCodes historyStatusCodes =
            this->getStatusCodes(this->m_StatusCollectionHistory.lastValue(aHistory.size() - i));
        TStatusCodes testStatusCodes = TStatusCodes() << aHistory[i];

        if (!historyStatusCodes.contains(testStatusCodes)) {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::replaceConformedStatusCodes(TStatusCodes &aStatusCodes,
                                                      int aStatusCodeFrom,
                                                      int aStatusCodeTo) {
    TStatusCodesHistory history = TStatusCodesHistory() << aStatusCodeFrom;

    if (aStatusCodes.contains(aStatusCodeFrom) &&
        (isStatusCollectionConformed(history) ||
         isStatusCollectionConformed(history << aStatusCodeTo))) {
        aStatusCodes.remove(aStatusCodeFrom);
        aStatusCodes.insert(aStatusCodeTo);
    }
}

//--------------------------------------------------------------------------------
template <class T> void CashAcceptorBase<T>::cleanStatusCodes(TStatusCodes &aStatusCodes) {
    if (m_CurrencyError != ECurrencyError::OK) {
        aStatusCodes.insert(ECurrencyError::Specification[m_CurrencyError]);
    }

    T::cleanStatusCodes(aStatusCodes);

    TStatusCodes ordinaryStatusCodes = TStatusCodes() << BillAcceptorStatusCode::Normal::Enabled
                                                      << BillAcceptorStatusCode::Normal::Disabled
                                                      << BillAcceptorStatusCode::Normal::Inhibit;

    if ((aStatusCodes - ordinaryStatusCodes).isEmpty()) {
        bool enabled = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Enabled);
        bool disabled = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Disabled);
        bool inhibit = aStatusCodes.contains(BillAcceptorStatusCode::Normal::Inhibit);

        // купюроприемник одновременно говорит, что он и включен, и выключен на прием
        if (enabled && (disabled || inhibit)) {
            if (this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool()) {
                aStatusCodes.remove(BillAcceptorStatusCode::Normal::Disabled);
                aStatusCodes.remove(BillAcceptorStatusCode::Normal::Inhibit);
                aStatusCodes.insert(BillAcceptorStatusCode::Normal::Enabled);
            } else {
                aStatusCodes.remove(BillAcceptorStatusCode::Normal::Enabled);
                aStatusCodes.insert(BillAcceptorStatusCode::Normal::Disabled);

                if (disabled && inhibit) {
                    aStatusCodes.remove(BillAcceptorStatusCode::Normal::Disabled);
                }
            }
        }
    }

    TStatusCodes notOrdinaryStatusCodes = aStatusCodes - ordinaryStatusCodes;

    if (!notOrdinaryStatusCodes.isEmpty() &&
        (notOrdinaryStatusCodes.size() < aStatusCodes.size())) {
        aStatusCodes = notOrdinaryStatusCodes;
    }

    cleanSpecificStatusCodes(aStatusCodes);

    if (aStatusCodes.contains(BillAcceptorStatusCode::MechanicFailure::StackerFull)) {
        aStatusCodes.remove(BillAcceptorStatusCode::Warning::StackerNearFull);
    }

    if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Accept)) {
        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Accepting);
    }

    if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Escrow)) {
        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);
    }

    if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Stack)) {
        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Stacked);
    }

    if (aStatusCodes.contains(BillAcceptorStatusCode::OperationError::Return)) {
        aStatusCodes.remove(BillAcceptorStatusCode::Busy::Returned);
    }

    CCashAcceptor::TStatuses statuses;

    foreach (int statusCode, aStatusCodes) {
        ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(
            this->m_StatusCodesSpecification->value(statusCode).status);
        statuses[status].insert(statusCode);
    }

    TStatusCodes rejects = statuses.value(ECashAcceptorStatus::Rejected);

    if (rejects.size() > 1) {
        aStatusCodes.remove(BillAcceptorStatusCode::Reject::Unknown);
    }

    bool warningNotCheated = !statuses.isEmpty(ECashAcceptorStatus::Warning) &&
                             !statuses.contains(BillAcceptorStatusCode::Warning::Cheated);

    TStatusCodes unknownErrors = TStatusCodes() << BillAcceptorStatusCode::Error::Clock
                                                << BillAcceptorStatusCode::Error::NoParsAvailable
                                                << BillAcceptorStatusCode::Error::Firmware
                                                << DeviceStatusCode::Error::Initialization
                                                << DeviceStatusCode::Error::MemoryStorage;
    TStatusCodes actualUnknownErrors = statuses.value(ECashAcceptorStatus::Error) & unknownErrors;

    if ((aStatusCodes.size() > 1) && !warningNotCheated &&
        (!statuses.contains(ECashAcceptorStatus::Error) || actualUnknownErrors.isEmpty())) {
        aStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Unknown);
    }

    if (aStatusCodes.contains(BillAcceptorStatusCode::Busy::Returning) ||
        aStatusCodes.contains(BillAcceptorStatusCode::Busy::Returned)) {
        foreach (int statusCode, aStatusCodes) {
            ECashAcceptorStatus::Enum status = static_cast<ECashAcceptorStatus::Enum>(
                this->m_StatusCodesSpecification->value(statusCode).status);

            if (status == ECashAcceptorStatus::Rejected) {
                aStatusCodes.remove(statusCode);
            }
        }
    }

    if (aStatusCodes.size() > 1) {
        aStatusCodes.remove(DeviceStatusCode::OK::OK);
    }
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::saveStatuses(const CCashAcceptor::TStatuses &aStatuses,
                                       ECashAcceptorStatus::Enum aTargetStatus,
                                       const CCashAcceptor::TStatusSet aSourceStatuses) {
    CCashAcceptor::SStatusSpecification &lastStatusHistory = this->m_StatusHistory.last();

    CCashAcceptor::TStatusSet sourceStatuses =
        aSourceStatuses + (CCashAcceptor::TStatusSet() << aTargetStatus);

    foreach (ECashAcceptorStatus::Enum status, sourceStatuses) {
        if (!aStatuses[status].isEmpty()) {
            lastStatusHistory.statuses[aTargetStatus].unite(aStatuses[status]);
        }
    }

    lastStatusHistory.warningLevel =
        qMax(lastStatusHistory.warningLevel,
             this->m_StatusCodesSpecification->warningLevelByStatus(aTargetStatus));
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::emitStatuses(CCashAcceptor::SStatusSpecification &aSpecification,
                                       const CCashAcceptor::TStatusSet &aSet) {
    if (!this->m_PostPollingAction) {
        this->m_StatusHistory.saveLevel();
        return;
    }

    foreach (ECashAcceptorStatus::Enum currentStatus, aSet) {
        if (aSpecification.statuses.contains(currentStatus) &&
            (currentStatus != ECashAcceptorStatus::Escrow) &&
            (currentStatus != ECashAcceptorStatus::Stacked)) {
            TStatusCollection statusCollection =
                this->getStatusCollection(aSpecification.statuses[currentStatus]);
            this->emitStatusCodes(statusCollection, currentStatus);
        }
    }

    this->m_StatusHistory.updateLevel();
}

//--------------------------------------------------------------------------------
template <class T>
void CashAcceptorBase<T>::sendStatuses(const TStatusCollection &aNewStatusCollection,
                                       const TStatusCollection &aOldStatusCollection) {
    // Предполагается, что дублирующие и недостоверные статусы уже отфильтрованы на предыдущем шаге.
    // Конвертируем warning-уровни в статус-уровни.
    CCashAcceptor::TStatuses statuses = this->getLastStatuses();

    this->m_StatusHistory.append(CCashAcceptor::SStatusSpecification());
    CCashAcceptor::SStatusSpecification &lastStatusHistory = this->m_StatusHistory.last();
    CCashAcceptor::TStatuses &lastStatuses = lastStatusHistory.statuses;

    // 1. Сначала Stacked, т.к. по нему начисляется сумма.
    if (!statuses.isEmpty(ECashAcceptorStatus::Stacked) &&
        (this->m_Statuses.isEmpty(ECashAcceptorStatus::Stacked) ||
         this->getConfigParameter(CHardware::CashAcceptor::StackedFilter).toBool())) {
        this->saveStatuses(statuses, ECashAcceptorStatus::Stacked);

        foreach (auto par, this->m_EscrowPars) {
            this->toLog(LogLevel::Normal,
                        QString("Send statuses: Stacked, note = %1, currency = %2")
                            .arg(par.nominal)
                            .arg(par.currencyId));

            Q_EMIT this->stacked(TParList() << par);
        }

        this->m_EscrowPars.clear();
    }

    // 2. Потом - Escrow.
    if (statuses.size(ECashAcceptorStatus::Escrow) >
        this->m_Statuses.size(ECashAcceptorStatus::Escrow)) {
        this->saveStatuses(statuses, ECashAcceptorStatus::Escrow);

        if (this->m_PostPollingAction) {
            SPar par = this->m_EscrowPars[0];
            this->toLog(LogLevel::Normal,
                        QString("Send statuses: Escrow, note = %1, currency = %2")
                            .arg(par.nominal)
                            .arg(par.currencyId));

            Q_EMIT this->escrow(par);
        }
    }

    // 3. Спец. статусы и статистика
    TStatusCodes badSpecialStatusCodes;
    ECashAcceptorStatus::Enum specialStatus = ECashAcceptorStatus::OK;

    foreach (ECashAcceptorStatus::Enum status, CCashAcceptor::Set::SpecialStatuses) {
        if (!statuses.isEmpty(status)) {
            if (CCashAcceptor::Set::BadSpecialStatuses.contains(status)) {
                badSpecialStatusCodes.unite(statuses.value(status));
                specialStatus =
                    qMax(specialStatus, CCashAcceptor::SpecialStatus::Specification[status]);
            }

            if (statuses.value(status) != this->m_Statuses.value(status)) {
                this->saveStatuses(statuses, status);
                this->emitStatuses(lastStatusHistory, CCashAcceptor::TStatusSet() << status);
            }
        }
    }

    // 4. Остальные статусы.
    // Если есть ErrorStatus, все warning-и подтягиваем к error-ам, а если валидатор либо в Busy,
    // либо в Rejected, либо в Cheated, при этом ошибок нет, то ждем, чем дело кончится.
    // Соответственно, если сменился error на warning, то эмитим warning с WarningLevel-ом Error;
    // если сменился error на OK, но есть либо Reject, либо Busy, то  ничего не эмитим. Если есть и
    // MechanicFailure-ы, и Error-ы, то Error-ы сливаем в MechanicFailure-ы, последние хуже, из-за
    // подозрения, что купюра застряла в терминале.

    QList<ECashAcceptorStatus::Enum> badStatusList = QList<ECashAcceptorStatus::Enum>(
        CCashAcceptor::Set::BadStatuses.begin(), CCashAcceptor::Set::BadStatuses.end());
    std::sort(badStatusList.begin(), badStatusList.end());

    auto saveBadStatuses = [&](ECashAcceptorStatus::Enum aStatus) {
        ECashAcceptorStatus::Enum totalStatus = qMax(specialStatus, aStatus);
        statuses[totalStatus].unite(badSpecialStatusCodes);
        QList<ECashAcceptorStatus::Enum> statusList =
            badStatusList.mid(0, badStatusList.indexOf(totalStatus));

        saveStatuses(statuses,
                     totalStatus,
                     QSet<ECashAcceptorStatus::Enum>(statusList.begin(), statusList.end()));
    };

    // 4.1. Сохраняем значимые статусы
    if (!statuses.isEmpty(ECashAcceptorStatus::MechanicFailure)) {
        saveBadStatuses(ECashAcceptorStatus::MechanicFailure);
    } else if (!statuses.isEmpty(ECashAcceptorStatus::Error)) {
        saveBadStatuses(ECashAcceptorStatus::Error);
    } else if (!statuses.isEmpty(ECashAcceptorStatus::Warning)) {
        saveBadStatuses(ECashAcceptorStatus::Warning);
    } else {
        this->saveStatuses(statuses, ECashAcceptorStatus::OK, CCashAcceptor::Set::NormalStatuses);

        if (!QSet<ECashAcceptorStatus::Enum>(lastStatuses.keys().begin(), lastStatuses.keys().end())
                 .intersect(CCashAcceptor::Set::NormalStatuses)
                 .isEmpty()) {
            foreach (ECashAcceptorStatus::Enum status, CCashAcceptor::Set::NormalStatuses) {
                lastStatuses.remove(status);
            }

            lastStatuses[ECashAcceptorStatus::OK].insert(DeviceStatusCode::OK::OK);
        }
    }

    // 4.2. Принимаем решение об эмите
    CCashAcceptor::SStatusSpecification beforeLastStatusSpec = this->m_StatusHistory.lastValue(2);
    CCashAcceptor::TStatuses &beforeLastStatuses = beforeLastStatusSpec.statuses;

    TStatusCodes beforeStatusCodes;
    TStatusCodes newStatusCodes;

    foreach (ECashAcceptorStatus::Enum status, CCashAcceptor::Set::GeneralStatuses) {
        if (beforeLastStatuses.contains(status) && !beforeLastStatuses.isEmpty(status)) {
            beforeStatusCodes.unite(beforeLastStatuses.value(status));
        }

        newStatusCodes.unite(lastStatuses.value(status));
    }

    bool emitSignal = !newStatusCodes.isEmpty();

    // если выходим из ошибки, но купюроприемник чем-то занят - ничего не делаем, ждем, пока
    // закончит
    if ((beforeLastStatusSpec.warningLevel == EWarningLevel::Error) &&
        !beforeLastStatuses.isEmpty() && (lastStatusHistory.warningLevel != EWarningLevel::Error)) {
        TStatusCodes statusCodes;

        foreach (ECashAcceptorStatus::Enum status, CCashAcceptor::Set::BusyStatuses) {
            statusCodes.unite(statuses.value(status));
        }

        if (!statusCodes.isEmpty()) {
            this->toLog(LogLevel::Warning,
                        this->m_DeviceName + " is busy, waiting for change status...");
            lastStatusHistory.warningLevel = EWarningLevel::Error;

            emitSignal = false;
        }
    }

    if (emitSignal && !beforeLastStatuses.isEmpty() &&
        (beforeLastStatusSpec.warningLevel == lastStatusHistory.warningLevel)) {
        emitSignal = (beforeStatusCodes != newStatusCodes) || aOldStatusCollection.isEmpty();
    }

    emitSignal = emitSignal || this->environmentChanged();
    bool statusChanged = aNewStatusCollection != aOldStatusCollection;

    if (statusChanged) {
        this->toLog(LogLevel::Normal,
                    QString("Signal emitting is %1allowed, post polling action is %2enabled")
                        .arg(emitSignal ? "" : "not ")
                        .arg(this->m_PostPollingAction ? "" : "not "));

        QString debugLog = "Status history :";

        for (int i = 0; i < this->m_StatusHistory.size(); ++i) {
            CCashAcceptor::SStatusSpecification statusSpecification = this->m_StatusHistory[i];
            QString warningLevel =
                (statusSpecification.warningLevel == EWarningLevel::Error)
                    ? "Error"
                    : ((statusSpecification.warningLevel == EWarningLevel::Warning) ? "Warning"
                                                                                    : "OK");

            QString statusLog;

#define DEBUG_DECLARE_CA_STATUS(aStatus)                                                           \
    QString debug##aStatus = this->getStatusTranslations(                                          \
        statusSpecification.statuses.value(ECashAcceptorStatus::aStatus), false);                  \
    QString name##aStatus = #aStatus;                                                              \
    name##aStatus += QString(15 - name##aStatus.size(), QChar(' '));                               \
    if (!debug##aStatus.isEmpty())                                                                 \
        statusLog += QString("\n%1 : %2").arg(name##aStatus).arg(debug##aStatus);

            DEBUG_DECLARE_CA_STATUS(OK);
            DEBUG_DECLARE_CA_STATUS(Escrow);
            DEBUG_DECLARE_CA_STATUS(Stacked);

            DEBUG_DECLARE_CA_STATUS(Warning);

            DEBUG_DECLARE_CA_STATUS(Error);
            DEBUG_DECLARE_CA_STATUS(MechanicFailure);
            DEBUG_DECLARE_CA_STATUS(StackerFull);
            DEBUG_DECLARE_CA_STATUS(StackerOpen);

            DEBUG_DECLARE_CA_STATUS(Cheated);
            DEBUG_DECLARE_CA_STATUS(Rejected);

            DEBUG_DECLARE_CA_STATUS(Inhibit);
            DEBUG_DECLARE_CA_STATUS(Disabled);
            DEBUG_DECLARE_CA_STATUS(Enabled);
            DEBUG_DECLARE_CA_STATUS(BillOperation);
            DEBUG_DECLARE_CA_STATUS(Busy);
            DEBUG_DECLARE_CA_STATUS(OperationError);
            DEBUG_DECLARE_CA_STATUS(Unknown);

            debugLog +=
                QString("\n [%1] : warning level = %2%3").arg(i).arg(warningLevel).arg(statusLog);
        }

        this->toLog(LogLevel::Normal, debugLog);
    }

    // 4.3. Если надо - эмитим статус
    if (emitSignal) {
        this->emitStatuses(lastStatusHistory, CCashAcceptor::Set::GeneralStatuses);
    } else if ((lastStatusHistory == beforeLastStatusSpec) ||
               QSet<ECashAcceptorStatus::Enum>(lastStatusHistory.statuses.keys().begin(),
                                               lastStatusHistory.statuses.keys().end())
                   .intersect(CCashAcceptor::Set::MainStatuses)
                   .isEmpty()) {
        this->m_StatusHistory.removeLast();
    }

    this->m_StatusHistory.updateLevel(true);

    if (statusChanged) {
        this->toLog(LogLevel::Normal,
                    QString("Status history: level = %1, size = %2")
                        .arg(this->m_StatusHistory.getLevel())
                        .arg(this->m_StatusHistory.size()));
    }

    this->m_Statuses = statuses;

    if (this->m_StatusHistory.isEmpty()) {
        this->m_Ready = false;
    } else {
        CCashAcceptor::TStatuses statusBuffer = this->m_StatusHistory.lastValue().statuses;
        CCashAcceptor::TStatusSet statusSet =
            QSet<ECashAcceptorStatus::Enum>(statusBuffer.keys().begin(), statusBuffer.keys().end());

        this->m_Ready = (this->m_Initialized == ERequestStatus::Success) && !statusSet.isEmpty() &&
                       statusSet.intersect(CCashAcceptor::Set::ErrorStatuses).isEmpty() &&
                       !statusBuffer.contains(BillAcceptorStatusCode::Busy::Pause);
    }
}

//--------------------------------------------------------------------------------
