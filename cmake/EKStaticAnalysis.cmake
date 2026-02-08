# EKStaticAnalysis.cmake - Static analysis integration for EKiosk
#
# Provides functions to add static analysis (cppcheck, clang-tidy, etc.)
# to CMake targets in a consistent, cross-platform way.
#
# Usage:
#   include(cmake/EKStaticAnalysis.cmake)
#   ek_enable_static_analysis(<target> [CPPCHECK] [CLANG_TIDY])
#
# Options:
#   CPPCHECK    Enable cppcheck for the target
#   CLANG_TIDY  Enable clang-tidy for the target
#
# This module is Qt5/Qt6 agnostic and can be extended for more tools.

function(ek_enable_static_analysis target)
    set(options CPPCHECK CLANG_TIDY)
    cmake_parse_arguments(EKSA "${options}" "" "" ${ARGN})

    if(EKSA_CPPCHECK)
        find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
        if(CPPCHECK_EXECUTABLE)
            set(CPPCHECK_ARGS
                ${CPPCHECK_EXECUTABLE}
                --enable=all
                --inconclusive
                --force
                --inline-suppr
                --std=c++14
            )

            # Exclude third-party vendored code and vcpkg-installed artifacts from scans
            if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty")
                list(APPEND CPPCHECK_ARGS --exclude "${CMAKE_SOURCE_DIR}/thirdparty")
            endif()
            if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg_installed")
                list(APPEND CPPCHECK_ARGS --exclude "${CMAKE_SOURCE_DIR}/vcpkg_installed")
            endif()

            set_target_properties(${target} PROPERTIES
                CXX_CPPCHECK "${CPPCHECK_ARGS}"
            )
        else()
            message(WARNING "cppcheck not found: static analysis will be skipped for ${target}")
        endif()
    endif()

    if(EKSA_CLANG_TIDY)
        find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
        if(CLANG_TIDY_EXECUTABLE)
            set_target_properties(${target} PROPERTIES
                CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};--header-filter=^(?!.*(thirdparty|vcpkg_installed|build|external|moc_|ui_|qrc_)).*\\.(h|hpp)$"
            )
        else()
            message(WARNING "clang-tidy not found: static analysis will be skipped for ${target}")
        endif()
    endif()
endfunction()

# Example:
# ek_enable_static_analysis(myapp CPPCHECK CLANG_TIDY)
