/* @file Приоритет, с которым производится авто-поиск устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

namespace SDK {
    namespace Driver {

        //---------------------------------------------------------------------------
        /// Приоритет авто-поиска устройств.
        namespace EDetectingPriority {
            enum Enum { Fallback, Low, Normal, High, VeryHigh };
        } // namespace EDetectingPriority

    } // namespace Driver
} // namespace SDK

Q_DECLARE_METATYPE(SDK::Driver::EDetectingPriority::Enum);

//------------------------------------------------------------------------------
