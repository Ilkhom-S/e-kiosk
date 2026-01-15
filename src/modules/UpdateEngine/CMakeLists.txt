# UpdateEngine module

file(GLOB UPDATEENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# XmlPatterns is deprecated in Qt6
if(QT_VERSION_MAJOR EQUAL 5)
    tc_add_library(UpdateEngine
        SOURCES ${UPDATEENGINE_SOURCES}
        QT_MODULES Core Network Xml XmlPatterns
        DEPENDS NetworkTaskManager
    )
else()
    # Qt6 doesn't have XmlPatterns - use Core5Compat or alternative
    ek_add_library(UpdateEngine
        SOURCES ${UPDATEENGINE_SOURCES}
        QT_MODULES Core Network Xml
        DEPENDS NetworkTaskManager
    )
    target_compile_definitions(UpdateEngine PRIVATE QT6_NO_XMLPATTERNS)
endif()
