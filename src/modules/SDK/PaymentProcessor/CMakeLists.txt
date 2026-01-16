# PPSDK (PaymentProcessor SDK) module

file(GLOB_RECURSE PPSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Ensure Q_OBJECT-containing public headers are processed by automoc (generate moc_* for metaobjects)
list(APPEND PPSDK_SOURCES
    ${CMAKE_SOURCE_DIR}/include/SDK/PaymentProcessor/Core/IRemoteClient.h
)

ek_add_library(PPSDK
    SOURCES ${PPSDK_SOURCES}
    QT_MODULES Core Widgets
    INCLUDE_DIRS
    DEPENDS ek_boost ek_common
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
