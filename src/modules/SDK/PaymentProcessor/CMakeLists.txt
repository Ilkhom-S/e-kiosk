# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK headers
file(GLOB PPSDK_PP_HEADERS
    ${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/*.h
    ${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/*/*.h
)

file(GLOB PPSDK_GUI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI/*.h
)

ek_add_library(PPSDK
    SOURCES ${PPSDK_SOURCES} ${PPSDK_PP_HEADERS} ${PPSDK_GUI_HEADERS}
    QT_MODULES Core Widgets
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS boost
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
