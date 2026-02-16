# Macro: ek_generate_ini_for_all_configs(NAME TEMPLATE)
# Generates ini files for the app in the correct output directory alongside the executable.
# For multi-config generators (Visual Studio), generates per-config INI files.
# For single-config generators (Unix Makefiles, Ninja), generates one INI to bin folder.
# Creates a custom target ${NAME}_ini that can be used as a dependency.
# Returns list of generated files in ${NAME}_INI_FILES variable in parent scope.
macro(ek_generate_ini_for_all_configs NAME TEMPLATE)
    # Check if using multi-config generator (Visual Studio, Xcode)
    get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    set(_all_ini_outputs "")

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
            list(APPEND _all_ini_outputs "${${NAME}_INI_OUTPUT}")
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
        list(APPEND _all_ini_outputs "${${NAME}_INI_OUTPUT}")
    endif()

    # Create a custom target that depends on all generated .ini files
    # This target will only be built when something depends on it
    add_custom_target(${NAME}_ini
        DEPENDS ${_all_ini_outputs}
        COMMENT "Ensuring ${NAME}.ini files are generated"
    )

    # Return the list of all generated .ini files to parent scope
    set(${NAME}_INI_FILES ${_all_ini_outputs} PARENT_SCOPE)

    message(STATUS "Created ini generation target: ${NAME}_ini (builds only when ${NAME} is built)")
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
#
# Merge behavior: section-aware merge
#   - Same section in base + platform: merge keys (platform keys override ONLY if key exists in base)
#   - Section only in platform: add as new section
#   - Section only in base: keep as is
#
# Returns the output file path in ${NAME}_INI_OUTPUT variable in parent scope

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

    # Determine merge strategy
    set(_merge_script "")
    if(EXISTS "${_platform_template}")
        message(STATUS "Will merge platform-specific template at build time: ${_platform_template}")
        set(_template_deps "${TEMPLATE}" "${_platform_template}")

        # Create Python script for proper INI merging
        set(_merge_script "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_merge.py")
        file(WRITE "${_merge_script}" "
import sys
import configparser

def merge_ini(base_path, platform_path, output_path):
    # Read both INI files
    config = configparser.ConfigParser()
    config.read(base_path, encoding='utf-8')

    platform = configparser.ConfigParser()
    platform.read(platform_path, encoding='utf-8')

    # Section-aware merge
    for section in platform.sections():
        if not config.has_section(section):
            # Section only in platform - add as new
            config.add_section(section)
        # Merge keys: platform adds new keys, but doesn't override existing base keys
        for key in platform.options(section):
            if not config.has_option(section, key):
                # Key doesn't exist in base - add from platform
                config.set(section, key, platform.get(section, key))
            # If key exists in base, keep base value (don't override)

    # Write merged result
    with open(output_path, 'w', encoding='utf-8') as f:
        config.write(f)

if __name__ == '__main__':
    merge_ini(sys.argv[1], sys.argv[2], sys.argv[3])
")
        set(_merge_type "MERGE")
    else()
        message(STATUS "Will use generic template at build time: ${TEMPLATE}")
        set(_template_deps "${TEMPLATE}")
        set(_merge_type "COPY")
    endif()

    # Process user-provided variables for substitution
    # First, read the content to substitute variables
    if(EXISTS "${TEMPLATE}")
        file(READ "${TEMPLATE}" _template_content)
    else()
        set(_template_content "")
    endif()

    set(_i 0)
    list(LENGTH ARGN _len)
    while(_i LESS _len)
        list(GET ARGN ${_i} _var)
        math(EXPR _j "${_i} + 1")
        list(GET ARGN ${_j} _val)
        string(REPLACE "@${_var}@" "${_val}" _template_content "${_template_content}")
        math(EXPR _i "${_i} + 2")
    endwhile()

    # Output file path
    set(_out_ini "${OUTPUT_DIR}/${NAME}.ini")

    if(_merge_type STREQUAL "MERGE")
        # Write processed base template (with substitutions)
        set(_base_processed "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_base.ini")
        file(WRITE "${_base_processed}" "${_template_content}")

        # Write processed platform template (with substitutions applied to copy)
        file(READ "${_platform_template}" _platform_content)
        set(_i 0)
        list(LENGTH ARGN _len)
        while(_i LESS _len)
            list(GET ARGN ${_i} _var)
            math(EXPR _j "${_i} + 1")
            list(GET ARGN ${_j} _val)
            string(REPLACE "@${_var}@" "${_val}" _platform_content "${_platform_content}")
            math(EXPR _i "${_i} + 2")
        endwhile()
        set(_platform_processed "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_platform.ini")
        file(WRITE "${_platform_processed}" "${_platform_content}")

        # Create a CMake script that runs the merge
        set(_script_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_gen.cmake")
        file(WRITE "${_script_file}" "
# Auto-generated script to merge and create ${NAME}.ini at build time
execute_process(
    COMMAND \${CMAKE_COMMAND} -E env python3 \"${_merge_script}\" \"${_base_processed}\" \"${_platform_processed}\" \"${_out_ini}\"
    RESULT_VARIABLE _result
)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR \"Failed to merge INI files\")
endif()
file(MAKE_DIRECTORY \"${OUTPUT_DIR}\")
message(STATUS \"Generated merged .ini at build time: ${_out_ini}\")
")

        # Add custom command to generate .ini file at build time (only when needed)
        add_custom_command(
            OUTPUT "${_out_ini}"
            COMMAND ${CMAKE_COMMAND} -P "${_script_file}"
            DEPENDS ${_template_deps} "${_merge_script}"
            COMMENT "Merging ${NAME}.ini from base + platform config"
            VERBATIM
        )
    else()
        # Simple copy mode - just write processed content
        set(_script_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_gen.cmake")

        # Escape the content properly for writing to script
        string(REPLACE "\\" "\\\\" _escaped_content "${_template_content}")
        string(REPLACE "\"" "\\\"" _escaped_content "${_escaped_content}")

        file(WRITE "${_script_file}" "
# Auto-generated script to create ${NAME}.ini at build time
file(MAKE_DIRECTORY \"${OUTPUT_DIR}\")
file(WRITE \"${_out_ini}\" \"${_escaped_content}\")
message(STATUS \"Generated .ini at build time: ${_out_ini}\")
")

        # Add custom command to generate .ini file at build time (only when needed)
        add_custom_command(
            OUTPUT "${_out_ini}"
            COMMAND ${CMAKE_COMMAND} -P "${_script_file}"
            DEPENDS ${_template_deps}
            COMMENT "Generating ${NAME}.ini"
            VERBATIM
        )
    endif()

    # Return the output file path to parent scope
    set(${NAME}_INI_OUTPUT "${_out_ini}" PARENT_SCOPE)

    message(STATUS "Configured .ini generation (will run at build time): ${_out_ini}")
endfunction()
