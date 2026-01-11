# EKPlugin.cmake - Helper function for creating plugins (shared libraries)

function(ek_add_plugin TARGET_NAME)
    set(options "")
    set(oneValueArgs INSTALL_DIR)
    set(multiValueArgs SOURCES QT_MODULES DEPENDS INCLUDE_DIRS COMPILE_DEFINITIONS LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_INSTALL_DIR)
        set(ARG_INSTALL_DIR "plugins")
    endif()
    add_library(${TARGET_NAME} SHARED ${ARG_SOURCES})
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ARG_INSTALL_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ARG_INSTALL_DIR}
        OUTPUT_NAME ${TARGET_NAME}
        DEBUG_POSTFIX "d"
    )
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
    )
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    if(ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin/${ARG_INSTALL_DIR}
        LIBRARY DESTINATION lib/${ARG_INSTALL_DIR}
    )
endfunction()
