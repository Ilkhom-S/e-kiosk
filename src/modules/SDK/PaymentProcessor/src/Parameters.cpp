/* @file Definitions for Payment Parameters constants. */

#include <SDK/PaymentProcessor/Payment/Parameters.h>

namespace SDK {
namespace PaymentProcessor {
namespace CPayment {
namespace Parameters {

// Обязательные параметры.
extern const char ID[] = "ID";
extern const char Type[] = "PROCESSING_TYPE";
extern const char CreationDate[] = "CREATION_DATE";
extern const char LastUpdateDate[] = "LAST_UPDATE_DATE";
extern const char CompleteDate[] = "COMPLETE_DATE";
extern const char Provider[] = "PROVIDER";
extern const char Status[] = "STATUS";
extern const char Priority[] = "PRIORITY";
extern const char InitialSession[] = "INITIAL_SESSION";
extern const char Amount[] = "AMOUNT";
extern const char AmountAll[] = "AMOUNT_ALL";
extern const char CRC[] = "CRC";
extern const char Cheated[] = "CHEATED";

// Опциональные параметры.
extern const char Change[] = "CHANGE";
extern const char Fee[] = "FEE";
extern const char DealerFee[] = "DEALER_FEE";
extern const char ProcessingFee[] = "PROCESSING_FEE";
extern const char Session[] = "SESSION";
extern const char Step[] = "STEP";
extern const char ServerError[] = "SERVER_ERROR";
extern const char ServerResult[] = "SERVER_RESULT";
extern const char ErrorMessage[] = "ERROR_MESSAGE";
extern const char NumberOfTries[] = "NUMBER_OF_TRIES";
extern const char NextTryDate[] = "NEXT_TRY_DATE";
extern const char Signature[] = "SIGNATURE";
extern const char AddInfo[] = "ADDINFO";
extern const char TransactionId[] = "TRANSID";
extern const char AuthCode[] = "AUTHCODE";
extern const char Vat[] = "VAT";

extern const char PayTool[] = "PAY_TOOL";

// Вспомогательные параметры.
extern const char MinAmount[] = "MIN_AMOUNT";
extern const char MaxAmount[] = "MAX_AMOUNT";
extern const char MaxAmountAll[] = "MAX_AMOUNT_ALL";
extern const char AcceptAmount[] = "ACCEPT_AMOUNT";
extern const char ProviderFields[] = "PROVIDER_FIELDS";
extern const char ProviderFieldsExt[] = "PROVIDER_FIELDS_EXT";
extern const char ProviderFieldsDelimiter[] = "#";
extern const char ReceiptPrinted[] = "RECEIPT_PRINTED";
extern const char OriginalPayment[] = "ORIGINAL_PAYMENT";
extern const char BlockUpdateLimits[] = "BLOCK_UPDATE_LIMITS";

// MNP
extern const char MNPGatewayIn[] = "GATEWAY_IN";
extern const char MNPGatewayOut[] = "GATEWAY_OUT";

} // namespace Parameters
} // namespace CPayment
} // namespace PaymentProcessor
} // namespace SDK
