# GraphicsEngine module

file(GLOB GRAPHICSENGINE_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(GraphicsEngine
    SOURCES ${GRAPHICSENGINE_SOURCES}
    QT_MODULES Core Gui Widgets Concurrent Quick Qml QuickWidgets
)

# Qt5 migration: GraphicsEngine now uses QtQuick instead of QGraphicsView
# This replaces the old QInputContext-based virtual keyboard with Qt5 input methods
