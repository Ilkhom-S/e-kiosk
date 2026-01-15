# Ad plugin

file(GLOB AD_SOURCES
    src/*.cpp
    src/*.h
)

# Use project-relative paths (PROJECT_SOURCE_DIR points to 3.0/src)
set(AD_VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

# Add explicit header used by qbs (IAdSource) so IDEs and build systems see it
list(APPEND AD_SOURCES ${TC_INCLUDES_DIR}/SDK/GUI/IAdSource.h)

tc_add_plugin(ad
    SOURCES ${AD_SOURCES} ${AD_VERSION_RC}
    QT_MODULES Core Sql Network Qml
    DEPENDS
    SysUtils
    PaymentBase
    NetworkTaskManager
    AdBackend
    INCLUDE_DIRS
    ${TC_SRC_ROOT}/apps/PaymentProcessor/src
    ${TC_INCLUDES_DIR}/SDK/GUI
    INSTALL_DIR plugins
)
