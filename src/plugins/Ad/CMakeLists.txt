# Ad plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB AD_SOURCES
    src/*.cpp
    src/*.h
)

# Add explicit header used by qbs (IAdSource) so IDEs and build systems see it
list(APPEND AD_SOURCES ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IAdSource.h)

ek_add_plugin(ad
    SOURCES ${AD_SOURCES}
    QT_MODULES Core Sql Network Qml
    DEPENDS
    SysUtils
    PaymentBase
    NetworkTaskManager
    AdBackend
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/SDK/GUI
    INSTALL_DIR plugins
)
