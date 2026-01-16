# DatabaseProxy module

file(GLOB DATABASEPROXY_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(DatabaseProxy FOLDER "modules/DatabaseProxy" SOURCES ${DATABASEPROXY_SOURCES}
    QT_MODULES Core Sql
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
)
