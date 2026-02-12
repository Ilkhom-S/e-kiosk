# EKFindZint.cmake - Zint barcode library builder for EKiosk
# Only builds the backend (zint-static) and Qt wrapper (QZint), not the GUI application

if(NOT TARGET EK::QZint)
    set(ZINT_DIR "${CMAKE_CURRENT_LIST_DIR}/../zint")

    # Get Zint version from zint.h
    if(EXISTS "${ZINT_DIR}/backend/zint.h")
        file(STRINGS "${ZINT_DIR}/backend/zint.h" ZINT_VERSION_MAJOR_LINE REGEX "^#define[ \t]+ZINT_VERSION_MAJOR[ \t]+[0-9]+")
        file(STRINGS "${ZINT_DIR}/backend/zint.h" ZINT_VERSION_MINOR_LINE REGEX "^#define[ \t]+ZINT_VERSION_MINOR[ \t]+[0-9]+")
        file(STRINGS "${ZINT_DIR}/backend/zint.h" ZINT_VERSION_RELEASE_LINE REGEX "^#define[ \t]+ZINT_VERSION_RELEASE[ \t]+[0-9]+")
        string(REGEX REPLACE "^#define[ \t]+ZINT_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" ZINT_VERSION_MAJOR "${ZINT_VERSION_MAJOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+ZINT_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" ZINT_VERSION_MINOR "${ZINT_VERSION_MINOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+ZINT_VERSION_RELEASE[ \t]+([0-9]+).*" "\\1" ZINT_VERSION_RELEASE "${ZINT_VERSION_RELEASE_LINE}")
        set(ZINT_VERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}.${ZINT_VERSION_RELEASE}")
    else()
        set(ZINT_VERSION "2.13.0")
        set(ZINT_VERSION_MAJOR "2")
        set(ZINT_VERSION_MINOR "13")
    endif()

    # =========================================================================
    # Backend library (zint-static) - Core barcode generation
    # =========================================================================
    set(ZINT_COMMON_SRCS
        ${ZINT_DIR}/backend/common.c
        ${ZINT_DIR}/backend/eci.c
        ${ZINT_DIR}/backend/filemem.c
        ${ZINT_DIR}/backend/general_field.c
        ${ZINT_DIR}/backend/gs1.c
        ${ZINT_DIR}/backend/large.c
        ${ZINT_DIR}/backend/library.c
        ${ZINT_DIR}/backend/reedsol.c
    )

    set(ZINT_ONEDIM_SRCS
        ${ZINT_DIR}/backend/2of5.c
        ${ZINT_DIR}/backend/2of5inter.c
        ${ZINT_DIR}/backend/2of5inter_based.c
        ${ZINT_DIR}/backend/bc412.c
        ${ZINT_DIR}/backend/channel.c
        ${ZINT_DIR}/backend/codabar.c
        ${ZINT_DIR}/backend/code.c
        ${ZINT_DIR}/backend/code11.c
        ${ZINT_DIR}/backend/code128.c
        ${ZINT_DIR}/backend/code128_based.c
        ${ZINT_DIR}/backend/dxfilmedge.c
        ${ZINT_DIR}/backend/medical.c
        ${ZINT_DIR}/backend/plessey.c
        ${ZINT_DIR}/backend/rss.c
        ${ZINT_DIR}/backend/telepen.c
        ${ZINT_DIR}/backend/upcean.c
    )

    set(ZINT_POSTAL_SRCS
        ${ZINT_DIR}/backend/auspost.c
        ${ZINT_DIR}/backend/imail.c
        ${ZINT_DIR}/backend/mailmark.c
        ${ZINT_DIR}/backend/postal.c
    )

    set(ZINT_TWODIM_SRCS
        ${ZINT_DIR}/backend/aztec.c
        ${ZINT_DIR}/backend/codablock.c
        ${ZINT_DIR}/backend/code1.c
        ${ZINT_DIR}/backend/code16k.c
        ${ZINT_DIR}/backend/code49.c
        ${ZINT_DIR}/backend/composite.c
        ${ZINT_DIR}/backend/dmatrix.c
        ${ZINT_DIR}/backend/dotcode.c
        ${ZINT_DIR}/backend/gridmtx.c
        ${ZINT_DIR}/backend/hanxin.c
        ${ZINT_DIR}/backend/maxicode.c
        ${ZINT_DIR}/backend/pdf417.c
        ${ZINT_DIR}/backend/qr.c
        ${ZINT_DIR}/backend/ultra.c
    )

    set(ZINT_OUTPUT_SRCS
        ${ZINT_DIR}/backend/bmp.c
        ${ZINT_DIR}/backend/emf.c
        ${ZINT_DIR}/backend/gif.c
        ${ZINT_DIR}/backend/output.c
        ${ZINT_DIR}/backend/pcx.c
        ${ZINT_DIR}/backend/ps.c
        ${ZINT_DIR}/backend/raster.c
        ${ZINT_DIR}/backend/svg.c
        ${ZINT_DIR}/backend/tif.c
        ${ZINT_DIR}/backend/vector.c
    )
    
    # PNG support is optional - only include if libpng is available
    find_package(PNG QUIET)
    if(PNG_FOUND)
        list(APPEND ZINT_OUTPUT_SRCS ${ZINT_DIR}/backend/png.c)
        set(ZINT_HAS_PNG TRUE)
    else()
        set(ZINT_HAS_PNG FALSE)
        message(STATUS "[thirdparty] PNG support disabled for Zint (libpng not found)")
    endif()

    # Create zint-static library
    add_library(zint-static STATIC
        ${ZINT_COMMON_SRCS}
        ${ZINT_ONEDIM_SRCS}
        ${ZINT_POSTAL_SRCS}
        ${ZINT_TWODIM_SRCS}
        ${ZINT_OUTPUT_SRCS}
    )

    target_include_directories(zint-static
        PUBLIC
        $<BUILD_INTERFACE:${ZINT_DIR}/backend>
        $<INSTALL_INTERFACE:include>
    )
    
    # Link libpng if available
    if(ZINT_HAS_PNG)
        target_link_libraries(zint-static PRIVATE PNG::PNG)
    else()
        target_compile_definitions(zint-static PRIVATE NO_PNG)
    endif()

    # Platform-specific settings
    if(WIN32)
        set_target_properties(zint-static PROPERTIES OUTPUT_NAME zint-static)
    else()
        set_target_properties(zint-static PROPERTIES OUTPUT_NAME zint)
    endif()

    set_target_properties(zint-static PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    # Suppress warnings for third-party code
    if(MSVC)
        target_compile_options(zint-static PRIVATE /W0)
    else()
        target_compile_options(zint-static PRIVATE -w)
    endif()

    # =========================================================================
    # Qt wrapper library (QZint) - Qt integration for zint
    # =========================================================================
    set(QZINT_SRCS
        ${ZINT_DIR}/backend_qt/qzint.cpp
        ${ZINT_DIR}/backend_qt/qzint.h
    )

    # Create QZint library
    add_library(QZint STATIC ${QZINT_SRCS})

    # Set Qt-specific properties
    set_target_properties(QZint PROPERTIES
        AUTOMOC ON
        SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
        VERSION ${ZINT_VERSION}
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(QZint
        PUBLIC
        $<BUILD_INTERFACE:${ZINT_DIR}/backend_qt>
        $<BUILD_INTERFACE:${ZINT_DIR}/backend>
        $<INSTALL_INTERFACE:include>
    )

    # Link dependencies
    target_link_libraries(QZint
        PUBLIC
        zint-static
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
    )

    # Suppress warnings for third-party code
    if(MSVC)
        target_compile_options(QZint PRIVATE /W0)
    else()
        target_compile_options(QZint PRIVATE -w)
    endif()

    # Create aliases for EKiosk
    add_library(EK::zint ALIAS zint-static)
    add_library(EK::QZint ALIAS QZint)

    message(STATUS "[thirdparty] Built Zint ${ZINT_VERSION} (backend + Qt wrapper only, no GUI)")
endif()
