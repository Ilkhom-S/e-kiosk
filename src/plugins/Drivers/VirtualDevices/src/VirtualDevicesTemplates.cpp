/* @file Инстанцирование шаблонов виртуальных устройств. */

// System
#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"

template class CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>;
template class DispenserBase<DeviceBase<ProtoDispenser>>;

//------------------------------------------------------------------------------
