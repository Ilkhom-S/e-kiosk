#pragma once

/* Close the Qt headers warning suppression started by QtHeadersBegin.h

   Example:
	 #include <Common/QtHeadersBegin.h>
	 #include <QtCore/QList>
	 #include <Common/QtHeadersEnd.h>
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
