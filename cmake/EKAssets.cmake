# EKAssets.cmake - Helper for copying runtime assets at build time
#
# This module provides macros for copying application assets (images, sounds, styles, etc.)
# from source directories to runtime output directories during the build phase.
#
# Usage:
#   ek_copy_assets(TARGET_NAME SOURCE_DIR OUTPUT_DIR [DIRS dir1 dir2 ...])
#
# Example:
#   ek_copy_assets(ekiosk
#       SOURCE_DIR "${CMAKE_SOURCE_DIR}/runtimes/common/assets"
#       OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
#       DIRS images sounds styles
#   )
#
# This creates dependencies such that when you build the app target,
# all specified directories are copied from SOURCE_DIR to OUTPUT_DIR.
# Files are only copied if the source files have changed (via custom_command dependency tracking).

# Function: ek_copy_assets
# Copies asset directories from source to output at build time using CMake custom commands
#
# Parameters:
#   TARGET_NAME: The app target name (used for dependency linking)
#   SOURCE_DIR: Source directory containing assets
#   OUTPUT_DIR: Output directory where assets should be copied
#   DIRS: (Optional) Specific directories to copy. If empty, copies entire SOURCE_DIR
#
# Returns:
#   ${TARGET_NAME}_ASSET_TARGET: Name of the custom target created
#
function(ek_copy_assets TARGET_NAME)
    set(options)
    set(oneValueArgs SOURCE_DIR OUTPUT_DIR)
    set(multiValueArgs DIRS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_SOURCE_DIR)
        message(FATAL_ERROR "ek_copy_assets: SOURCE_DIR is required")
    endif()

    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "ek_copy_assets: OUTPUT_DIR is required")
    endif()

    if(NOT EXISTS "${ARG_SOURCE_DIR}")
        message(WARNING "ek_copy_assets: SOURCE_DIR does not exist: ${ARG_SOURCE_DIR}")
        return()
    endif()

    set(_asset_target "${TARGET_NAME}_assets")
    set(_input_files "")

    if(ARG_DIRS)
        # Copy specific directories
        foreach(_dir ${ARG_DIRS})
            set(_src_dir "${ARG_SOURCE_DIR}/${_dir}")

            if(EXISTS "${_src_dir}")
                # Collect all files in this directory for dependency tracking
                file(GLOB_RECURSE _dir_files "${_src_dir}/*")
                list(APPEND _input_files ${_dir_files})
                message(STATUS "Will copy asset directory at build time: ${_dir} → ${ARG_OUTPUT_DIR}")
            else()
                message(WARNING "ek_copy_assets: Directory not found: ${_src_dir}")
            endif()
        endforeach()
    else()
        # Copy entire SOURCE_DIR
        file(GLOB_RECURSE _input_files "${ARG_SOURCE_DIR}/*")
        message(STATUS "Will copy all assets at build time: ${ARG_SOURCE_DIR} → ${ARG_OUTPUT_DIR}")
    endif()

    if(NOT _input_files AND ARG_DIRS)
        message(WARNING "ek_copy_assets: No assets found to copy for ${TARGET_NAME}")
        return()
    endif()

    # Build list of copy commands
    set(_copy_commands "")

    file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

    if(ARG_DIRS)
        foreach(_dir ${ARG_DIRS})
            set(_src_dir "${ARG_SOURCE_DIR}/${_dir}")
            if(EXISTS "${_src_dir}")
                list(APPEND _copy_commands
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${_src_dir}" "${ARG_OUTPUT_DIR}/${_dir}"
                )
            endif()
        endforeach()
    else()
        list(APPEND _copy_commands
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ARG_SOURCE_DIR}" "${ARG_OUTPUT_DIR}"
        )
    endif()

    # Create custom command that copies assets at build time
    add_custom_command(
        OUTPUT "${ARG_OUTPUT_DIR}/.assets_copied_${TARGET_NAME}"
        ${_copy_commands}
        COMMAND ${CMAKE_COMMAND} -E touch "${ARG_OUTPUT_DIR}/.assets_copied_${TARGET_NAME}"
        DEPENDS ${_input_files}
        COMMENT "Copying assets for ${TARGET_NAME}"
    )

    # Create custom target that depends on the custom command
    add_custom_target(${_asset_target}
        DEPENDS "${ARG_OUTPUT_DIR}/.assets_copied_${TARGET_NAME}"
        COMMENT "Asset copy target for ${TARGET_NAME}"
    )

    # Return the target name to parent scope for easy dependency linking
    set(${TARGET_NAME}_ASSET_TARGET "${_asset_target}" PARENT_SCOPE)

    message(STATUS "Created asset target: ${_asset_target}")
endfunction()


# Macro: ek_link_app_assets(TARGET_NAME)
# Links app target to its assets target (if it exists)
# Ensures assets are copied when the app is built
#
# Usage:
#   ek_link_app_assets(ekiosk)
#
macro(ek_link_app_assets TARGET_NAME)
    if(TARGET ${TARGET_NAME}_assets)
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_assets)
        message(STATUS "Linked ${TARGET_NAME} to ${TARGET_NAME}_assets")
    endif()
endmacro()
