# GUISDK module

file(GLOB GUISDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK GUI headers
set(GUISDK_HEADERS
    ${TC_INCLUDES_DIR}/SDK/GUI/GraphicsItemInfo.h
    ${TC_INCLUDES_DIR}/SDK/GUI/IGraphicsBackend.h
    ${TC_INCLUDES_DIR}/SDK/GUI/IGraphicsEngine.h
    ${TC_INCLUDES_DIR}/SDK/GUI/IGraphicsHost.h
    ${TC_INCLUDES_DIR}/SDK/GUI/IGraphicsItem.h
    ${TC_INCLUDES_DIR}/SDK/GUI/MessageBoxParams.h
)

ek_add_library(GUISDK
    SOURCES ${GUISDK_SOURCES} ${GUISDK_HEADERS}
    QT_MODULES Core Gui
    INCLUDE_DIRS ${TC_INCLUDES_DIR}/SDK/GUI
)
