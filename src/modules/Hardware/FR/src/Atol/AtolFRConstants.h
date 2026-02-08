/* @file Константы, коды команд и ответов протокола ФР АТОЛ. */

#pragma once

#include <Hardware/Printers/Tags.h>

#include "AtolDataTypes.h"
#include "Hardware/Common/ASCII.h"
#include "Hardware/FR/FRErrorDescription.h"

//--------------------------------------------------------------------------------
namespace CAtolFR {
/// Количество товара.
const int GoodsCountByte = 1;

/// Минимальный код ошибки.
const uchar MinErrorCode = 0x02;

/// Маска для парсинга режима и подрежима.
const uchar ModeMask = 0x0F;

/// Формат печати X- и Z-отчетов - печатать необнуляемую сумму с момента последней перерегистрации
/// на расширенном отчете.
const char ReportMode = '\x03';

/// Маска для обнуления суммы в кассе при закрытии смены.
const char NullingSum_InCashMask = '\x04';

/// Маска для длинных отчетов.
const char LongReportMask = '\xE4';

/// Флаги выполнения фискальных операций.
namespace FiscalFlags {
const char ExecutionMode = '\x00';  /// Режим выполнения операций.
const char CashChecking = '\x00';   /// Проверять денежную наличность.
const char TaxForPosition = '\x00'; /// Налог на позицию.
} // namespace FiscalFlags

/// Флаги выполнения продажи - Выполнить операцию + проверить денежную наличность.
const char SaleFlags = FiscalFlags::ExecutionMode | FiscalFlags::CashChecking;

/// Номер ряда в таблице 2 - режим работы ККМ.
const char FRModeTableSeries = 1;

/// Тип налога - налог на каждую покупку.
const char Custom_SaleTax = 2;

/// Область действия налога - на регистрацию.
const char TaxControlArea = 2;

/// Максимальное количество заполненных по дефолту строк.
const char MaxDocumentCapStrings = 20;

/// Размер ИТОГ на фискальном чеке - двойная ширина (еще можно и/или двойную высоту).
const char ResumeSize = 4;

/// Флаг полной отрезки чека.
const char FullCutting = 4;

/// Сколько строк надо промотать после печати фискального чека.
const char DefaultFiscalFeedCTC2000 = 4;

/// Формат представления даты и времени в ответе на длинный запрос статуса.
extern const char DateTimeFormat[];

/// Формат представления даты и времени в ответе на запрос даты-времени сессии.
extern const char SessionDTFormat[];

/// Подсистемы ФР, имеющие свой софт - константы для запроса версий софта.
namespace FRSubSystems {
const char FR = 0x01; /// Фискальная плата.
const char FM = 0x02; /// Фискальная память.
const char BL = 0x03; /// Загрузчик.
} // namespace FRSubSystems

/// Типы отчетов без гашения.
namespace Balances {
/// Тип отчета без гашения - X-отчет.
const char XReport = '\x01';
} // namespace Balances

/// Таймауты чтения, [мс].
namespace Timeouts {
/// После окончания печати нефискального чека, чтобы принтер не захлебнулся командами, не начал
/// отрезать по напечатанному и пр.
const int EndNotFiscalPrint = 500;

/// Максимальный таймаут для получения ответа после паузы при выполнении Z-отчета.
const int ZReportNoAnswer = 120000;

/// Для получения ответа после паузы при выполнении X-отчета.
const int XReportNoAnswer = 30000;

/// Пауза между посылками запроса статуса при выполнении Z-отчета, во время которой ждем смены
/// состояний ФР.
const int ZReportPoll = 500;

/// Пауза между посылками запроса статуса при выполнении Z-отчета, во время которой ждем смены
/// состояний ФР.
const int XReportPoll = 500;
} // namespace Timeouts

class CPayOffTypeData : public CSpecification<SDK::Driver::EPayOffTypes::Enum, char> {
public:
    CPayOffTypeData() {
        using namespace SDK::Driver;

        append(EPayOffTypes::Debit, 1);
        append(EPayOffTypes::DebitBack, 2);
        append(EPayOffTypes::Credit, 4);
        append(EPayOffTypes::CreditBack, 5);
    }
};

static CPayOffTypeData PayOffTypeData;

/// Тип оплаты.
namespace PaymentSource {
const char Cash = 1;  /// Наличные.
const char Type2 = 2; /// Типы 2..4 - пользовательские.
const char Type3 = 3;
const char Type4 = 4;
} // namespace PaymentSource

//------------------------------------------------------------------------------------------------
/// Теги.
class TagEngine : public Tags::Engine {
public:
    TagEngine() { appendSingle(Tags::Type::DoubleWidth, "", "\x09"); }
};

//------------------------------------------------------------------------------------------------
/// Коды команд.
namespace Commands {
/// Команды получения информации об устройстве.
const char GetModelInfo = '\xA5';    /// Получить инфо о модели ФР.
const char GetSoftInfo = '\x9D';     /// Получить инфо о ПО девайса (ФР, ФП, загрузчик).
const char GetLongStatus = '\x3F';   /// Длинный статус.
const char GetShortStatus = '\x45';  /// Короткий статус.
const char GetFRRegister = '\x91';   /// Прочитать регистр ФР.
const char GetFRParameter = '\x46';  /// Прочитать параметр ФР.
const char SetFRParameters = '\x50'; /// Установить параметр ФР.

/// Нефискальные служебные команды.
const char Cut = '\x75';           /// Отрезка.
const char PrintString = '\x4C';   /// Печать строки.
const char PrinterAccess = '\x8F'; /// Прямой доступ к принтеру.
const char EnterToMode = '\x56';   /// Вход в режим.
const char ExitFrom_Mode = '\x48';  /// Выход из режима.

/// Фискальные команды.
const char OpenDocument = '\x92';   /// Открыть документ.
const char CloseDocument = '\x4A';  /// Закрыть документ.
const char Sale = '\x52';           /// Продажа.
const char Encashment = '\x4F';     /// Выплата.
const char CancelDocument = '\x59'; /// Аннулировать чек.
const char OpenFRSession = '\x9A';  /// Открыть смену.

/// Фискальные отчеты.
const char XReport = '\x67'; /// Печать X-отчета.
const char ZReport = '\x5A'; /// Печать Z отчета.
} // namespace Commands

/// Коды состояний, возвращаемых в байте флагов на команду 3Fh.
namespace States {
const char Fiscalized = 0x01;    /// Признак фискализированности ККМ.
const char SessionOpened = 0x02; /// Признак открытой смены.
const char CoverIsOpened = 0x20; /// Признак открытия крышки ККМ.
} // namespace States

/// Ошибки.
namespace Errors {
const char EnterToModeIsLocked = '\x1E';   /// Вход в режим заблокирован.
const char BadModeForCommand = '\x66';     /// Команда не может быть выполнена в текущем режиме.
const char NoPaper = '\x67';               /// Нет бумаги. Может означать все, что угодно.
const char CannotExecCommand = '\x7A';     /// Команда не может быть выполнена данной моделью ФР.
const char NeedZReport = '\x88';           /// Необходимо выполнить Z-отчет.
const char NeedCloseSaleDocument = '\x89'; /// Необходимо закрыть чек продажи.
const char WrongSeriesNumber = '\x92';     /// Неверный номер ряда
const char WrongFieldNumber = '\x93';      /// Неверный номер поля
const char NoMoneyForPayout = '\x98';      /// В ККТ нет денег для выплаты.
const char NeedCloseDocument = '\x9B';     /// Необходимо закрыть фискальный документ.
const char NeedCloseSession = '\x9C';      /// Смена открыта, операция невозможна
const char PrinterHeadOverheat = '\xD1';   /// Перегрев головки принтера.
} // namespace Errors

class CShortFlags : public CSpecification<char, int> {
public:
    CShortFlags() {
        append('\x01', PrinterStatusCode::Error::PaperEnd);
        append('\x02', PrinterStatusCode::Error::PrinterFRNotAvailable);
        append('\x04', PrinterStatusCode::Error::PrintingHead);
        append('\x08', PrinterStatusCode::Error::Cutter);
        append('\x10', PrinterStatusCode::Error::Temperature);
        append('\x40', PrinterStatusCode::Error::PrinterFR);
        append('\x80', PrinterStatusCode::Warning::PaperNearEnd);
    }
};

static CShortFlags ShortFlags;

namespace FRParameters {
const SData TaxType = SData(2, 1, 11);
const SData PrintSectionName = SData(2, 1, 15);
const SData ReportMode = SData(2, 1, 18);
const SData EjectorParameters = SData(2, 1, 22);
const SData Cut = SData(2, 1, 24);
const SData ResumeSize = SData(2, 1, 25);
const SData PrintCashier = SData(2, 1, 26);
const SData ContinuousDocumentNumbering = SData(2, 1, 27);
const SData AutoNullingChequeCounter = SData(2, 1, 28);
const SData LineSpacing = SData(2, 1, 30);
const SData DocumentCapStringsAmount = SData(2, 1, 36);
const SData PrintSectionNumber = SData(2, 1, 42);
const SData OpeningSessionDocument = SData(2, 1, 43);
const SData PrintNotFiscalData = SData(2, 1, 51);
const SData PrintingSettings = SData(2, 1, 55);

inline SData DocumentCap(int aSeries) {
    return SData(6, ushort(aSeries), 1);
}
inline SData SectionName(int aSeries) {
    return SData(7, ushort(aSeries), 1);
}
inline SData Tax(int aSeries) {
    return SData(8, ushort(aSeries), 1);
}
inline SData TaxDescription(int aSeries) {
    return SData(13, ushort(aSeries), 1);
}
} // namespace FRParameters

/// Регистры
namespace Registers {
extern const char PaymentAmount[];
extern const char PaymentCount[];
extern const char MoneyInCash[];
extern const char CurrentDateTime[];
extern const char SessionInfo[];
extern const char SerialNumber[];
extern const char NonNullableAmount[];
extern const char PrintingSettings[];
} // namespace Registers

/// Режимы.
namespace InnerModes {
const char NoMode = '\xFF';
const char Choice = '\x00';
const char Register = '\x01';
const char NotCancel = '\x02';
const char Cancel = '\x03';
const char Programming = '\x04';
const char AccessToFP = '\x05';
const char AccessToEKLZ = '\x06';
const char ExtraCommand = '\x07';
} // namespace InnerModes

/// Подрежимы.
namespace InnerSubmodes {
const char NoSubmode = '\xFF';
const char EnterDate = '\x05';
const char EnterTime = '\x06';
const char FMDataTimeError = '\x0B';
const char FMDataTimeConfirm = '\x0C';
const char EKLZError = '\x0E';
} // namespace InnerSubmodes

/// Пользователи. По умолчанию пароль для входа в режим совпадает с номером пользователя.
namespace Users {
const uchar Admin = 0x29;
const uchar SysAdmin = 0x30;
} // namespace Users

/// Данные языковых таблиц.
typedef QMap<uchar, QString> TLanguages;

struct SLanguages {
    TLanguages m_Languages;

