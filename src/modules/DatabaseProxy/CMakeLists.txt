# DatabaseProxy module

file(GLOB DATABASEPROXY_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(DatabaseProxy
    SOURCES ${DATABASEPROXY_SOURCES}
    QT_MODULES Core Sql
)
