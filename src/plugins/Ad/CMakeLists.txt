# Ad plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(AD_SOURCES
    src/AdPluginFactory.cpp
    src/AdPluginFactory.h
    src/AdPluginImpl.cpp
    src/AdPluginImpl.h
    src/AdService.cpp
    src/AdService.h
    src/AdPayment.cpp
    src/AdPayment.h
    src/AdPaymentRequest.cpp
    src/AdPaymentRequest.h
    src/AdRemotePlugin.cpp
    src/AdRemotePlugin.h
    src/AdSourcePlugin.cpp
    src/AdSourcePlugin.h
    src/PaymentFactory.cpp
    src/PaymentFactory.h
    src/PaymentFactoryBase.cpp
    src/PaymentFactoryBase.h
    # Include application constants that plugins need
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src/Services/ServiceNames.cpp
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src/Services/ServiceNames.h
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src/System/SettingsConstants.cpp
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src/System/SettingsConstants.h
)

set(AD_VERSION_RC ${CMAKE_SOURCE_DIR}/include/Common/CoreVersion.rc)

# Add explicit header used by qbs (IAdSource) so IDEs and build systems see it
list(APPEND AD_SOURCES ${CMAKE_SOURCE_DIR}/include/SDK/GUI/IAdSource.h)


ek_add_plugin(ad
    FOLDER "plugins/BusinessLogic"
    SOURCES ${AD_SOURCES} ${AD_VERSION_RC}
    QT_MODULES Core Sql Network Qml
    DEPENDS
    SysUtils
    PaymentBase
    NetworkTaskManager
    AdBackend
    PluginsSDK
    PPSDK
    DriversSDK
    ek_common
    ek_boost
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src
    INSTALL_DIR plugins
)
