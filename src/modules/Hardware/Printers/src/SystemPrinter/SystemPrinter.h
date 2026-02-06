/* @file Системный принтер. */

#pragma once

#include <QtGui/QBitmap>
#include <QtGui/QTextDocument>
#include <QtPrintSupport/QPrinter>

#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Printers/PrinterBase.h"
#include "SysUtils/ISysUtils.h"

//--------------------------------------------------------------------------------
/// Константы системного принтера.
namespace CSystemPrinter {
/// Тег конца строки.
const char BRtag[] = "<br>";

/// Отступ по умолчанию.
const qreal DefaultMargin = 1.0;

/// Коэффициент масштабирования для изображений.
const qreal ImageScalingFactor =
    0.5; // 0.475 соответствует попиксельной печати 1:1 для Custom VKP-80

//----------------------------------------------------------------------------
/// Теги.
class TagEngine : public Tags::Engine {
public:
    TagEngine() {
        appendSingle(Tags::Type::Italic, "", "<i>", "</i>");
        appendSingle(Tags::Type::Bold, "", "<b>", "</b>");
        appendSingle(Tags::Type::UnderLine, "", "<u>", "</u>");
        appendSingle(Tags::Type::Image,
                     "",
                     "<div align='center'><img src='data:image/png;base64,",
                     "'/></div>");
    }
};
} // namespace CSystemPrinter

//--------------------------------------------------------------------------------
class SystemPrinter : public PrinterBase<PollingDeviceBase<ProtoPrinter>> {
public:
    SystemPrinter();

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Напечатать чек.
    virtual bool printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt);

    /// Qt-принтер.
    QPrinter mPrinter;

    /// Боковой отступ.
    qreal mSideMargin;

    /// Боковой отступ.
    TStatusGroupNames mLastStatusesNames;
};

//--------------------------------------------------------------------------------
