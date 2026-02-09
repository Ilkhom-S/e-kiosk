/* @file Константы купюроприемников Creator на протоколе CCNet. */

#pragma once

#include <QtCore/QString>

//--------------------------------------------------------------------------------
namespace CCCNetCreator {
/// Формат представления даты в ответе на запросы идентификации.
const QString DateFormat = "yyyyMM";

/// Формат представления даты для формирования данных устройства.
const QString DateLogFormat = "MM.yyyy";

/// Выход из initialize-а.
const int ExitInitializeTimeout = 5 * 1000;

/// Команды
namespace Commands {
/// Обновление прошивки. A1..A3 - это вроде как команды купюроприёмника
namespace UpdatingFirmware {
extern const char SetBaudRate[];
extern const char WriteHead[];
extern const char WriteBlock[];
extern const char Exit[];
} // namespace UpdatingFirmware

extern const char GetInternalVersion[];
extern const char GetSerial[];
} // namespace Commands

/// Обновление прошивки.
namespace UpdatingFirmware {
/// Размер заголовка прошивки.
const int HeadSize = 39;

/// Размер блока прошивки.
const int BlockSize = 128;

/// Ответы на команды обновления прошивки.
namespace Answers {
const char WritingBlockOK = '\xA4';
const char WritingBlockError = '\xA5';
} // namespace Answers
} // namespace UpdatingFirmware
} // namespace CCCNetCreator

//--------------------------------------------------------------------------------
