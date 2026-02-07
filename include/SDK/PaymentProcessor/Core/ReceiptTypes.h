/* @file Типы чеков по умолчанию. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CReceiptType {
extern const char Payment[];
extern const char Error[];
extern const char Balance[];
extern const char Encashment[];
extern const char ZReport[];
extern const char ZReportFull[];
extern const char XReport[];
extern const char Test[];
extern const char DispenserEncashment[];
extern const char DispenserBalance[];
extern const char Disabled[];
} // namespace CReceiptType

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
