# Macro: ek_generate_ini_for_all_configs(NAME TEMPLATE)
# Generates ini files for the app in the correct output directory alongside the executable.
# For multi-config generators (Visual Studio), generates per-config INI files.
# For single-config generators (Unix Makefiles, Ninja), generates one INI to bin folder.
macro(ek_generate_ini_for_all_configs NAME TEMPLATE)
    # Check if using multi-config generator (Visual Studio, Xcode)
    get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    if(_is_multi_config)
        # Multi-config generator: generate per-config INI files
        set(_ekiosk_configs Debug Release RelWithDebInfo MinSizeRel)
        foreach(_cfg ${_ekiosk_configs})
            string(TOUPPER "${_cfg}" _CFG_UPPER)
            # Try config-specific runtime directory first
            set(_ini_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_UPPER}}")
            if(NOT _ini_output_dir)
                # Fall back to base runtime directory with config subdirectory
                if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
                    set(_ini_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_cfg}")
                else()
                    set(_ini_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${_cfg}")
                endif()
            endif()
            set(WORKING_DIRECTORY "${_ini_output_dir}")
            set(LOG_PATH "${_ini_output_dir}/logs")
            ek_generate_ini_template(
                ${NAME}
                "${TEMPLATE}"
                "${_ini_output_dir}"
                WORKING_DIRECTORY "${WORKING_DIRECTORY}"
                LOG_PATH "${LOG_PATH}"
            )
        endforeach()
    else()
        # Single-config generator: generate one INI file to bin folder
        set(_ini_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        if(NOT _ini_output_dir)
            set(_ini_output_dir "${CMAKE_BINARY_DIR}/bin")
        endif()
        set(WORKING_DIRECTORY "${_ini_output_dir}")
        set(LOG_PATH "${_ini_output_dir}/logs")
        ek_generate_ini_template(
            ${NAME}
            "${TEMPLATE}"
            "${_ini_output_dir}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}"
            LOG_PATH "${LOG_PATH}"
        )
    endif()
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
# Automatically selects platform-specific templates using directory structure:
#   runtimes/common/data/{name}/
#     ├── {name}.ini.in           (generic template)
#     ├── {name}.win.in          (Windows-specific)
#     ├── {name}.mac.in          (macOS-specific)
#     └── {name}.linux.in         (Linux-specific)
#
# Falls back to generic template if platform-specific version doesn't exist

function(ek_generate_ini_template NAME TEMPLATE OUTPUT_DIR)
    # Extract directory and base name from template path
    get_filename_component(_template_dir "${TEMPLATE}" DIRECTORY)
    get_filename_component(_template_name "${TEMPLATE}" NAME_WE) # Remove .in extension
    get_filename_component(_template_base "${_template_name}" NAME_WE) # Remove .ini extension

    # Determine platform-specific template path
    # Look in same directory as generic template (e.g., updater/updater.ini.mac.in)
    if(WIN32)
        set(_platform_template "${_template_dir}/${_template_base}.ini.win.in")
    elseif(APPLE)
        set(_platform_template "${_template_dir}/${_template_base}.ini.mac.in")
    elseif(UNIX)
        set(_platform_template "${_template_dir}/${_template_base}.ini.linux.in")
    else()
        set(_platform_template "")
    endif()

    # Start with the generic template content
    set(_merged_content "")
    if(EXISTS "${TEMPLATE}")
        file(READ "${TEMPLATE}" _generic_content)
        set(_merged_content "${_generic_content}")
    endif()

    # If platform-specific template exists, merge it with generic content
    if(EXISTS "${_platform_template}")
        file(READ "${_platform_template}" _platform_content)
        # Simple merge: append platform-specific content (it will override sections)
        set(_merged_content "${_merged_content}\n\n${_platform_content}")
        message(STATUS "Merged platform-specific template: ${_platform_template}")
    else()
        message(STATUS "Using generic template only: ${TEMPLATE}")
    endif()

    # Process user-provided variables for substitution
    set(_i 0)
    list(LENGTH ARGN _len)
    while(_i LESS _len)
        list(GET ARGN ${_i} _var)
        math(EXPR _j "${_i} + 1")
        list(GET ARGN ${_j} _val)
        string(REPLACE "@${_var}@" "${_val}" _merged_content "${_merged_content}")
        math(EXPR _i "${_i} + 2")
    endwhile()

    # Write the merged content to output file
    set(_out_ini "${OUTPUT_DIR}/${NAME}.ini")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    file(WRITE "${_out_ini}" "${_merged_content}")
    message(STATUS "Generated ini: ${_out_ini}")
endfunction()
