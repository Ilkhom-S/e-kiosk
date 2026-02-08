/* @file Ресайклер на протоколе CCNet. */

#include "CCNetRecycler.h"

#include "CCNetCashAcceptorConstants.h"

//---------------------------------------------------------------------------
CCNetRecycler::CCNetRecycler() {
    // данные устройства
    m_DeviceName = CCCNet::Models::CashcodeG200;
    m_SupportedModels = QStringList() << m_DeviceName;
    m_ResetWaiting = EResetWaiting::Full;

    setConfigParameter(CHardware::CashAcceptor::InitializeTimeout,
                       CCCNetRecycler::ExitInitializeTimeout);

    // параметры протокола
    m_Protocol.setAddress(CCCNet::Addresses::BillToBill);
}

//--------------------------------------------------------------------------------
bool CCNetRecycler::processReset() {
    bool result = processCommand(CCCNet::Commands::Reset);

    if (!waitNotBusyPowerUp()) {
        result = processCommand(CCCNet::Commands::Reset);
        waitNotBusyPowerUp();
    }

    return result || !m_ForceWaitResetCompleting;
}

//--------------------------------------------------------------------------------
