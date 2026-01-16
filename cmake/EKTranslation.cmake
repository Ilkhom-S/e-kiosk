# EKTranslation.cmake - Qt translation helpers

option(EK_ENABLE_TRANSLATIONS "Enable building translations (Qt LinguistTools or lrelease required)" ON)

function(ek_add_translations TARGET_NAME)
    set(options INSTALL)
    set(oneValueArgs OUTPUT_DIR TS_DIR)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EK_ENABLE_TRANSLATIONS)
        message(STATUS "Translation support explicitly disabled via EK_ENABLE_TRANSLATIONS=OFF")
        return()
    endif()

    # First try to use CMake's Qt package detection for LinguistTools
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools QUIET)
    set(_use_qt_lrelease FALSE)
    set(_lrelease_exec "")
    if(Qt${QT_VERSION_MAJOR}_LinguistTools_FOUND)
        set(_use_qt_lrelease TRUE)
    else()
        # Fall back to finding lrelease in PATH
        find_program(LRELEASE_EXECUTABLE NAMES lrelease lrelease.exe)
        if(LRELEASE_EXECUTABLE)
            set(_lrelease_exec "${LRELEASE_EXECUTABLE}")
            # Print where lrelease was found only once to avoid noisy repeated messages
            if(NOT DEFINED EK__LRELEASE_PRINTED)
                set(EK__LRELEASE_PRINTED TRUE CACHE INTERNAL "Whether lrelease path was already printed")
                message(STATUS "Found lrelease at: ${_lrelease_exec}")
            endif()
        else()
            message(WARNING "Qt LinguistTools not found and 'lrelease' not found in PATH. Translation support disabled.")
            message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
            return()
        endif()
    endif()

    set(_qm_files)
    foreach(_ts_file ${ARG_SOURCES})
        get_filename_component(_ts_name ${_ts_file} NAME_WE)
        set(_qm_file "${ARG_OUTPUT_DIR}/${_ts_name}.qm")
        if(_use_qt_lrelease)
            add_custom_command(
                OUTPUT ${_qm_file}
                COMMAND Qt${QT_VERSION_MAJOR}::lrelease ${_ts_file} -qm ${_qm_file}
                DEPENDS ${_ts_file}
                COMMENT "Compiling translation ${_ts_name}.qm"
                VERBATIM
            )
        else()
            add_custom_command(
                OUTPUT ${_qm_file}
                COMMAND ${_lrelease_exec} ${_ts_file} -qm ${_qm_file}
                DEPENDS ${_ts_file}
                COMMENT "Compiling translation ${_ts_name}.qm (using external lrelease)"
                VERBATIM
            )
        endif()
        list(APPEND _qm_files ${_qm_file})
    endforeach()
    add_custom_target(${TARGET_NAME}_translations ALL DEPENDS ${_qm_files})
    if(TARGET ${TARGET_NAME})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_translations)
    endif()
endfunction()
