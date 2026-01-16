# EKiosk Thirdparty Dependencies
# This file configures all thirdparty libraries and provides clean targets for modules to link against.

cmake_minimum_required(VERSION 3.16)
project(ek_thirdparty NONE)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# =============================================================================
# SingleApplication - Qt Single Application Library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/SingleApplication")
    message(STATUS "[thirdparty] Configuring SingleApplication")

    # Configure SingleApplication with EKiosk settings
    set(QT_DEFAULT_MAJOR_VERSION ${QT_VERSION_MAJOR} CACHE STRING "Qt version to use" FORCE)
    set(QAPPLICATION_CLASS QApplication CACHE STRING "Qt application base class" FORCE)
    set(SINGLEAPPLICATION_INSTALL OFF CACHE BOOL "Disable install" FORCE)

    # Add the SingleApplication subdirectory
    add_subdirectory(SingleApplication)

    # Provide clean alias target
    if(NOT TARGET EK::SingleApplication)
        add_library(EK::SingleApplication ALIAS SingleApplication)
    endif()
    # Place the real target into the thirdparty folder in IDEs if present
    if(TARGET SingleApplication)
        set_target_properties(SingleApplication PROPERTIES FOLDER "thirdparty/SingleApplication")
    endif()

    message(STATUS "[thirdparty] SingleApplication configured as EK::SingleApplication")
else()
    message(WARNING "SingleApplication submodule not found")
endif()

# =============================================================================
# SmsMessage - SMS PDU encoding/decoding library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/SmsMessage")
    message(STATUS "[thirdparty] Configuring SmsMessage")

    # Configure SmsMessage
    set(SMSMESSAGE_BUILD_TESTS OFF CACHE BOOL "Disable tests" FORCE)
    set(SMSMESSAGE_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
    set(SMSMESSAGE_BUILD_SHARED OFF CACHE BOOL "Build static" FORCE)
    set(SMSMESSAGE_INSTALL OFF CACHE BOOL "Disable install" FORCE)

    # Add the SmsMessage subdirectory
    add_subdirectory(SmsMessage)

    # Provide clean alias target
    if(NOT TARGET EK::SmsMessage)
        add_library(EK::SmsMessage ALIAS SmsMessage)
    endif()
    if(TARGET SmsMessage)
        set_target_properties(SmsMessage PROPERTIES FOLDER "thirdparty/SmsMessage")
    endif()

    message(STATUS "[thirdparty] SmsMessage configured as EK::SmsMessage")
else()
    message(WARNING "SmsMessage submodule not found")
endif()

# =============================================================================
# qntp - NTP client library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/qntp")
    message(STATUS "[thirdparty] Configuring qntp")

    # Add the qntp subdirectory
    add_subdirectory(qntp)

    # Provide clean alias target
    if(NOT TARGET EK::qntp)
        add_library(EK::qntp ALIAS qntp)
    endif()
    if(TARGET qntp)
        set_target_properties(qntp PROPERTIES FOLDER "thirdparty/qntp")
    endif()

    message(STATUS "[thirdparty] qntp configured as EK::qntp")
else()
    message(WARNING "qntp submodule not found")
endif()

# =============================================================================
# libipriv - Private key library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/libipriv")
    message(STATUS "[thirdparty] Configuring libipriv")

    # Add the libipriv subdirectory
    add_subdirectory(libipriv)

    # Provide clean alias target
    if(NOT TARGET EK::libipriv)
        add_library(EK::libipriv ALIAS libipriv)
    endif()
    if(TARGET libipriv)
        set_target_properties(libipriv PROPERTIES FOLDER "thirdparty/libipriv")
    endif()

    message(STATUS "[thirdparty] libipriv configured as EK::libipriv")
else()
    message(WARNING "libipriv submodule not found")
endif()

# =============================================================================
# LibUSB - USB library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/libusb")
    message(STATUS "[thirdparty] Configuring LibUSB")

    # Include the LibUSB finder/builder
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/TCFindLibUSB.cmake)

    message(STATUS "[thirdparty] LibUSB configured as EK::LibUSB")
else()
    message(WARNING "LibUSB submodule not found")
endif()

# =============================================================================
# Zint - Barcode generator library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/zint")
    message(STATUS "[thirdparty] Configuring Zint")

    # Configure Zint options
    set(ZINT_SHARED OFF CACHE BOOL "Disable shared library" FORCE)
    set(ZINT_STATIC ON CACHE BOOL "Enable static library" FORCE)
    set(ZINT_FRONTEND OFF CACHE BOOL "Disable frontend" FORCE)
    set(ZINT_USE_QT ON CACHE BOOL "Enable Qt support" FORCE)
    set(ZINT_QT6 OFF CACHE BOOL "Use Qt5" FORCE)
    set(ZINT_USE_PNG OFF CACHE BOOL "Disable PNG support" FORCE)
    set(ZINT_USE_GS1SE OFF CACHE BOOL "Disable GS1 Syntax Engine" FORCE)
    set(ZINT_UNINSTALL OFF CACHE BOOL "Disable uninstall" FORCE)

    # Add the zint subdirectory
    add_subdirectory(zint)

    # Provide clean alias targets
    if(NOT TARGET EK::QZint)
        add_library(EK::QZint ALIAS QZint)
    endif()

    if(NOT TARGET EK::zint)
        add_library(EK::zint ALIAS zint-static)
    endif()
    if(TARGET QZint)
        set_target_properties(QZint PROPERTIES FOLDER "thirdparty/zint")
    endif()
    if(TARGET zint-static)
        set_target_properties(zint-static PROPERTIES FOLDER "thirdparty/zint")
    endif()

    message(STATUS "[thirdparty] Zint configured as EK::QZint and EK::zint")
else()
    message(WARNING "Zint submodule not found")
endif()

# =============================================================================
# DelayImpHlp - Delay import helper (header-only)
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/DelayImpHlp")
    message(STATUS "[thirdparty] Configuring DelayImpHlp")

    # DelayImpHlp is header-only, create interface target
    if(NOT TARGET DelayImpHlp)
        add_library(DelayImpHlp INTERFACE)
        target_include_directories(DelayImpHlp INTERFACE "${THIRD_PARTY_DIR}/DelayImpHlp")
        add_library(EK::DelayImpHlp ALIAS DelayImpHlp)
    endif()

    if(TARGET DelayImpHlp)
        set_target_properties(DelayImpHlp PROPERTIES FOLDER "thirdparty/DelayImpHlp")
    endif()

    message(STATUS "[thirdparty] DelayImpHlp configured as EK::DelayImpHlp")
else()
    message(WARNING "DelayImpHlp not found")
endif()

# Attempt to place any known thirdparty aliases into the thirdparty folder for IDEs
if(TARGET EK::LibUSB)
    # If underlying LibUSB target exists it will be grouped as well by CMake
    set_target_properties(EK::LibUSB PROPERTIES FOLDER "thirdparty/LibUSB")
endif()

message(STATUS "[thirdparty] Thirdparty configuration complete")
message(STATUS "[thirdparty] Available targets: EK::SingleApplication, EK::SmsMessage, EK::qntp, EK::libipriv, EK::LibUSB, EK::QZint, EK::zint, EK::DelayImpHlp")
message(STATUS "[thirdparty] Available targets: EK::SingleApplication, EK::SmsMessage, EK::qntp, EK::libipriv, EK::LibUSB, EK::QZint, EK::zint, EK::DelayImpHlp")
