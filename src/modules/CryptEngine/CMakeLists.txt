# CryptEngine module

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB CRYPTENGINE_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(CryptEngine
    SOURCES ${CRYPTENGINE_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS boost libipriv
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
