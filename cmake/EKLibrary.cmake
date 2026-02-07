# EKLibrary.cmake - Helper function for creating static libraries

function(ek_add_library TARGET_NAME)
    set(options "")
    set(oneValueArgs "FOLDER")
    set(multiValueArgs SOURCES QT_MODULES DEPENDS INCLUDE_DIRS COMPILE_DEFINITIONS LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${TARGET_NAME} STATIC ${ARG_SOURCES})
    # Ensure Qt auto-processing (moc/uic/rcc) runs for library targets that use Qt
    set_target_properties(${TARGET_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    # Guard setting archive output directory if it's defined
    set(_props)
    if(DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        list(APPEND _props ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    endif()
    list(APPEND _props OUTPUT_NAME ${TARGET_NAME} DEBUG_POSTFIX "d")
    set_target_properties(${TARGET_NAME} PROPERTIES ${_props})

    # Compute folder for module libraries (defaults to modules/<directory-name>)
    if(ARG_FOLDER)
        set(_ek_folder "${ARG_FOLDER}")
    else()
        get_filename_component(_ek_module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        set(_ek_folder "modules/${_ek_module_name}")
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${_ek_folder}")

    if(ARG_QT_MODULES)
        foreach(qt_module ${ARG_QT_MODULES})
            target_link_libraries(${TARGET_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::${qt_module})
        endforeach()
    else()
        target_link_libraries(${TARGET_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core)
    endif()

    # Add Qt include directories as SYSTEM to suppress warnings from Qt headers
    if(ARG_QT_MODULES)
        foreach(qt_module ${ARG_QT_MODULES})
            target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${Qt${QT_VERSION_MAJOR}${qt_module}_INCLUDE_DIRS})
        endforeach()
    else()
        target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS})
    endif()

    if(ARG_DEPENDS)
        target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_DEPENDS})
    endif()
    if(ARG_LIBRARIES)
        target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_LIBRARIES})
    endif()
    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PUBLIC ${ARG_INCLUDE_DIRS})
    endif()
    if(ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_COMPILE_DEFINITIONS})
    endif()
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
endfunction()
