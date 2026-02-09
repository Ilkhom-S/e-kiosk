/* @file Определения констант базы данных. */

#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>

namespace SDK {
namespace PaymentProcessor {
namespace CDatabaseConstants {
namespace Devices {
extern const char Terminal[] = "Terminal";
} // namespace Devices

namespace Parameters {
extern const char DeviceName[] = "device_name";
extern const char DeviceInfo[] = "device_info";
extern const char ReceiptCount[] = "receipt_count";
extern const char BalanceLevel[] = "balance_level";
extern const char SignalLevel[] = "signal_level";
extern const char ConnectionName[] = "connection_name";
extern const char LastCheckBalanceTime[] = "last_check_balance_time";
extern const char RejectCount[] = "reject_count";
extern const char DisabledParam[] = "disabled";
extern const char LastUpdateTime[] = "last_update_time";
extern const char LaunchCount[] = "launch_count";
extern const char LastStartDate[] = "last_start_date";
extern const char Configuration[] = "configuration";
extern const char OperationSystem[] = "operation_system";
extern const char DisplayResolution[] = "display_resolution";
extern const char CashUnits[] = "cash_units";
} // namespace Parameters
} // namespace CDatabaseConstants

//---------------------------------------------------------------------------
// Definitions for CServiceParameters constants
namespace CServiceParameters {
namespace Networking {
extern const char Sim_Balance[] = "sim_balance";
extern const char SignalLevel[] = "signal_level";
extern const char Provider[] = "provider";
} // namespace Networking

namespace Funds {
extern const char RejectCount[] = "reject_count";
} // namespace Funds

namespace Printing {
extern const char ReceiptCount[] = "receipts_printed";
extern const char SessionStatus[] = "session_status";
extern const char ZReportCount[] = "z_report_count";
} // namespace Printing

namespace Payment {
extern const char UnprocessedPaymentCount[] = "unprocessed_payments";
extern const char PaymentsPerDay[] = "payments_per_day";
} // namespace Payment

namespace Terminal {
extern const char RestartCount[] = "restart_count";
} // namespace Terminal
} // namespace CServiceParameters

} // namespace PaymentProcessor
} // namespace SDK
