/* @file Инстанцирование шаблонов купюроприемников. */

#include "../../../../modules/Hardware/Acceptors/src/CCTalk/CCTalkAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashAcceptors/src/PortCashAcceptor.cpp"
#include "../../../../modules/Hardware/CashDevices/src/CCTalk/CCTalkDeviceBase.cpp"
#include "Hardware/CashAcceptors/SerialCashAcceptor.h"

template class CashAcceptorBase<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;
template class PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;
template class CCTalkDeviceBase<TSerialCashAcceptor>;
template class CCTalkAcceptorBase<TSerialCashAcceptor>;

//------------------------------------------------------------------------------
