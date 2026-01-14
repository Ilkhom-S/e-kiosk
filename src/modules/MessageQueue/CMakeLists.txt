# MessageQueue module (TCP-based)
# Note: The original QBS file is in Tcp subfolder

file(GLOB MESSAGEQUEUE_SOURCES
    Tcp/src/*.cpp
    Tcp/src/*.h
)

ek_add_library(MessageQueue
    SOURCES ${MESSAGEQUEUE_SOURCES}
    QT_MODULES Core Network
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Tcp/src
)
