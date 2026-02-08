/* @file Константы купюроприемника на протоколе CCNet. */

#include "CCNetCashAcceptorConstants.h"

namespace CCCNet {
namespace Commands {
const char Reset[] = R"(0)";
const char SetSecurity[] = R"(2)";
const char GetStatus[] = R"(3)";
const char EnableBillTypes[] = R"(4)";
const char Stack[] = R"(5)";
const char Return[] = R"(6)";
const char GetVersion[] = R"(7)";
const char GetParList[] = R"(A)";
const char UpdateFirmware[] = R"(P)";

namespace UpdatingFirmware {
const char GetBlockSize[] = "\x50\x01";
const char Write[] = "\x50\x02";
const char Exit[] = "\x50\x03";
const char SetBaudRate[] = "\x50\x05";
} // namespace UpdatingFirmware
} // namespace Commands
} // namespace CCCNet
