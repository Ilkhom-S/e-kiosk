# Connection module

file(GLOB CONNECTION_COMMON_SOURCES
    Common/*.cpp
    Common/*.h
)

set(CONNECTION_SOURCES ${CONNECTION_COMMON_SOURCES})

# Platform-specific sources
if(WIN32)
    file(GLOB CONNECTION_WIN32_SOURCES
        Win32/src/*.cpp
        Win32/src/*.h
    )
    list(APPEND CONNECTION_SOURCES ${CONNECTION_WIN32_SOURCES})
endif()

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

ek_add_library(Connection
    FOLDER "modules/Connection"
    SOURCES ${CONNECTION_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

if(WIN32)
    target_link_libraries(Connection PUBLIC Rasapi32)
endif()
