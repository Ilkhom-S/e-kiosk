/* @file Константы купюроприемника CCNet Creator. */

#include "CCNetCreatorConstants.h"

namespace CCCNet {
namespace Commands {
namespace UpdatingFirmware {
const char SetBaudRate[] = "\xA0";
const char WriteHead[] = "\xA1";
const char WriteBlock[] = "\xA2";
const char Exit[] = "\xA3";
} // namespace UpdatingFirmware

const char GetInternalVersion[] = "\x70";
const char GetSerial[] = "\x72";
} // namespace Commands
} // namespace CCCNet
