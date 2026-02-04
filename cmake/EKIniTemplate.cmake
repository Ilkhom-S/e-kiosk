# Macro: ek_generate_ini_for_all_configs(NAME TEMPLATE)
# Generates ini files for all configs (Debug, Release, etc.) in the correct output directory, matching the executable name.
macro(ek_generate_ini_for_all_configs NAME TEMPLATE)
    set(_ekiosk_configs Debug Release RelWithDebInfo MinSizeRel)
    foreach(_cfg ${_ekiosk_configs})
        string(TOUPPER "${_cfg}" _CFG_UPPER)
        set(_ini_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_UPPER}}")
        if(NOT _ini_output_dir)
            set(_ini_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${_cfg}")
        endif()
        set(WORKING_DIRECTORY "${_ini_output_dir}")
        ek_generate_ini_template(
            ${NAME}
            "${TEMPLATE}"
            "${_ini_output_dir}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        )
    endforeach()
endmacro()
# EKiosk CMake helper: ek_generate_ini_template
#
# Usage:
#   ek_generate_ini_template(<name> <template> <output_dir> [VAR value ...])
#
# Example:
#   ek_generate_ini_template(controller "${CMAKE_SOURCE_DIR}/runtimes/common/data/controller.ini.in" "${CMAKE_BINARY_DIR}/apps/WatchServiceController" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#
# This macro configures a .ini file from a template, substituting CMake variables.
# Automatically selects platform-specific templates if available:
#   - <template>.win.in for Windows
#   - <template>.mac.in for macOS
#   - <template>.linux.in for Linux
#   Falls back to <template>.in if platform-specific version doesn't exist

function(ek_generate_ini_template NAME TEMPLATE OUTPUT_DIR)
    # Determine platform-specific template path
    # Replace .in extension with platform-specific extension
    string(REGEX REPLACE "\\.in$" "" _template_base "${TEMPLATE}")
    if(WIN32)
        set(_platform_template "${_template_base}.win.in")
    elseif(APPLE)
        set(_platform_template "${_template_base}.mac.in")
    elseif(UNIX)
        set(_platform_template "${_template_base}.linux.in")
    else()
        set(_platform_template "${TEMPLATE}")
    endif()

    # Use platform-specific template if it exists, otherwise use the generic one
    if(EXISTS "${_platform_template}")
        set(_actual_template "${_platform_template}")
        message(STATUS "Using platform-specific template: ${_actual_template}")
    else()
        set(_actual_template "${TEMPLATE}")
        message(STATUS "Using generic template: ${_actual_template}")
    endif()

    # Process user-provided variables
    set(_i 0)
    list(LENGTH ARGN _len)
    while(_i LESS _len)
        list(GET ARGN ${_i} _var)
        math(EXPR _j "${_i} + 1")
        list(GET ARGN ${_j} _val)
        set(${_var} "${_val}")
        math(EXPR _i "${_i} + 2")
    endwhile()

    set(_out_ini "${OUTPUT_DIR}/${NAME}.ini")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    configure_file("${_actual_template}" "${_out_ini}" @ONLY)
    message(STATUS "Generated ini: ${_out_ini} from ${_actual_template}")
endfunction()
