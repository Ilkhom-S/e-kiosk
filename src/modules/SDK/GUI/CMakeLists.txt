# GUISDK module

file(GLOB GUISDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK GUI headers
set(GUISDK_HEADERS
    ${EK_INCLUDES_DIR}/SDK/GUI/GraphicsItemInfo.h
    ${EK_INCLUDES_DIR}/SDK/GUI/IGraphicsBackend.h
    ${EK_INCLUDES_DIR}/SDK/GUI/IGraphicsEngine.h
    ${EK_INCLUDES_DIR}/SDK/GUI/IGraphicsHost.h
    ${EK_INCLUDES_DIR}/SDK/GUI/IGraphicsItem.h
    ${EK_INCLUDES_DIR}/SDK/GUI/MessageBoxParams.h
)

ek_add_library(GUISDK
    FOLDER "modules/SDK"
    SOURCES ${GUISDK_SOURCES} ${GUISDK_HEADERS}
    QT_MODULES Core Gui Quick
    DEPENDS ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
