/* @file Устройство приема денег на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

// System
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"
#include "Hardware/CashDevices/CCTalkModelData.h"

// Project
#include "CCTalkComplexEnableAcceptor.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------
template <class T> bool CCTalkComplexEnableAcceptor<T>::applyParTable() {
    QByteArray commandData = this->getParTableData() + CCCTalk::DefaultSorterMask;

    return this->processCommand(CCCTalk::Command::ModifyInhibitsAndRegesters, commandData + commandData);
}

//--------------------------------------------------------------------------------
template <class T>
void CCTalkComplexEnableAcceptor<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                                       const TStatusCollection &aOldStatusCollection) {
    if (aOldStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) &&
        !aNewStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) &&
        this->getConfigParameter(CHardware::CashAcceptor::Enabled).toBool()) {
        this->reset(true);
        bool enabled = this->enableMoneyAcceptingMode(true);
        this->setConfigParameter(CHardware::CashAcceptor::Enabled, enabled);
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
