# EKTranslation.cmake - Qt translation helpers

option(EK_ENABLE_TRANSLATIONS "Enable building translations (Qt LinguistTools or lrelease required)" ON)

function(ek_add_translations TARGET_NAME)
    set(options INSTALL)
    set(oneValueArgs OUTPUT_DIR TS_DIR INSTALL_DIR FOLDER PREFIX)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EK_ENABLE_TRANSLATIONS)
        message(STATUS "Translation support explicitly disabled via EK_ENABLE_TRANSLATIONS=OFF")
        return()
    endif()

    # Set default output directory if not specified
    if(NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/locale)
    endif()

    # Set default PREFIX to TARGET_NAME if not specified
    # PREFIX is used for .qm output filename, allowing separation from CMake target name
    if(NOT ARG_PREFIX)
        set(ARG_PREFIX ${TARGET_NAME})
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

    # Group .ts files by language (suffix after last underscore)
    set(_ts_groups)
    foreach(_ts_file ${ARG_SOURCES})
        get_filename_component(_ts_name ${_ts_file} NAME_WE)
        # Extract language suffix (everything after the last underscore)
        string(REGEX MATCH "_([^_]+)$" _lang_match ${_ts_name})
        if(_lang_match)
            set(_language ${CMAKE_MATCH_1})
            list(APPEND _ts_groups ${_language})
        else()
            # If no language suffix, treat as 'en' or skip
            set(_language "en")
            list(APPEND _ts_groups ${_language})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _ts_groups)

    # For each language, collect all .ts files and create merged .qm
    foreach(_language ${_ts_groups})
        set(_ts_files_for_lang)
        set(_qm_file "${ARG_OUTPUT_DIR}/${ARG_PREFIX}_${_language}.qm")

        # Collect all .ts files for this language
        foreach(_ts_file ${ARG_SOURCES})
            get_filename_component(_ts_name ${_ts_file} NAME_WE)
            if(_ts_name MATCHES "_${_language}$" OR NOT _ts_name MATCHES "_")
                list(APPEND _ts_files_for_lang ${_ts_file})
            endif()
        endforeach()

        # Create custom command to merge all .ts files for this language into one .qm
        if(_use_qt_lrelease)
            add_custom_command(
                OUTPUT ${_qm_file}
                COMMAND Qt${QT_VERSION_MAJOR}::lrelease ${_ts_files_for_lang} -qm ${_qm_file}
                DEPENDS ${_ts_files_for_lang}
                COMMENT "Compiling merged translation ${ARG_PREFIX}_${_language}.qm"
                VERBATIM
            )
        else()
            add_custom_command(
                OUTPUT ${_qm_file}
                COMMAND ${_lrelease_exec} ${_ts_files_for_lang} -qm ${_qm_file}
                DEPENDS ${_ts_files_for_lang}
                COMMENT "Compiling merged translation ${ARG_PREFIX}_${_language}.qm (using external lrelease)"
                VERBATIM
            )
        endif()
        list(APPEND _qm_files ${_qm_file})
    endforeach()
    add_custom_target(${TARGET_NAME}_translations ALL DEPENDS ${_qm_files})

    # Copy compiled .qm files to CMAKE_BINARY_DIR/bin/locale for development/testing
    # This allows apps to find translations in the build directory without installation
    if(DEFINED CMAKE_BINARY_DIR)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/locale)
        add_custom_command(TARGET ${TARGET_NAME}_translations POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ARG_OUTPUT_DIR}
            ${CMAKE_BINARY_DIR}/bin/locale
            COMMENT "Copying translations to ${CMAKE_BINARY_DIR}/bin/locale"
        )
    endif()

    # Set folder for translation targets in IDE project outlines
    if(ARG_FOLDER)
        set(_ek_folder "${ARG_FOLDER}")
    else()
        # Default to translations folder for all translation targets
        set(_ek_folder "translations")
    endif()
    set_target_properties(${TARGET_NAME}_translations PROPERTIES FOLDER "${_ek_folder}")

    if(TARGET ${TARGET_NAME})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_translations)
    endif()

    # Install translations if requested
    if(ARG_INSTALL)
        # Allow overriding the translations install directory via EK_TRANSLATIONS_DIR environment variable
        # Final fallback is bin/locale relative to CMAKE_INSTALL_PREFIX
        if(DEFINED ENV{EK_TRANSLATIONS_DIR})
            set(_translations_install_dir $ENV{EK_TRANSLATIONS_DIR})
        elseif(ARG_INSTALL_DIR)
            set(_translations_install_dir ${ARG_INSTALL_DIR})
        else()
            set(_translations_install_dir bin/locale)
        endif()

        install(FILES ${_qm_files}
            DESTINATION ${_translations_install_dir}
            COMPONENT translations
        )
    endif()
endfunction()
