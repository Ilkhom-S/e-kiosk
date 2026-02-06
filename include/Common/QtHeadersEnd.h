/* @file Конец блока заголовков Qt с подавлением предупреждений. */

#pragma once

//--------------------------------------------------------------------------------
/* Close the Qt headers warning suppression started by QtHeadersBegin.h

   Example:
         #include <QtCore/QList>
*/

/*
      Note for Linux/macOS/Clang/GCC users:
      ------------------------------------------------------------
      The pragma diagnostic pop warnings are now properly handled by only popping
      when diagnostics were actually pushed in QtHeadersBegin.h.
      On Windows/MSVC, warning suppression works as intended.
*/

// Only pop diagnostics if they were actually pushed in QtHeadersBegin.h
#if defined(_MSC_VER)
#pragma warning(pop)
#undef HUMO_SUPPRESS_QT_WARNINGS
#endif

#if defined(QT_HEADERS_DIAGNOSTICS_PUSHED_CLANG)
#pragma clang diagnostic pop
#undef QT_HEADERS_DIAGNOSTICS_PUSHED_CLANG
#endif

#if defined(QT_HEADERS_DIAGNOSTICS_PUSHED_GCC)
#pragma GCC diagnostic pop
#undef QT_HEADERS_DIAGNOSTICS_PUSHED_GCC
#endif

// Clean up tracking macros
#undef QT_HEADERS_DIAGNOSTICS_PUSHED_MSVC
#undef QT_HEADERS_DIAGNOSTICS_PUSHED
