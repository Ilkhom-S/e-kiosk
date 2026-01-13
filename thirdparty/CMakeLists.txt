# Consolidated thirdparty CMake helper
# Scans the `thirdparty/` folder and adds any subdirectory that contains a CMakeLists.txt
# so vendored libraries are available to the top-level build via their exported targets.

cmake_minimum_required(VERSION 3.16)
project(ek_thirdparty NONE)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB children RELATIVE ${THIRD_PARTY_DIR} ${THIRD_PARTY_DIR}/*)
foreach(child ${children})
    if(IS_DIRECTORY "${THIRD_PARTY_DIR}/${child}")
        if(EXISTS "${THIRD_PARTY_DIR}/${child}/CMakeLists.txt")
            message(STATUS "[thirdparty] Adding ${child}")
            add_subdirectory("${THIRD_PARTY_DIR}/${child}" "${CMAKE_BINARY_DIR}/thirdparty/${child}")
        else()
            message(STATUS "[thirdparty] Skipping ${child} (no CMakeLists.txt)")
        endif()
    endif()
endforeach()
