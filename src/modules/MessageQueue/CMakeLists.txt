# MessageQueue module (TCP-based)

file(GLOB MESSAGEQUEUE_SOURCES
    Tcp/src/*.cpp
    Tcp/src/*.h
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)

ek_add_library(MessageQueue
    FOLDER "modules"
    SOURCES ${MESSAGEQUEUE_SOURCES}
    QT_MODULES Core Network
    DEPENDS Log
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/Tcp/src
)




