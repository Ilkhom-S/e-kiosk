# UpdateEngine module

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UPDATEENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# Windows-specific BITS files
if(WIN32)
    list(APPEND UPDATEENGINE_SOURCES
        src/WindowsBITS.cpp
        src/WindowsBITS.h
        src/WindowsBITS_p.cpp
        src/WindowsBITS_p.h
    )
endif()

# XmlPatterns is deprecated in Qt6
if(QT_VERSION_MAJOR EQUAL 5)
    ek_add_library(UpdateEngine
        FOLDER "modules"
        SOURCES ${UPDATEENGINE_SOURCES} ${CMAKE_SOURCE_DIR}/include/UpdateEngine/ReportBuilder.h ${CMAKE_SOURCE_DIR}/include/UpdateEngine/Updater.h
        QT_MODULES Core Network Xml XmlPatterns
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS NetworkTaskManager
        COMPILE_DEFINITIONS _UNICODE UNICODE
    )
else()
    # Qt6 doesn't have XmlPatterns - use Core5Compat or alternative
    ek_add_library(UpdateEngine
        FOLDER "modules"
        SOURCES ${UPDATEENGINE_SOURCES} ${CMAKE_SOURCE_DIR}/include/UpdateEngine/ReportBuilder.h ${CMAKE_SOURCE_DIR}/include/UpdateEngine/Updater.h
        QT_MODULES Core Network Xml
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS NetworkTaskManager
        COMPILE_DEFINITIONS _UNICODE UNICODE QT6_NO_XMLPATTERNS
    )
endif()
