/* @file Константы купюроприемника на протоколе CCNet. */

#include "CCNetCashAcceptorConstants.h"

namespace CCCNet {
namespace Commands {
const char Reset[] = "\x30";
const char SetSecurity[] = "\x32";
const char GetStatus[] = "\x33";
const char EnableBillTypes[] = "\x34";
const char Stack[] = "\x35";
const char Return[] = "\x36";
const char GetVersion[] = "\x37";
const char GetParList[] = "\x41";
const char UpdateFirmware[] = "\x50";

namespace UpdatingFirmware {
const char GetBlockSize[] = "\x50\x01";
const char Write[] = "\x50\x02";
const char Exit[] = "\x50\x03";
const char SetBaudRate[] = "\x50\x05";
} // namespace UpdatingFirmware
} // namespace Commands
} // namespace CCCNet
