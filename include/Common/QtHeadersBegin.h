#pragma once

// Suppress warnings from Qt headers for various compilers
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4251) // dll-interface
#  pragma warning(disable: 4996) // deprecated
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
