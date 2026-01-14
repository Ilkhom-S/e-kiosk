# SettingsManager module tests

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Core REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestSettingsManager
    SOURCES TestSettingsManager.cpp
    QT_MODULES Test Core
    DEPENDS SettingsManager ek_common
)
