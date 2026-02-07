/* @file Константы принтера AV268. */

#include "AV268Constants.h"

//--------------------------------------------------------------------------------
namespace CAV268 {
/// Пакеты команд.
namespace Commands {
const char Initialize[] = "\x1B\x40";
const char GetStatus[] = "\x1B\x76";
const char GetSettings[] = "\x1D\xFB";
const char GetPresenterStatus[] = "\x1D\xFC";
} // namespace Commands

namespace Answers {
const char GetSettings[] = "\xEB\xCF";
} // namespace Answers
} // namespace CAV268