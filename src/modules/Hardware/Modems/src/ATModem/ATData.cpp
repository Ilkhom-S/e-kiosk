/* @file Данные протокола AT. */

#include "ATData.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе AT.
namespace AT {
namespace Commands {
const char AT[] = "";   /// Префикс AT добавляется автоматически.
const char ATZ[] = "Z"; /// Сброс конфигурации.
const char ATE[] = "E";
const char ATI[] = "I";
const char ATandF0[] = "&F0";
const char ATandV[] = "&V";
const char Revision[] = "+GMR";       /// Получение ревизии модема
const char IMEI[] = "+GSN";           /// Запрос получения IMEI модема
const char IMSI[] = "+CIMI";          /// Запрос получения IMSI
const char CPIN[] = "+CPIN?";         /// Запрос состояния SIM карты
const char CNUM[] = "+CNUM";          /// Запрос номера телефона симки
const char CopsMode[] = "+COPS=3,0";  /// Переключение режима вывода имени оператора
const char COPS[] = "+COPS?";         /// Запрос имени оператора.
const char CSQ[] = "+CSQ";            /// Запрос качества сигнала.
const char CUSD[] = "+CUSD=1";        /// Отправка USSD запроса.
const char CREG[] = "+CREG?";         /// Проверка доступности провайдера.
const char SetTextMode[] = "+CMGF=1"; /// Текстовый режим при отправке SMS.
const char SetPDUMode[] = "+CMGF=0";  /// Бинарный режим работы с SMS.
const char SendSMS[] = "+CMGS=";      /// Отправка SMS.
const char ListSMS[] = "+CMGL=4";     /// Получение списка всех SMS.
const char DeleteSMS[] = "+CMGD=";    /// Удаление SMS.

namespace Huawei {
const char SIMID[] = "^ICCID?"; /// SIM card identification number
} // namespace Huawei

namespace Siemens {
const char Restart[] = "+CFUN=1,1";    /// Перегрузить модем и заставить его перерегистрироваться в
                                       /// сети. Требует 5 секундного ожидания.
const char SIMID[] = "^SCID";          /// SIM card identification number
const char GetCellList[] = "^SMONC";   /// получение списка базовых станций
const char ActiveCellInfo[] = "^MONI"; /// получение уровня сигнала активной базовой станции
const char InactiveCellInfo[] = "^MONP"; /// получение уровня сигнала не активных базовых станции
} // namespace Siemens

namespace ZTE {
const char SIMID[] = "+CRSM=176,12258,0,0,10"; /// SIM card identification number
const char PowerOn[] = "+CFUN=1";              /// Подать питание на GSM модуль
} // namespace ZTE

namespace Sim_Com {
const char Restart[] = "+CFUN=1,1";  /// Перегрузить модем и заставить его перерегистрироваться в
                                     /// сети. Требует 5 секундного ожидания.
const char CSCS[] = "+CSCS=\"GSM\""; /// Переключение кодировки USSD запросов.

const char CGREG[] = "+CGREG";        /// Network Registration Status
const char GetCellList[] = "+CCINFO"; /// получение списка базовых станций
} // namespace Sim_Com
} // namespace Commands
} // namespace AT
