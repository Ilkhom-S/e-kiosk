/* @file Данные принтера PrimexNP2511. */

#pragma once

// Models
#include "Hardware/Common/DeviceDataConstants.h"
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
namespace CPrimexNP2511 {
/// Команды.
namespace Commands {
extern const char PrinterInfo[];
extern const char GetStatus[];
extern const char BackFeed[];
extern const char BackFeed2[];
extern const char ClearDispenser[];
extern const char Initialize[];
extern const char SetCyrillicPage[];
extern const char AutoRetract[]; // Через 1 минуту засасываем в ретрактор

/// Команды для печати штрих-кода.
namespace BarCode {
extern const char SetFont[];
extern const char SetHeight[];
extern const char SetHRIPosition[];
extern const char SetWidth[];
extern const char Print[];
} // namespace BarCode
} // namespace Commands

/// Статусы.
class CStatuses : public CSpecification<int, int> {
public:
    CStatuses() {
        append(0, PrinterStatusCode::Warning::PaperNearEnd);
        append(1, PrinterStatusCode::Error::PrintingHead);
        append(2, PrinterStatusCode::Error::PaperEnd);
        append(3, PrinterStatusCode::Error::PaperJam);
        append(4, PrinterStatusCode::Error::Cutter);
        append(5, DeviceStatusCode::Error::Unknown);
        append(6, PrinterStatusCode::Error::PaperJam);
    }
};

static CStatuses Statuses;

/// Размер шрифта штрих-кода.
const char PrinterBarCodeFontSize = '\x00';

/// Количество строк обратной промотки.
const int BackFeedCount = 11;

//----------------------------------------------------------------------------
/// Теги.
class TagEngine : public Tags::Engine {
public:
    TagEngine() {
        appendSingle(Tags::Type::Bold, "\x1B\x45", "\x01");
        appendSingle(Tags::Type::UnderLine, "\x1B\x2D", "\x01");
        appendCommon(Tags::Type::DoubleWidth, "\x1C\x21", "\x40");
        appendCommon(Tags::Type::DoubleHeight, "\x1C\x21", "\x80");
    }
};

//----------------------------------------------------------------------------
/// Данные устройства.
struct SDeviceParameters {
    int size;
    QString description;

    SDeviceParameters() : size(0) {}
    SDeviceParameters(int aSize, const QString &aDescription)
        : size(aSize), description(aDescription) {}
};

typedef QMap<char, SDeviceParameters> TDeviceParameters;
typedef QMap<char, SDeviceParameters>::iterator TDeviceParametersIt;

class CDeviceParameters : public CSpecification<char, SDeviceParameters> {
public:
    CDeviceParameters() {
        append(ASCII::STX, SDeviceParameters(7, CDeviceData::ModelName));
        append(ASCII::ETX, SDeviceParameters(8, CDeviceData::Firmware));
        append(ASCII::EOT, SDeviceParameters(8, CDeviceData::BootFirmware));
        append(ASCII::ENQ, SDeviceParameters(4, CDeviceData::Switches));
    }
};
}; // namespace CPrimexNP2511

//--------------------------------------------------------------------------------
