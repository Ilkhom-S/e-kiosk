# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(PPSDK
    SOURCES ${PPSDK_SOURCES}
    QT_MODULES Core Widgets
    INCLUDE_DIRS
    DEPENDS ek_boost ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
