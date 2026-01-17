# QMLBackend unit tests

include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

include_directories(${CMAKE_SOURCE_DIR}/src/plugins/QMLBackend/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

ek_add_test(qml_backend_test
    FOLDER "tests/plugins"
    SOURCES qml_backend_test.cpp
    DEPENDS qml_backend
    QT_MODULES Test Core Quick
)
