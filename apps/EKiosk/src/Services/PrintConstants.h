/* @file Менеджер конфигураций. */

#pragma once

//---------------------------------------------------------------------------
/// Константы для печати чеков.
namespace CPrintConstants {
extern const char BankName[];
extern const char BankBik[];
extern const char BankInn[];
extern const char BankAddress[];
extern const char BankPhone[];
extern const char DealerAddress[];
extern const char DealerBusinessAddress[];
extern const char DealerInn[];
extern const char DealerKbk[];
extern const char DealerName[];
extern const char DealerPhone[];
extern const char DealerIsBank[];
extern const char DealerSupportPhone[];
extern const char DealerVAT[];
extern const char DealerAgentFlag[];
extern const char DealerTaxSystem[];
extern const char FiscalData[];
extern const char MtRegistrationAddress[];
extern const char PointAddress[];
extern const char PointName[];
extern const char PointExternalID[];
extern const char OpBrand[];
extern const char OpName[];
extern const char OpINN[];
extern const char OpPhone[];
extern const char RecipientInn[];
extern const char RecipientName[];
extern const char ServiceType[];
extern const char TermNumber[];
extern const char Currency[];
extern const char ContractNumber[];
extern const char DateTime[];
extern const char ReceiptNumber[];
extern const char NoFiscal[];

namespace KKM {
extern const char TaxSystem[]; // система налогообложения (СНО)
extern const char DateTimeStamp[]; // дата и время получения фискального документа
extern const char SerialNumber[]; // серийный номер фискальника (заводской номер ККТ)
extern const char RNM[]; // регистрационный номер ККТ (РНМ)
extern const char SessionNumber[]; // номер смены
extern const char FDSerialNumber[]; // порядковый номер фискального чека
extern const char FSNumber[]; // заводской номер фискального накопителя
extern const char FDNumber[]; // номер фискального чека
extern const char FDSign[]; // фискальный признак данных

extern const char TaxAmount02[]; // сумма НДС чека по ставке 18(20)% (1102)
extern const char TaxAmount03[]; // сумма НДС чека по ставке 10% (1103)
extern const char TaxAmount04[]; // сумма расчета по чеку с НДС по ставке 0% (1104)
extern const char TaxAmount05[]; // сумма расчета по чеку без НДС (1105)
} // namespace KKM
} // namespace CPrintConstants

//---------------------------------------------------------------------------
