# DebugUtils module

# Cross-platform source files using Boost.Stacktrace
set(DEBUGUTILS_SOURCES
    src/DebugUtils.cpp
)

# Platform-specific headers (kept for compatibility)
if(WIN32)
    list(APPEND DEBUGUTILS_SOURCES
        src/StackWalker.h # Keep for compatibility but not used
        src/QStackWalker.h # Keep for compatibility but not used
    )
endif()

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# Find Boost stacktrace using the standard approach
find_package(Boost REQUIRED)

ek_add_library(DebugUtils
    FOLDER "modules"
    SOURCES ${DEBUGUTILS_SOURCES}
    QT_MODULES Core
    LIBRARIES Boost::boost
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

# Set minimum Windows version for XP compatibility and Unicode support
if(WIN32 AND MSVC)
    target_compile_definitions(DebugUtils PRIVATE _WIN32_WINNT=0x0501 UNICODE _UNICODE)
endif()
