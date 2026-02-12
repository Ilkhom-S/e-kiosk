/* @file Константы купюроприемника CCNet Creator. */

#include "CCNetCreatorConstants.h"

namespace CCCNetCreator {
namespace Commands {
namespace UpdatingFirmware {
const char SetBaudRate[] = "\xA0";
const char WriteHead[] = "\xA1";
const char WriteBlock[] = "\xA2";
const char Exit[] = "\xA3";
} // namespace UpdatingFirmware

const char GetInternalVersion[] = R"(p)";
const char GetSerial[] = R"(r)";
} // namespace Commands
} // namespace CCCNetCreator
