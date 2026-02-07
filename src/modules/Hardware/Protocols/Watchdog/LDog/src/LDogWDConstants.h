/* @file Константы протокола сторожевого таймера LDog. */

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QPair>

//--------------------------------------------------------------------------------
namespace CLDogWD {
/// Адрес платы.
const char Address = '\x10';

/// Конечный байт пакета.
const char Postfix = '\x0D';

/// Маскированный конечный байт пакета.
extern const char MaskedPostfix[];

/// Минимальный размер ответного пакета.
const int MinAnswerSize = 4;

/// Маска корректного ответа для адреса и команды.
const char AnswerMask = '\x80';

typedef QPair<char, QByteArray> TReplaceData;
typedef QList<TReplaceData> TReplaceDataList;

/// Список данных для маскирования.
const TReplaceDataList ReplaceDataList =
    TReplaceDataList() << TReplaceData('\x40', QByteArray::fromRawData("\x40\x00", 2))
                       << TReplaceData('\x0D', MaskedPostfix);

/// Таймауты, [мс].
namespace Timeouts {
/// Чтение ответа.
const ushort Answer = 2000;
} // namespace Timeouts
} // namespace CLDogWD

//--------------------------------------------------------------------------------
