/* @file Уровни тревожности статус-кодов. */

#pragma once

#include <QtCore/QMetaType>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Общие состояния устройств - уровень тревожности.
namespace EWarningLevel {
enum Enum {
    OK,      /// Нет ошибок.
    Warning, /// Предупреждение.
    Error    /// Ошибка.
};
} // namespace EWarningLevel

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
Q_DECLARE_METATYPE(SDK::Driver::EWarningLevel::Enum);

//--------------------------------------------------------------------------------