    SLanguages() {
        m_Languages.insert(0, "Russian");
        m_Languages.insert(1, "Armenian");
        m_Languages.insert(2, "Moldavian");
        m_Languages.insert(3, "Ukrainian");
        m_Languages.insert(4, "Lithuanian");
        m_Languages.insert(5, "Turkmen");
        m_Languages.insert(6, "Mongolian");
        m_Languages.insert(7, "Belarus");
        m_Languages.insert(8, "Latvian");
        m_Languages.insert(9, "Georgian");
        m_Languages.insert(10, "Kazakh");
        m_Languages.insert(11, "Estonian");
        m_Languages.insert(12, "Azerbaijan");
        m_Languages.insert(13, "Kirghiz");
        m_Languages.insert(14, "Tadjik");
        m_Languages.insert(15, "Uzbek");
        m_Languages.insert(16, "Polish");
        m_Languages.insert(17, "Romanian");
        m_Languages.insert(18, "Bulgarian");
        m_Languages.insert(19, "English");
        m_Languages.insert(20, "Finnish");
    }

    QString operator[](const uchar languageKey) const {
        return m_Languages.contains(languageKey) ? m_Languages[languageKey] : "Unknown";
    }
};

static SLanguages Languages;
} // namespace CAtolFR

//--------------------------------------------------------------------------------
