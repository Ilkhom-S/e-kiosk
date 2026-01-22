# SysUtils module

# Platform-specific source files

if(WIN32)
    file(GLOB SYSUTILS_SOURCES
        src/windows/*.cpp
        src/windows/*.h
    )
else()
    file(GLOB SYSUTILS_SOURCES
        src/unix/*.cpp
        src/unix/*.h
    )
endif()

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)


ek_add_library(SysUtils
    FOLDER "modules"
    SOURCES ${SYSUTILS_SOURCES}
    QT_MODULES Core
    LIBRARIES DelayImpHlp
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/DelayImpHlp
)

# Link Advapi32 only on Windows
if(WIN32)
    target_link_libraries(SysUtils PRIVATE Advapi32)
endif()

# Set minimum Windows version for XP compatibility
if(WIN32 AND MSVC)
    target_compile_definitions(SysUtils PRIVATE _WIN32_WINNT=0x0501 UNICODE _UNICODE)
endif()
