# GraphicsEngine module

file(GLOB GRAPHICSENGINE_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(GraphicsEngine
    FOLDER "modules"
    SOURCES ${GRAPHICSENGINE_SOURCES}
    QT_MODULES Core Gui Widgets Concurrent Quick Qml QuickWidgets
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
)


