# EKApplication.cmake - Helper function for creating applications

function(ek_add_application TARGET_NAME)
    set(options CONSOLE)
    set(oneValueArgs "")
    set(multiValueArgs SOURCES QT_MODULES DEPENDS INCLUDE_DIRS COMPILE_DEFINITIONS LIBRARIES RESOURCES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_CONSOLE)
        add_executable(${TARGET_NAME} ${ARG_SOURCES} ${ARG_RESOURCES})
    else()
        if(WIN32)
            add_executable(${TARGET_NAME} WIN32 ${ARG_SOURCES} ${ARG_RESOURCES})
        else()
            add_executable(${TARGET_NAME} ${ARG_SOURCES} ${ARG_RESOURCES})
        endif()
    endif()
    if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND NOT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" STREQUAL "")
        set(_ek_runtime_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(_ek_runtime_dir "")
    endif()
    if(NOT "${_ek_runtime_dir}" STREQUAL "")
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${_ek_runtime_dir}
            OUTPUT_NAME ${TARGET_NAME}
            DEBUG_POSTFIX "d"
        )
    else()
        set_target_properties(${TARGET_NAME} PROPERTIES
            OUTPUT_NAME ${TARGET_NAME}
            DEBUG_POSTFIX "d"
        )
    endif()
    set(default_qt_modules Core)
    foreach(qt_module ${default_qt_modules})
        target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${qt_module})
    endforeach()
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
    if(WIN32)
        target_compile_definitions(${TARGET_NAME} PRIVATE _USING_V110_SDK71_)
    endif()
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin
    )
endfunction()
