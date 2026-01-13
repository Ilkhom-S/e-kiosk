# SysUtils module

# Platform-specific source files
if(WIN32)
    file(GLOB SYSUTILS_SOURCES
        src/windows/*.cpp
        src/windows/*.h
    )
endif()

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

ek_add_library(SysUtils
    SOURCES ${SYSUTILS_SOURCES}
    QT_MODULES Core
    LIBRARIES Advapi32 DelayImpHlp
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/DelayImpHlp
)

# Set minimum Windows version for XP compatibility
if(WIN32 AND MSVC)
    target_compile_definitions(SysUtils PRIVATE _WIN32_WINNT=0x0501 UNICODE _UNICODE)
endif()
