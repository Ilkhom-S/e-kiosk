/* @file Константы, коды команд и ответов протокола ФР АТОЛ. */

#include "AtolFRConstants.h"

//--------------------------------------------------------------------------------
namespace CAtolFR {
const char DateTimeFormat[] = "yyyyMMddhhmmss";
const char SessionDTFormat[] = "ddMMyyyyhhmmss";

//--------------------------------------------------------------------------------
namespace Registers {
const char PaymentAmount[] = "amount of payments";
const char PaymentCount[] = "count of successful payments";
const char MoneyInCash[] = "money in cash";
const char CurrentDateTime[] = "current date and time";
const char SessionInfo[] = "last session info";
const char SerialNumber[] = "serial number";
const char NonNullableAmount[] = "non-nullable amount";
const char PrintingSettings[] = "printing settings";
} // namespace CAtolFR
