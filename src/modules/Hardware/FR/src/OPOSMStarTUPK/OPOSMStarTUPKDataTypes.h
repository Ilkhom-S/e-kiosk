/* @file Типы данных MStar TUP-K на OPOS-драйвере. */

#pragma once

#include <QtCore/QString>

#include "WinDef.h"

//--------------------------------------------------------------------------------
namespace COPOSMStarTUPK {
/// Параметры.
namespace Parameters {
enum Enum { AutoCutter, TaxesPrint, ZBuffer };

struct Data {
    QString name;
    DWORD value;

    Data() : name(""), value(0) {}
    Data(const char *aName, DWORD aValue) : name(aName), value(aValue) {}
};
} // namespace Parameters

struct SErrorData {
    QString function;
    int error;

    SErrorData() : error(0) {}
    SErrorData(const QString &aFunction, int aError) : function(aFunction), error(aError) {}

    bool operator==(const SErrorData &aErrorData) const {
        return (aErrorData.function == function) && (aErrorData.error == error);
    }
};
} // namespace COPOSMStarTUPK

//--------------------------------------------------------------------------------
