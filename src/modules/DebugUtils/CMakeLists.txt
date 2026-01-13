# DebugUtils module

# Platform-specific source files
if(WIN32)
    file(GLOB DEBUGUTILS_SOURCES
        src/*.cpp
        src/*.h
    )
endif()

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

ek_add_library(DebugUtils
    SOURCES ${DEBUGUTILS_SOURCES}
    QT_MODULES Core
    LIBRARIES Advapi32 User32
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

# Set minimum Windows version for XP compatibility and Unicode support
if(WIN32 AND MSVC)
    target_compile_definitions(DebugUtils PRIVATE _WIN32_WINNT=0x0501 UNICODE _UNICODE)
endif()
