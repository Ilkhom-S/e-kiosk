# EKPlugin.cmake - Helper function for creating plugins (shared libraries)

function(ek_add_plugin TARGET_NAME)
    set(options "")
    set(oneValueArgs INSTALL_DIR FOLDER)
    set(multiValueArgs SOURCES QT_MODULES DEPENDS INCLUDE_DIRS COMPILE_DEFINITIONS LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})


    # Allow overriding the default plugin install directory via EK_PLUGIN_DIR environment variable
    if(NOT ARG_INSTALL_DIR)
        if(DEFINED ENV{EK_PLUGIN_DIR})
            set(ARG_INSTALL_DIR $ENV{EK_PLUGIN_DIR})
        else()
            set(ARG_INSTALL_DIR "plugins")
        endif()
    endif()

    # Set plugin build output directory from EK_PLUGIN_DIR if set, else use bin/plugins
    if(DEFINED ENV{EK_PLUGIN_DIR})
        set(_plugin_output_dir "$ENV{EK_PLUGIN_DIR}")
    else()
        set(_plugin_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")
    endif()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_plugin_output_dir}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_plugin_output_dir}")
    foreach(_cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${_cfg}" _CFG_UPPER)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG_UPPER} "${_plugin_output_dir}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_UPPER} "${_plugin_output_dir}")
    endforeach()

    add_library(${TARGET_NAME} SHARED ${ARG_SOURCES})
    # Ensure Qt auto-generation (moc/uic/rcc) runs for plugin targets that use Qt
    set_target_properties(${TARGET_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ARG_INSTALL_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ARG_INSTALL_DIR}
        OUTPUT_NAME ${TARGET_NAME}
        DEBUG_POSTFIX "d"
    )
    # Compute plugin folder (default plugins/<directory-name>)
    if(ARG_FOLDER)
        set(_ek_folder "${ARG_FOLDER}")
    else()
        get_filename_component(_ek_module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        set(_ek_folder "plugins/${_ek_module_name}")
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${_ek_folder}")
    if(ARG_QT_MODULES)
        foreach(qt_module ${ARG_QT_MODULES})
            target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${qt_module})
        endforeach()
    endif()
    if(ARG_DEPENDS)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDS})
    endif()
    if(ARG_LIBRARIES)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_LIBRARIES})
    endif()
    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include # ensure public headers are visible for automoc/IDE
        ${Boost_INCLUDE_DIRS} # Include boost headers for dependencies
    )
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    if(ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()

    # Sanitize link libraries: remove accidental literal 'Core' entries that
    # may end up as 'Core.lib' in generated MSBuild projects. This is a
    # pragmatic workaround for legacy or transitive entries. Prefer to fix
    # the root cause if possible (look for target_link_libraries(... Core)).
    get_target_property(_tmp_link_libs ${TARGET_NAME} LINK_LIBRARIES)
    if(_tmp_link_libs)
        set(_filtered_libs)
        set(_removed_core FALSE)
        foreach(_ll ${_tmp_link_libs})
            string(TOUPPER _ll_up "${_ll}")
            if(_ll_up MATCHES "(^|;)?CORE(\\.LIB)?(;|$)")
                message(STATUS "Removing accidental link '${_ll}' from target ${TARGET_NAME}")
                set(_removed_core TRUE)
            else()
                list(APPEND _filtered_libs ${_ll})
            endif()
        endforeach()
        if(_removed_core)
            # Override the LINK_LIBRARIES property with the filtered list
            set_target_properties(${TARGET_NAME} PROPERTIES LINK_LIBRARIES "${_filtered_libs}")
        endif()
    endif()

    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin/${ARG_INSTALL_DIR}
        LIBRARY DESTINATION lib/${ARG_INSTALL_DIR}
    )
endfunction()
