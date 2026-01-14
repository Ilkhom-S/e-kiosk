# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include SDK headers
file(GLOB PPSDK_PP_HEADERS
    ${TC_INCLUDES_DIR}/SDK/PaymentProcessor/*.h
    ${TC_INCLUDES_DIR}/SDK/PaymentProcessor/*/*.h
)

file(GLOB PPSDK_GUI_HEADERS
    ${TC_INCLUDES_DIR}/SDK/GUI/*.h
)

ek_add_library(PPSDK
    SOURCES ${PPSDK_SOURCES} ${PPSDK_PP_HEADERS} ${PPSDK_GUI_HEADERS}
    QT_MODULES Core Widgets
    INCLUDE_DIRS
    ${TC_INCLUDES_DIR}/SDK/PaymentProcessor
    ${TC_INCLUDES_DIR}/SDK/GUI
    DEPENDS boost
)
