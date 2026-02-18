# Log module tests

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Core REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestSimpleLog
    FOLDER "tests/modules/Common/log"
    SOURCES TestSimpleLog.cpp
    QT_MODULES Test Core
    DEPENDS Log ek_common SysUtils
)
