# GUISDK module

file(GLOB GUISDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK GUI headers
set(GUISDK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/GraphicsItemInfo.h
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IGraphicsBackend.h
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IGraphicsEngine.h
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IGraphicsHost.h
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IGraphicsItem.h
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/MessageBoxParams.h
)

ek_add_library(GUISDK
    SOURCES ${GUISDK_SOURCES} ${GUISDK_HEADERS}
    QT_MODULES Core Gui
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include/SDK/GUI ${CMAKE_CURRENT_SOURCE_DIR}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
