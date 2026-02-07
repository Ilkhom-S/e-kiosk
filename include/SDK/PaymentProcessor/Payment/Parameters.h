/* @file Список обязательных для каждого платежа полей. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
namespace CPayment {
namespace Parameters {
/// Обязательные параметры.
extern const char ID[];
extern const char Type[];
extern const char CreationDate[];
extern const char LastUpdateDate[];
extern const char CompleteDate[];
extern const char Provider[];
extern const char Status[];
extern const char Priority[];
extern const char InitialSession[];
extern const char Amount[];
extern const char AmountAll[];
extern const char CRC[];
extern const char Cheated[];

/// Опциональные параметры.
extern const char Change[];
extern const char Fee[];
extern const char DealerFee[];
extern const char ProcessingFee[];
extern const char Session[];
extern const char Step[];
extern const char ServerError[];
extern const char ServerResult[];
extern const char ErrorMessage[];
extern const char NumberOfTries[];
extern const char NextTryDate[];
extern const char Signature[];
extern const char AddInfo[];
extern const char TransactionId[];
extern const char AuthCode[];
extern const char Vat[];

/*
PAY_TOOL = N – тип оплаты :
0 – наличные средства,
        1 – по банковской карте, эмитированной Банком - партнером(«свои» карты),
        2 – по банковской карте, не эмитированной Банком - партнером(«чужие» карты).
        В случае если Контрагент, не являющийся банком, принимает платеж по
        банковской карте, значение параметра PAY_TOOL = 2.
        При отсутствии параметра значение принимается равным 0. */
extern const char PayTool[];

/// Вспомогательные параметры.
extern const char MinAmount[];
extern const char MaxAmount[];
extern const char MaxAmountAll[];
extern const char AcceptAmount[];
extern const char ProviderFields[];
extern const char ProviderFieldsExt[];
extern const char ProviderFieldsDelimiter[];
extern const char ReceiptPrinted[];
extern const char OriginalPayment[];
extern const char BlockUpdateLimits[];

/// MNP
extern const char MNPGatewayIn[];
extern const char MNPGatewayOut[];
} // namespace Parameters
} // namespace CPayment

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
