# MessageQueue (TCP) module tests

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Core Network REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestMessageQueueTcp
    FOLDER "tests/modules/MessageQueue"
    SOURCES TestMessageQueueTcp.cpp
    QT_MODULES Test Core Network
    DEPENDS MessageQueue
)
