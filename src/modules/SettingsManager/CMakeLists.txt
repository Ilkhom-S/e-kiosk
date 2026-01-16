# SettingsManager module

file(GLOB SETTINGSMANAGER_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

ek_add_library(SettingsManager
    FOLDER "modules/SettingsManager"
    SOURCES ${SETTINGSMANAGER_SOURCES}
    QT_MODULES Core
    DEPENDS ek_boost
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
