/* @file Параметры LPT-порта. */

#pragma once

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Адреса регистров.
namespace CLPTPortAddress {
const int DataRegister = 0x378;
const int StatusRegister = 0x379;
const int ControlRegister = 0x37A;
} // namespace CLPTPortAddress

//--------------------------------------------------------------------------------
/// Параметры LPT-порта.
namespace ELPTPortParameters {
// Note: Port number is typically specified as a configuration parameter, not an enum value.
// If specific port numbers need to be defined, they should be added as constants here.
const int DefaultPortNumber = 0; /// Номер порта по умолчанию.
} // namespace ELPTPortParameters

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
