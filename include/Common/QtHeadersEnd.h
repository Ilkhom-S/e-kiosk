/* @file Конец блока заголовков Qt с подавлением предупреждений. */

#pragma once

//--------------------------------------------------------------------------------
/* Close the Qt headers warning suppression started by QtHeadersBegin.h

   Example:
         #include <Common/QtHeadersBegin.h>
         #include <QtCore/QList>
         #include <Common/QtHeadersEnd.h>
*/

/*
      Note for Linux/macOS/Clang/GCC users:
      ------------------------------------------------------------
      You may see a warning like:
            'pragma diagnostic pop could not pop, no matching push [-Wunknown-pragmas]'
      This is harmless and occurs if QtHeadersBegin.h did not push diagnostics, or if the compiler does not support the pragma.
      It does not affect compilation or runtime. Safe to ignore.
      On Windows/MSVC, warning suppression works as intended.
*/

#if defined(_MSC_VER)
#pragma warning(pop)
#undef HUMO_SUPPRESS_QT_WARNINGS
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#pragma once

// Restore warning state after including Qt headers
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
