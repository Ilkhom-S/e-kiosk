/* @file Константы POS-принтеров. */

#pragma once

#include <QtCore/QByteArray>

#include "Hardware/Common/WaitingData.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний POS-принтеров.
namespace CPOSPrinter {
/// Команды.
namespace Command {
extern const char GetModelId[];    /// Получение идентификатора модели.
extern const char GetTypeId[];     /// Получение идентификатора типа модели.
extern const char GetROMVersion[]; /// Получение версии прошивки.
extern const char Initialize[];        /// Инициализация.
extern const char SetEnabled[];    /// Включение доступности принтера.
extern const char SetUSCharacterSet[]; /// Установка набора символов США (не кодовая страница).
extern const char SetStandardMode[]; /// Установка стандартного режима.
extern const char Present[];     /// Неизменяемая часть команды презентации чека.
extern const char Retract[];     /// Забирание чека в ретрактор.
extern const char Push[];        /// Выталкивание чека.
extern const char LoopEnable[];  /// Включение петли.
extern const char LoopDisable[]; /// Выключение петли.
extern const char GetPaperStatus[];  /// Запрос статуса бумаги.
extern const char Cut[];             /// Отрезка.
extern const char PrintImage[];  /// Печать картинки.
extern const char AlignLeft[];   /// Выравнивание по левому краю.

inline QByteArray GetStatus(char aStatusType) {
    return QByteArray("\x10\x04") + aStatusType;
} /// Запрос статуса.
inline QByteArray SetCodePage(char aCodePage) {
    return QByteArray("\x1B\x74") + aCodePage;
} /// Установка кодовой страницы.
inline QByteArray SetLineSpacing(int aSpacing) {
    return QByteArray("\x1B\x33") + char(aSpacing);
} /// Установка множителя высоты строки.

/// Штрих-коды.
namespace Barcode {
extern const char Height[];      /// Высота штрих-кода - 20.25 мм.
extern const char HRIPosition[]; /// Позиционирование символов штрих-кода - выше штрих-кода.
extern const char FontSize[];    /// Размер шрифта штрих-кода.
extern const char Width[];       /// Ширина линии - 0.25 мм.
extern const char Print[];       /// Печать.
} // namespace Barcode
} // namespace Command

const char RussianCodePage = '\x11';              /// Номер русской кодовой страницы.
extern const char DefaultName[]; /// Имя принтера по умолчанию.

/// Штрих-коды.
namespace Barcode {
const char Height = '\xA0';            /// Высота штрих-кода - 20.25 мм.
const char HRIPosition = '\x01';       /// Позиционирование символов штрих-кода - выше штрих-кода.
const char FontSize = '\x49';          /// Размер шрифта штрих-кода.
const char Width = '\x02';             /// Ширина линии - 0.25 мм.
const char CodeSystem128 = '\x49';     /// Система штрих-кода - CODE128.
extern const char Code128Spec[]; /// Спецификация (уточнение, подвид) системы Code128.
} // namespace Barcode

/// Константы для представления коэффициентов масштаба (1 или 2).
namespace ImageFactors {
const char DoubleWidth = '\x01';
const char DoubleHeight = '\x02';
} // namespace ImageFactors

/// Таймауты ожидания ответа на запрос, [мс].
namespace Timeouts {
const int Status = 200; /// Статус.
const int Info = 1000;  /// Информация о модели.
} // namespace Timeouts

/// Ожидание выхода из анабиоза, [мс].
const SWaitingData AvailableWaiting = SWaitingData(350, 800);

/// Пауза между запросами статусов.
const int StatusPause = 10;

/// Пауза после инициализации.
const int InitializationPause = 1000;
} // namespace CPOSPrinter

//--------------------------------------------------------------------------------
