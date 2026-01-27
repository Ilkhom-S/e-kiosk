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

    # Translations
    file(GLOB TS_FILES src/locale/*.ts)
    if(TS_FILES)
        ek_add_translations(HIDTranslations
            SOURCES ${TS_FILES}
            OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale
        )
    endif()
endif()
