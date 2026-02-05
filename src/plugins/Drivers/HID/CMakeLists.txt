# HID driver plugin

if(WIN32)
    include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

    file(GLOB HID_SOURCES
        src/*.cpp
        src/*.h
        ../../../includes/Common/CoreVersion.rc
    )

    ek_add_driver_plugin(hid_devices
        SOURCES ${HID_SOURCES}
        QT_MODULES Core Gui
        DEPENDS
        DriversSDK
        HardwareCommon
        HardwareIOPorts
        HardwareScanners
        # OPOSSDK # Need to migrate to native SDK from Device manufacturers
        SysUtils
    )

    # Translations - merge HID and common translations
    file(GLOB TS_FILES_PLUGIN src/locale/*.ts)
    file(GLOB TS_FILES_COMMON ../../../modules/Hardware/Common/src/locale/*.ts)
    set(TS_FILES ${TS_FILES_PLUGIN} ${TS_FILES_COMMON})
    if(TS_FILES)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
        ek_add_translations(hid_devices
            SOURCES ${TS_FILES}
            OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/locale
        )
    endif()
endif()
