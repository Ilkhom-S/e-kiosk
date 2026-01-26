# NetworkTaskManager module

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB NETWORKTASKMANAGER_SOURCES
    src/*.cpp
    src/*.h
)

# Add public headers for moc generation
file(GLOB NETWORKTASKMANAGER_PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/NetworkTaskManager/*.h
)

# Add resource files
file(GLOB NETWORKTASKMANAGER_RESOURCES
    res/*.qrc
)

# Ensure Qt components are found for this module
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)

ek_add_library(NetworkTaskManager
    FOLDER "modules"
    SOURCES ${NETWORKTASKMANAGER_SOURCES} ${NETWORKTASKMANAGER_PUBLIC_HEADERS} ${NETWORKTASKMANAGER_RESOURCES}
    QT_MODULES Core Network
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Log
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
