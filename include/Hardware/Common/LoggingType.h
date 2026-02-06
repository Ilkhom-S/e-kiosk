/* @file Тип логгирования сообщений устройств. */

#pragma once

#include <QtCore/QMetaType>

//--------------------------------------------------------------------------------
namespace ELoggingType {
enum Enum { None = 0, Write, ReadWrite };
} // namespace ELoggingType

Q_DECLARE_METATYPE(ELoggingType::Enum);

//--------------------------------------------------------------------------------
