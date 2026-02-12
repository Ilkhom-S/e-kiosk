/* @file Константы купюроприемника на протоколе ICT. */

#include "ICTCashAcceptorConstants.h"

namespace CICTBase {
namespace Answers {
const char Identification[] = R"(^>^)";
} // namespace Answers

namespace States {
const char PowerUp[] = "\x80\x8F";
} // namespace States
} // namespace CICTBase
