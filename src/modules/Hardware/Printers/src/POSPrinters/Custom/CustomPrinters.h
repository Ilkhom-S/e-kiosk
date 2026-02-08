/* @file Принтеры Custom. */

#pragma once

#include <QtCore/qmath.h>

#include "Hardware/Printers/PortPOSPrinters.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров Custom.
namespace CCustom_Printer {
namespace Models {
extern const char TG2480[];
extern const char TG2480H[];
extern const char TG2460H[];
extern const char TL80[];
extern const char TL60[];
} // namespace Models

/// Высота линии изображения в пикселях (1 пиксель == 1 бит).
const int LineHeight = 24;

/// Режим загрузки изображения в принтер - 24-х битовыми линиями.
const char Image24BitMode = '\x21';

/// Максимальная ширина печатаемой части изображения, [пикс] (1 пикс == 1 бит).
const int MaxImageWidth = 319;

//----------------------------------------------------------------------------
/// Команды.
namespace Commands {
extern const char PrintImage[]; /// Печать изображения.
} // namespace Commands

//----------------------------------------------------------------------------
/// GAM (Graphics advanced mode).
namespace GAM {
/// Максимальная ширина печатаемой части изображения, [пикс] (1 пикс == 1 бит).
const int MaxImageWidth = 638;

/// Получить время печати картинки, [мс].
inline qint64 getImagePause(const QImage &aImage,
                            const SDK::Driver::TPortParameters &aPortParameters) {
    int baudrate = aPortParameters[SDK::Driver::IOPort::COM::EParameters::BaudRate];
    int size = 21 + aImage.height() *
                        qCeil(6.0 + qCeil(aImage.width() /
                                          8.0)); // расчетный размер монохромной картинки в байтах

    return qCeil(1000.0 * size * getFrameSize(aPortParameters) / baudrate);
}

/// Команды.
namespace Commands {
extern const char SetPageLength[]; /// Установить длину страницы = 0. По размеру?
extern const char SetResolution204[]; /// Установить разрешение 240 dpi.
extern const char SetNoCompression[];         /// Отменить сжатие изображения.
extern const char SetLeftMargin[];                /// Установить сдвиг слева.
extern const char SendData[];                     /// Послать картинку в принтер.
extern const char PrintImage[];                   /// Печать изображения.
} // namespace Commands
} // namespace GAM
} // namespace CCustom_Printer

//--------------------------------------------------------------------------------
template <class T> class Custom_Printer : public POSPrinter<T> {
    SET_SUBSERIES("Custom")

public:
    Custom_Printer();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

protected:
    /// Напечатать картинку протокольным методом по умолчанию.
    bool printImageDefault(const QImage &aImage, const Tags::TTypes &aTags);

    /// Напечатать картинку методом GAM.
    virtual bool printImage(const QImage &aImage, const Tags::TTypes &aTags);
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<Custom_Printer<TSerialPrinterBase>> SerialCustom_Printer;
typedef Custom_Printer<TLibUSBPrinterBase> LibUSBCustom_Printer;

//--------------------------------------------------------------------------------
