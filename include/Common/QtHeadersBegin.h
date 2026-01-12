#pragma once

/*
 Suppress warnings coming from Qt headers.

 Usage:
	 #include <Common/QtHeadersBegin.h>
	 #include <QtCore/QList>
	 #include <Common/QtHeadersEnd.h>

 The MSVC variant enforces that every begin include is matched by an end include
 by defining `HUMO_SUPPRESS_QT_WARNINGS` while the Qt headers are included.
*/

#if defined(_MSC_VER)

#ifdef HUMO_SUPPRESS_QT_WARNINGS
#error QtHeadersBegin.h included without QtHeadersEnd.h!!!
#endif

#define HUMO_SUPPRESS_QT_WARNINGS

#pragma warning(push)

// Common MSVC warnings suppressed when including Qt headers or generated UI files
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4231) // nonstandard extension used : 'extern' before template explicit instantiation
#pragma warning(disable : 4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4251) // needs to have dll-interface
#pragma warning(disable : 4275) // non dll-interface class used as base for dll-interface class
#pragma warning(disable : 4290) // exception specification ignored
#pragma warning(disable : 4481) // override specifier 'override'
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4718) // recursive call has no side effects
#pragma warning(disable : 4800) // forcing value to bool (performance warning)
#pragma warning(disable : 4005) // macro redefinition

// Occurs in files auto-generated from .ui
#pragma warning(disable : 4125) // decimal digit terminates octal escape sequence

#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
