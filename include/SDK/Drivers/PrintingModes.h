/* @file Режимы печати. */

#pragma once

namespace SDK {
namespace Driver {

/// Режимы печати.
namespace EPrintingModes {
enum Enum { None = 0, Continuous, Glue };
}; // namespace EPrintingModes

} // namespace Driver
} // namespace SDK

namespace DSDK = SDK::Driver;

//---------------------------------------------------------------------------
