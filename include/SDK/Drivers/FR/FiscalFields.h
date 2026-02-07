/* @file Фискальные реквизиты). */

#pragma once

namespace SDK {
namespace Driver {
namespace CAllHardware {
/// Фискальные реквизиты.
namespace FiscalFields {
/// Наименование фискального документа.
extern const char FDName[]; // 1000 (Наименование фискального документа).
/// Телефон или электронный адрес покупателя.
extern const char UserContact[]; // 1008 (Телефон или электронный адрес покупателя).
/// Адрес расчетов.
extern const char PayOffAddress[]; // 1009 (Адрес расчетов).
/// Дата и время ФД.
extern const char FDDateTime[]; // 1012 (Дата и время ФД).
/// Заводской номер ФР.
extern const char SerialFRNumber[]; // 1013 (Заводской номер ФР).
/// ИНН ОФД.
extern const char OFDINN[]; // 1017 (ИНН ОФД).
/// ИНН пользователя.
extern const char INN[]; // 1018 (ИНН пользователя).
/// Сумма расчета в чеке.
extern const char PayOffAmount[]; // 1020 (Сумма расчета в чеке).
/// Кассир.
extern const char Cashier[]; // 1021 (Кассир).
/// Наименование товара.
extern const char UnitName[]; // 1030 (Наименование товара).
/// Номер автомата.
extern const char AutomaticNumber[]; // 1036 (Номер автомата).
/// Регистрационный номер ККТ.
extern const char RNM[]; // 1037 (Регистрационный номер ККТ).
/// Номер смены.
extern const char SessionNumber[]; // 1038 (Номер смены).
/// Номер ФД.
extern const char FDNumber[]; // 1040 (Номер ФД).
/// Заводской номер ФН.
extern const char SerialFSNumber[]; // 1041 (Заводской номер ФН).
/// Номер чека за смену.
extern const char DocumentNumber[]; // 1042 (Номер чека за смену).
/// Наименование ОФД.
extern const char OFDName[]; // 1046 (Наименование ОФД).
/// Наименование юр. лица владельца.
extern const char LegalOwner[]; // 1048 (Наименование юр. лица владельца).
/// Признак расчета.
extern const char PayOffType[]; // 1054 (Признак расчета).
/// СНО на платеже.
extern const char TaxSystem[]; // 1055 (СНО на платеже).
/// Адрес сайта ФНС.
extern const char FTSURL[]; // 1060 (Адрес сайта ФНС).
/// СНО из итогов регистрации.
extern const char TaxSystemsReg[]; // 1062 (СНО из итогов регистрации).
/// Телефон оператора по приему платежей.
extern const char ProcessingPhone[]; // 1074 (Телефон оператора по приему платежей).
/// Фискальный признак документа.
extern const char FDSign[]; // 1077 (Фискальный признак документа).
/// Количество непереданных ФД.
extern const char OFDNotSentFDQuantity[]; // 1097 (Количество непереданных ФД).
/// Дата и время первого из непереданных ФД.
extern const char OFDNotSentFDDateTime[]; // 1098 (Дата и время первого из непереданных ФД).
/// Код причины перерегистрации.
extern const char ReregistrationCause[]; // 1101 (Код причины перерегистрации).
/// Общее количество ФД за смену.
extern const char FDForSessionTotal[]; // 1111 (Общее количество ФД за смену).
/// Электронная почта отправителя чека.
extern const char SenderMail[]; // 1117 (Электронная почта отправителя чека).
/// Количество кассовых чеков (БСО) за смену.
extern const char FiscalsForSessionTotal[]; // 1118 (Количество кассовых чеков (БСО) за смену).
/// Место расчетов.
extern const char PayOffPlace[]; // 1187 (Место расчетов).
/// Версия модели ККТ.
extern const char ModelVersion[]; // 1188 (Версия модели ККТ).
/// Версия ФФД ФР.
extern const char FFDFR[]; // 1189 (Версия ФФД ФР).
/// Версия ФФД ФН.
extern const char FFDFS[]; // 1190 (Версия ФФД ФН).
/// Ставка НДС.
extern const char VATRate[]; // 1199 (Ставка НДС).
/// ИНН кассира.
extern const char CashierINN[]; // 1203 (ИНН кассира).
/// Адрес сайта для получения чека.
extern const char OFDURL[]; // 1208 (Адрес сайта для получения чека).
/// Версия ФФД.
extern const char FFD[]; // 1209 (Версия ФФД).

// Данные оператора перевода
extern const char TransferOperatorAddress[]; // 1005 (Адрес оператора перевода).
extern const char TransferOperatorINN[];     // 1016 (ИНН оператора перевода).
extern const char TransferOperatorName[];    // 1026 (Наименование оператора перевода).
extern const char TransferOperatorPhone[];   // 1075 (Телефон оператора перевода).

// Данные поставщика
extern const char ProviderPhone[]; // 1171 (Телефон поставщика).
extern const char ProviderINN[];   // 1226 (ИНН поставщика).

// Данные платежного агента
extern const char AgentOperation[]; // 1044 (Операция платежного агента).
extern const char
    AgentFlagsReg[]; // 1057 (Признак(и) платежного агента из итогов регистрации и на платеже).
extern const char AgentPhone[]; // 1073 (Телефон платежного агента).
extern const char AgentFlag[];  // 1222 (Признак платежного агента на платеже).

// Статусы
extern const char FSExpiredStatus[];    // 1050 (Признак исчерпания ресурса ФН).
extern const char FSNeedChangeStatus[]; // 1051 (Признак необходимости срочной замены ФН).
extern const char FSMemoryEnd[];        // 1052 (Признак заполнения памяти ФН).
extern const char OFDNoConnection[];    // 1053 (Признак превышения времени ожидания ответа ОФД).

// Режимы работы
extern const char AutomaticMode[];      // 1001 (Признак автоматического режима).
extern const char AutonomousMode[];     // 1002 (Признак автономного режима).
extern const char EncryptionMode[];     // 1056 (Признак шифрования).
extern const char InternetMode[];       // 1108 (Признак работы с интернет (без принтера).
extern const char ServiceAreaMode[];    // 1109 (Признак применения в сфере услуг).
extern const char FixedReportingMode[]; // 1110 (Признак работы с бланками строгой отчетности (БСО).
extern const char LotteryMode[];        // 1126 (Признак проведения лотереи).
extern const char GamblingMode[];       // 1193 (Признак проведения азартных игр).
extern const char ExcisableUnitMode[];  // 1207 (Признак торговли подакцизными товарами).
extern const char InAutomateMode[];     // 1221 (Признак установки в автомате).

// Предмет расчета (на конкретную продажу)
extern const char PayOffSubjectQuantity[]; // 1023 (Количество предмета расчета).
extern const char PayOffSubjectAmount[];   // 1043 (Стоимость предмета расчета).
extern const char PayOffSubject[];         // 1059 (Предмет расчета).
extern const char
    PayOffSubjectUnitPrice[]; // 1079 (Цена за единицу предмета расчета с учетом скидок и наценок).
extern const char PayOffSubjectTaxAmount[];  // 1200 (Сумма НДС за предмет расчета).
extern const char PayOffSubjectType[];       // 1212 (Признак предмета расчета).
extern const char PayOffSubjectMethodType[]; // 1214 (Признак способа расчета).

// Суммы по способу расчета (на весь чек)
extern const char CashFiscalTotal[]; // 1031 (Сумма по чеку (БСО) наличными).
extern const char CardFiscalTotal[]; // 1081 (Сумма по чеку (БСО) электронными).
extern const char
    PrePaymentFiscalTotal[]; // 1215 (Сумма по чеку (БСО) предоплатой (зачетом аванса)).
extern const char PostPaymentFiscalTotal[]; // 1216 (Сумма по чеку (БСО) постоплатой (в кредит)).
extern const char
    CounterOfferFiscalTotal[]; // 1217 (Сумма по чеку (БСО) встречным предоставлением).

// Налоги (на весь чек)
extern const char TaxAmount02[]; // 1102 (Сумма НДС чека по ставке 18(20)%).
extern const char TaxAmount03[]; // 1103 (Сумма НДС чека по ставке 10%).
extern const char TaxAmount04[]; // 1104 (Сумма расчета по чеку с НДС по ставке 0%).
extern const char TaxAmount05[]; // 1105 (Сумма расчета по чеку без НДС).
extern const char TaxAmount06[]; // 1106 (Сумма НДС чека по расчетной ставке 18/118 (20/120)).
extern const char TaxAmount07[]; // 1107 (Сумма НДС чека по расчетной ставке 10/110).

// Значения.
namespace Values {
/// ФФД маркер отсутствия данных.
extern const char NoData[];
} // namespace Values

} // namespace FiscalFields
} // namespace CAllHardware
} // namespace Driver
} // namespace SDK

namespace CFiscalSDK = SDK::Driver::CAllHardware::FiscalFields;

//--------------------------------------------------------------------------------
