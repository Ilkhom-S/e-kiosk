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
namespace CSystem_Printer {
/// Тег конца строки.
extern const char BRtag[];

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
} // namespace CSystem_Printer

//--------------------------------------------------------------------------------
class System_Printer : public PrinterBase<PollingDeviceBase<ProtoPrinter>> {
public:
    System_Printer();

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
    QPrinter m_Printer;

    /// Боковой отступ.
    qreal m_SideMargin;

    /// Боковой отступ.
    TStatusGroupNames m_LastStatusesNames;
};

//--------------------------------------------------------------------------------
