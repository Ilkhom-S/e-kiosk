/* @file Константы, команды и коды состояний принтеров Epson.. */

#include "EpsonConstants.h"

//--------------------------------------------------------------------------------
namespace CEpsonEUT400 {
namespace Command {
const char GetVersion[] = "\x1D\x49\x41";    /// Получение версии прошивки.
const char GetFont[] = "\x1D\x49\x45";       /// Получение установленного шрифта.
const char GetMemorySize[] = "\x1D\x49\x72"; /// Получение размера установленной памяти.
const char GetOptions[] = "\x1D\x49\x73";    /// Получение списка дополнительных устройств.
const char Cut[] = "\x1D\x56\x31";           /// Отрезка без возможности обратной промотки.
const char CutBackFeed[] =
    "\x1D\x56\x42\x01"; /// Промотка и отрезка с возможностью обратной промотки.

/// Работа с мemory-switch (MSW)
namespace MemorySwitch {
/// Префикс
const char Prefix[] = "\x37\x21";

/// MSW8: маска правильных значений при использовании обратной промотки
const char ReceiptProcessing2Mask[] = "xx0xx1xx";

/// MSW8: маска проверки обратной промотки
const char BackFeedMask[] = "xxxxx1xx";

/// MSW8: маска отсутствия обратной промотки
const char NoBackFeedMask[] = "xxxxx0xx";
} // namespace MemorySwitch
} // namespace CEpsonEUT400
