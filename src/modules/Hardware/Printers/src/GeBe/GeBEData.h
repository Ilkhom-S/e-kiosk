/* @file Константы принтеров GeBE. */

#pragma once

#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров GeBE.
namespace CGeBE {
/// Max число символов, передаваемых на печать за раз.
const int LineSize = 28;

/// Статусы.
class CStatuses : public CSpecification<char, int> {
public:
    CStatuses() {
        append('H', PrinterStatusCode::Error::PrintingHead);
        append('P', PrinterStatusCode::Error::PaperEnd);
        append('Z', PrinterStatusCode::Warning::PaperNearEnd);
        append('G', PrinterStatusCode::Warning::PaperNearEnd);
        append('C', PrinterStatusCode::Error::Cutter);
        append('K', PrinterStatusCode::Error::Temperature);
        append('I', PrinterStatusCode::Error::Temperature);
        append('U', DeviceStatusCode::Error::PowerSupply);
        append('M', DeviceStatusCode::Error::PowerSupply);
        append('?', PrinterStatusCode::Error::Port);
        append('E', DeviceStatusCode::Error::MemoryStorage);

        setDefault(DeviceStatusCode::OK::OK);
    }
};

static CStatuses Statuses;

/// Команды.
namespace Commands {
extern const char Initialize[];           /// Инициализация.
extern const char SetFont[];          /// Установить шрифт.
extern const char SetLeftAlign[]; /// Установить выравнивание слева.
extern const char GetStatus[];        /// Cтатус.
} // namespace Commands

/// Теги.
class TagEngine : public Tags::Engine {
public:
    TagEngine() {
        appendSingle(Tags::Type::UnderLine, "\x1B\x4C", "\x31", "\x30");
        appendSingle(Tags::Type::DoubleWidth, "\x1B\x57", "\x31", "\x30");
        appendSingle(Tags::Type::DoubleHeight, "\x1B\x48", "\x31", "\x30");
    }
};
} // namespace CGeBE

//--------------------------------------------------------------------------------
