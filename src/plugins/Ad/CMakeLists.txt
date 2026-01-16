# Ad plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB AD_SOURCES
    src/*.cpp
    src/*.h
)

set(AD_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)

# Add explicit header used by qbs (IAdSource) so IDEs and build systems see it
list(APPEND AD_SOURCES ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IAdSource.h)

ek_add_plugin(ad
    FOLDER "plugins"
    SOURCES ${AD_SOURCES} ${AD_VERSION_RC}
    QT_MODULES Core Sql Network Qml
    DEPENDS
    SysUtils
    PaymentBase
    NetworkTaskManager
    AdBackend
    PluginsSDK
    PPSDK
    ek_common
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/apps/kiosk/src
    INSTALL_DIR plugins
)
