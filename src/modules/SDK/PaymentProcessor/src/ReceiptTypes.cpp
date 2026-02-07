/* @file Определения типов чеков. */

#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

namespace SDK {
namespace PaymentProcessor {
namespace CReceiptType {
extern const char Payment[] = "payment";
extern const char Error[] = "error";
extern const char Balance[] = "balance";
extern const char Encashment[] = "encashment";
extern const char ZReport[] = "z_report";
extern const char ZReportFull[] = "z_report_full";
extern const char XReport[] = "x_report";
extern const char Test[] = "test";
extern const char DispenserEncashment[] = "dispense_encashment";
extern const char DispenserBalance[] = "dispense_balance";
extern const char Disabled[] = "disabled";
} // namespace CReceiptType
} // namespace PaymentProcessor
} // namespace SDK