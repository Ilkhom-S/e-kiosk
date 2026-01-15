# CryptEngine module

file(GLOB CRYPTENGINE_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(CryptEngine
    SOURCES ${CRYPTENGINE_SOURCES}
    QT_MODULES Core
    DEPENDS boost libipriv
)
