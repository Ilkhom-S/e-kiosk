# EKiosk Thirdparty Dependencies
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_CLANG_TIDY
    "clang-tidy"
    "-header-filter=^${CMAKE_SOURCE_DIR}/(src|apps)/" # Scans ONLY your code
)

# Enable CXX and C so sub-libraries can detect compiler features correctly
project(ek_thirdparty LANGUAGES CXX C)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Helper function to clean up IDE folder organization
function(ek_setup_thirdparty_target _target)
    if(TARGET ${_target})
        # 1. Clear Clang-Tidy for this target specifically
        set_target_properties(${_target} PROPERTIES
            CXX_CLANG_TIDY ""
            C_CLANG_TIDY ""
        )

        # 2. Mark as SYSTEM to silence compiler warnings
        set_target_properties(${_target} PROPERTIES
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:${_target},INTERFACE_INCLUDE_DIRECTORIES>"
        )

        # 3. Hide in IDE
        set_target_properties(${_target} PROPERTIES FOLDER "thirdparty")
    endif()
endfunction()


# =============================================================================
# SingleApplication - Qt Single Application Library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/SingleApplication")
    message(STATUS "[thirdparty] Configuring SingleApplication")
    set(QT_DEFAULT_MAJOR_VERSION ${QT_VERSION_MAJOR} CACHE STRING "" FORCE)
    set(QAPPLICATION_CLASS QApplication CACHE STRING "" FORCE)

    add_subdirectory(SingleApplication)

    if(TARGET SingleApplication)
        # PUBLIC ensures anyone linking to EK::SingleApplication gets the headers
        target_include_directories(SingleApplication SYSTEM PUBLIC "${THIRD_PARTY_DIR}/SingleApplication")
        if(NOT TARGET EK::SingleApplication)
            add_library(EK::SingleApplication ALIAS SingleApplication)
        endif()
        ek_setup_thirdparty_target(SingleApplication)
    endif()
endif()

# =============================================================================
# SmsMessage - SMS PDU encoding/decoding
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/SmsMessage")
    message(STATUS "[thirdparty] Configuring SmsMessage")
    set(SMSMESSAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SMSMESSAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMSMESSAGE_BUILD_SHARED OFF CACHE BOOL "" FORCE)

    add_subdirectory(SmsMessage)

    if(TARGET SmsMessage)
        set_target_properties(SmsMessage PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
            $<TARGET_PROPERTY:SmsMessage,INTERFACE_INCLUDE_DIRECTORIES>
        )
        if(NOT TARGET EK::SmsMessage)
            add_library(EK::SmsMessage ALIAS SmsMessage)
        endif()
        ek_setup_thirdparty_target(SmsMessage)
    endif()
endif()

# =============================================================================
# qntp - NTP client library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/qntp")
    message(STATUS "[thirdparty] Configuring qntp")
    add_subdirectory(qntp)

    if(TARGET qntp)
        if(NOT TARGET EK::qntp)
            add_library(EK::qntp ALIAS qntp)
        endif()
        ek_setup_thirdparty_target(qntp)
    endif()
endif()

# =============================================================================
# libipriv - Private key library
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/libipriv")
    message(STATUS "[thirdparty] Configuring libipriv")
    add_subdirectory(libipriv)

    if(TARGET libipriv)
        # Silence warnings when BUILDING libipriv itself
        if(MSVC)
            target_compile_options(libipriv PRIVATE /W0)
        else()
            target_compile_options(libipriv PRIVATE -w) # '-w' suppresses ALL warnings in GCC/Clang
        endif()
        if(NOT TARGET EK::libipriv)
            add_library(EK::libipriv ALIAS libipriv)
        endif()
        ek_setup_thirdparty_target(libipriv)
    endif()
endif()

# =============================================================================
# LibUSB
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/libusb")
    message(STATUS "[thirdparty] Configuring LibUSB")
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/TCFindLibUSB.cmake)
endif()

# =============================================================================
# Zint - Barcode generator (Qt 5/6 Toggle)
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/zint")
    message(STATUS "[thirdparty] Configuring Zint")
    set(ZINT_SHARED OFF CACHE BOOL "" FORCE)
    set(ZINT_STATIC ON CACHE BOOL "" FORCE)
    set(ZINT_FRONTEND OFF CACHE BOOL "" FORCE)
    set(ZINT_USE_QT ON CACHE BOOL "" FORCE)

    if(QT_VERSION_MAJOR EQUAL 6)
        set(ZINT_QT6 ON CACHE BOOL "" FORCE)
    else()
        set(ZINT_QT6 OFF CACHE BOOL "" FORCE)
    endif()

    add_subdirectory(zint)

    # Exclude zint-qt GUI application from default build (we only need QZint library)
    if(TARGET zint-qt)
        set_target_properties(zint-qt PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
        )
        message(STATUS "[thirdparty] Excluding zint-qt GUI application from default build")
    endif()

    if(TARGET QZint AND NOT TARGET EK::QZint)
        add_library(EK::QZint ALIAS QZint)
    endif()
    if(TARGET zint-static AND NOT TARGET EK::zint)
        add_library(EK::zint ALIAS zint-static)
    endif()

    # Apply thirdparty folder organization
    ek_setup_thirdparty_target(QZint)
    ek_setup_thirdparty_target(zint-static)
    if(TARGET zint-qt)
        ek_setup_thirdparty_target(zint-qt)
    endif()
endif()

# =============================================================================
# DelayImpHlp - Header-only helper
# =============================================================================
if(EXISTS "${THIRD_PARTY_DIR}/DelayImpHlp")
    if(NOT TARGET EK::DelayImpHlp)
        add_library(DelayImpHlp INTERFACE)
        target_include_directories(DelayImpHlp SYSTEM INTERFACE "${THIRD_PARTY_DIR}/DelayImpHlp")
        add_library(EK::DelayImpHlp ALIAS DelayImpHlp)
    endif()
    ek_setup_thirdparty_target(DelayImpHlp)
endif()
