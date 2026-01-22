# TCFindLibUSB.cmake - LibUSB library builder for EKiosk

if(NOT TARGET EK::LibUSB)
    # Build LibUSB from source
    set(LIBUSB_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/core.c
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/descriptor.c
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/hotplug.c
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/io.c
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/strerror.c
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/sync.c
    )

    # Platform-specific sources
    if(WIN32)
        list(APPEND LIBUSB_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/events_windows.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/threads_windows.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/windows_common.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/windows_usbdk.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/windows_winusb.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/libusb-1.0.def
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/libusb-1.0.rc
        )
    elseif(UNIX AND NOT APPLE)
        list(APPEND LIBUSB_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/events_posix.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/threads_posix.c
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb/os/linux_usbfs.c
        )
    endif()

    # Create static library
    add_library(libusb_static STATIC ${LIBUSB_SOURCES})

    # Set include directories
    target_include_directories(libusb_static
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb>
        $<INSTALL_INTERFACE:include>
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../libusb
        ${CMAKE_CURRENT_LIST_DIR}/../libusb/libusb
    )

    # Add config.h include for Windows, macOS, and Linux
    if(WIN32)
        target_include_directories(libusb_static
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/msvc>
            INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/msvc>
            PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/msvc
        )
    elseif(APPLE)
        target_include_directories(libusb_static
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/Xcode>
            INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/Xcode>
            PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/Xcode
        )
    elseif(UNIX AND NOT APPLE)
        target_include_directories(libusb_static
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/android>
            INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../libusb/android>
            PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/../libusb/android
        )
    endif()

    # Compiler definitions
    target_compile_definitions(libusb_static
        PRIVATE
        ENABLE_LOGGING
        ENABLE_DEBUG_LOGGING
    )

    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(libusb_static PRIVATE _WIN32_WINNT=0x0600)
        target_link_libraries(libusb_static PRIVATE setupapi)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(libusb_static PRIVATE pthread)
    endif()

    # Set library properties
    set_target_properties(libusb_static PROPERTIES
        OUTPUT_NAME "libusb-1.0"
        POSITION_INDEPENDENT_CODE ON
    )

    # Create alias for EKiosk
    add_library(EK::LibUSB ALIAS libusb_static)
    message(STATUS "Built LibUSB from source")
endif()
