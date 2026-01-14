# SettingsManager module

file(GLOB SETTINGSMANAGER_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(SettingsManager
    SOURCES ${SETTINGSMANAGER_SOURCES}
    QT_MODULES Core
    DEPENDS boost
)
