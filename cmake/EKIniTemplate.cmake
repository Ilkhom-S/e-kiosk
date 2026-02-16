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
    get_filename_component(_template_dir "${TEMPLATE}" DIRECTORY)
    get_filename_component(_template_name "${TEMPLATE}" NAME_WE)
    get_filename_component(_template_base "${_template_name}" NAME_WE)

    # Platform-specific template path
    if(WIN32)
        set(_platform_template "${_template_dir}/${_template_base}.ini.win.in")
    elseif(APPLE)
        set(_platform_template "${_template_dir}/${_template_base}.ini.mac.in")
    elseif(UNIX)
        set(_platform_template "${_template_dir}/${_template_base}.ini.linux.in")
    else()
        set(_platform_template "")
    endif()

    # Substitute variables in base template
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
    set(_base_processed "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ini_base.ini")
    file(WRITE "${_base_processed}" "${_template_content}")

    # Substitute variables in platform template if exists
    set(_platform_processed "")
    if(EXISTS "${_platform_template}")
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
    endif()

    set(_out_ini "${OUTPUT_DIR}/${NAME}.ini")
    set(_merge_script "${CMAKE_SOURCE_DIR}/scripts/merge_ini.py")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    # Build command line
    if(EXISTS "${_platform_processed}")
        set(_merge_cmd python3 "${_merge_script}" --base "${_base_processed}" --platform "${_platform_processed}" --output "${_out_ini}")
        set(_template_deps "${_base_processed}" "${_platform_processed}" "${_merge_script}")
    else()
        set(_merge_cmd python3 "${_merge_script}" --base "${_base_processed}" --output "${_out_ini}")
        set(_template_deps "${_base_processed}" "${_merge_script}")
    endif()

    add_custom_command(
        OUTPUT "${_out_ini}"
        COMMAND ${_merge_cmd}
        DEPENDS ${_template_deps}
        COMMENT "Merging ${NAME}.ini from base + platform config using merge_ini.py"
        VERBATIM
    )

    set(${NAME}_INI_OUTPUT "${_out_ini}" PARENT_SCOPE)
    message(STATUS "Configured .ini generation (merge_ini.py): ${_out_ini}")
endfunction()

