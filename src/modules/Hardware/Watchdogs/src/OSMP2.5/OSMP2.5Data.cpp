/* @file Данные протокола сторожевого таймера OSMP 2.5. */

#include "OSMP2.5Data.h"

//----------------------------------------------------------------------------
namespace COSMP25 {
const char ModelTag[] = "2.5.";
const char TimeLogFormat[] = "hh:mm:ss";

//----------------------------------------------------------------------------
namespace Commands {
const char SetModem_Pause[] = "\x02\x02"; /// Установка времени сброса модема
const char SetPingTimeout[] = "\x03\x02"; /// Установка времени сторожа таймера
const char SetPCPause[] = "\x0C\x02";     /// Установка таймаута сброса PC
} // namespace Commands
} // namespace COSMP25
