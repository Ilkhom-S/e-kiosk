/* @file Имена параметров сервисов. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
namespace CServiceParameters {
namespace Networking {
extern const char SimBalance[];
extern const char SignalLevel[];
extern const char Provider[];
} // namespace Networking

namespace Funds {
extern const char RejectCount[];
} // namespace Funds

namespace Printing {
extern const char ReceiptCount[];
extern const char SessionStatus[];
extern const char ZReportCount[];
} // namespace Printing

namespace Payment {
extern const char UnprocessedPaymentCount[];
extern const char PaymentsPerDay[];
} // namespace Payment

namespace Terminal {
extern const char RestartCount[];
} // namespace Terminal
} // namespace CServiceParameters

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
