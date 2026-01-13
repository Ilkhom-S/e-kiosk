# SysUtils module

# Platform-specific source files
if(WIN32)
    file(GLOB SYSUTILS_SOURCES
        src/windows/*.cpp
        src/windows/*.h
    )
endif()

ek_add_library(SysUtils
    SOURCES ${SYSUTILS_SOURCES}
    QT_MODULES Core
    LIBRARIES Advapi32 DelayImpHlp
)

# Set minimum Windows version for XP compatibility
if(WIN32 AND MSVC)
    target_compile_definitions(SysUtils PRIVATE _WIN32_WINNT=0x0501)
endif()
