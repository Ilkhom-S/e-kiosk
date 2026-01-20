
# Unit Tests for EKiosk


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)


# Helper binary used by tests to check BasicApplication behavior in a separate process
add_executable(CheckTestMode CheckTestMode.cpp)
set_target_properties(CheckTestMode PROPERTIES AUTOMOC ON AUTOUIC ON)
target_link_libraries(CheckTestMode PRIVATE BasicApplication ek_common Qt${QT_VERSION_MAJOR}::Core)

# Integration helper for instance tests
add_executable(InstanceHelper InstanceHelper.cpp)
set_target_properties(InstanceHelper PROPERTIES AUTOMOC ON AUTOUIC ON)
target_link_libraries(InstanceHelper PRIVATE BasicApplication ek_common Qt${QT_VERSION_MAJOR}::Core)

# QFile Logger tests
ek_add_test(TestQFileLogger
    FOLDER "tests/unit"
    SOURCES TestQFileLogger.cpp
    QT_MODULES Test Core
    DEPENDS ek_common Log
)

# SettingsManager tests moved to tests/modules/SettingsManager

