/* @file Константы для печати чеков. */

#include "PrintConstants.h"

namespace CPrintConstants {
const char BankName[] = "BANK_NAME";
const char BankBik[] = "BANK_BIK";
const char BankInn[] = "BANK_INN";
const char BankAddress[] = "BANK_ADDRESS";
const char BankPhone[] = "BANK_PHONE";
const char DealerAddress[] = "DEALER_ADDRESS";
const char DealerBusinessAddress[] = "DEALER_BUSINESS_ADDRESS";
const char DealerInn[] = "DEALER_INN";
const char DealerKbk[] = "DEALER_KBK";
const char DealerName[] = "DEALER_NAME";
const char DealerPhone[] = "DEALER_PHONE";
const char DealerIsBank[] = "DEALER_IS_BANK";
const char DealerSupportPhone[] = "DEALER_SUPPORT_PHONE";
const char DealerVAT[] = "DEALER_NDS";
const char DealerAgentFlag[] = "DEALER_AGENT_FLAG";
const char DealerTaxSystem[] = "DEALER_SNO";
const char FiscalData[] = "FISCAL_DATA";
const char MtRegistrationAddress[] = "MT_REGISTRATION_ADDRESS";
const char PointAddress[] = "POINT_ADDRESS";
const char PointName[] = "POINT_NAME";
const char PointExternalID[] = "POINT_EXTERNAL_ID";
const char OpBrand[] = "OPERATOR_BRAND";
const char OpName[] = "OPERATOR_NAME";
const char OpINN[] = "OPERATOR_INN";
const char OpPhone[] = "OPERATOR_PHONE";
const char RecipientInn[] = "RECIPIENT_INN";
const char RecipientName[] = "RECIPIENT_NAME";
const char ServiceType[] = "SERVICE_TYPE";
const char Term_Number[] = "TERMINAL_NUMBER";
const char Currency[] = "CURRENCY";
const char ContractNumber[] = "CONTRACT_NUMBER";
const char DateTime[] = "DATETIME";
const char ReceiptNumber[] = "RECEIPT_NUMBER";
const char NoFiscal[] = "NO_FISCAL";

namespace KKM {
const char TaxSystem[] = "TAXSYSTEM";              // система налогообложения (СНО)
const char DateTimeStamp[] = "KKM_DATETIME_STAMP"; // дата и время получения фискального документа
const char SerialNumber[] = "KKM_SERIAL_NUMBER"; // серийный номер фискальника (заводской номер ККТ)
const char RNM[] = "KKM_RNM";                    // регистрационный номер ККТ (РНМ)
const char SessionNumber[] = "KKM_SESSION_NUMBER";    // номер смены
const char FDSerialNumber[] = "KKM_FD_SERIAL_NUMBER"; // порядковый номер фискального чека
const char FSNumber[] = "KKM_FS_NUMBER";              // заводской номер фискального накопителя
const char FDNumber[] = "KKM_FD_NUMBER";              // номер фискального чека
const char FDSign[] = "KKM_FD_SIGN";                  // фискальный признак данных

const char TaxAmount02[] = "TAX_AMOUNT_02"; // сумма НДС чека по ставке 18(20)% (1102)
const char TaxAmount03[] = "TAX_AMOUNT_03"; // сумма НДС чека по ставке 10% (1103)
const char TaxAmount04[] = "TAX_AMOUNT_04"; // сумма расчета по чеку с НДС по ставке 0% (1104)
const char TaxAmount05[] = "TAX_AMOUNT_05"; // сумма расчета по чеку без НДС (1105)
} // namespace KKM
} // namespace CPrintConstants