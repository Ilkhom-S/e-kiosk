/* @file Инстанцирование шаблонов диспенсеров. */

#include "../../../../modules/Hardware/CashDevices/src/CCTalk/CCTalkDeviceBase.cpp"
#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"
#include "Hardware/Dispensers/PortDispenser.h"

//------------------------------------------------------------------------------
template class DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>>;
template class CCTalkDeviceBase<TSerialDispenser>;
template class CCTalkDeviceBase<PortDispenser>;

//------------------------------------------------------------------------------
