/* @file Константы, коды команд и ответов протокола ФР Штрих онлайн. */

#include "ShtrihFROnlineConstants.h"

//--------------------------------------------------------------------------------
namespace CShtrihOnlineFR {
const char DateFormat[] = "yyyyMMdd";
const char FiscalTaxData[] = "\xFF\xFF\xFF\xFF\xFF";

//--------------------------------------------------------------------------------
namespace Commands {
namespace FS {
const char GetStatus[] = "\xFF\x01";             /// Получить статус ФН.
const char GetNumber[] = "\xFF\x02";             /// Получить номер ФН.
const char GetValidity[] = "\xFF\x03";           /// Получить срок действия ФН.
const char GetVersion[] = "\xFF\x04";            /// Получить версию ФН.
const char GetFiscalizationTotal[] = "\xFF\x09"; /// Получить итог фискализации.
const char GetFDbyNumber[] = "\xFF\x0A";         /// Получить фискальный документ по его номеру.
const char SetOFDParameter[] = "\xFF\x0C";       /// Передать произвольную TLV структуру (реквизит для ОФД).
const char GetOFDInterchangeStatus[] = "\xFF\x39"; /// Получить статус информационного обмена c ОФД.
const char GetFiscalTLVData[] = "\xFF\x3B"; /// Получить данные фискального документа в TLV-формате.
const char GetSessionParameters[] = "\xFF\x40"; /// Получить параметры текущей смены.
const char CloseDocument[] = "\xFF\x45";        /// Закрыть фискальный чек.
const char Sale[] = "\xFF\x46";                 /// Продажа.
const char StartFiscalTLVData[] = "\xFF\x3A";     /// Начать получение данных фискального документа в TLV-формате.
const char SetOFDParameterLinked[] = "\xFF\x4D";  /// Передать произвольную TLV структуру (реквизит для ОФД), привязанную к операции.
} // namespace FS
} // namespace Commands
} // namespace CShtrihOnlineFR
