/* @file Константы купюроприемника на протоколе EBDS. */

#include "EBDSCashAcceptorConstants.h"

namespace CEBDS {
const char AdvancedModelTag[] = "SCN";

namespace Commands {
const char GetPar[] = "\x70\x02";
const char SetInhibits[] = "\x70\x03";
} // namespace Commands
} // namespace CEBDS
