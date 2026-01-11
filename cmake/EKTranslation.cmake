# EKTranslation.cmake - Qt translation helpers

function(ek_add_translations TARGET_NAME)
    set(options INSTALL)
    set(oneValueArgs OUTPUT_DIR TS_DIR)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools QUIET)
    if(NOT Qt${QT_VERSION_MAJOR}_LinguistTools_FOUND)
        message(WARNING "Qt LinguistTools not found. Translation support disabled.")
        return()
    endif()

    set(_qm_files)
    foreach(_ts_file ${ARG_SOURCES})
        get_filename_component(_ts_name ${_ts_file} NAME_WE)
        set(_qm_file "${ARG_OUTPUT_DIR}/${_ts_name}.qm")
        add_custom_command(
            OUTPUT ${_qm_file}
            COMMAND Qt${QT_VERSION_MAJOR}::lrelease ${_ts_file} -qm ${_qm_file}
            DEPENDS ${_ts_file}
            COMMENT "Compiling translation ${_ts_name}.qm"
            VERBATIM
        )
        list(APPEND _qm_files ${_qm_file})
    endforeach()
    add_custom_target(${TARGET_NAME}_translations ALL DEPENDS ${_qm_files})
    if(TARGET ${TARGET_NAME})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_translations)
    endif()
endfunction()
