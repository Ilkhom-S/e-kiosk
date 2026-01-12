# EKTesting.cmake - CTest integration and helpers

include(CTest)

function(ek_enable_testing)
    enable_testing()
endfunction()

function(ek_add_test TEST_NAME)
    set(options GUI NO_QT)
    set(oneValueArgs TIMEOUT WORKING_DIRECTORY)
    set(multiValueArgs SOURCES QT_MODULES DEPENDS LIBRARIES INCLUDE_DIRS LABELS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${TEST_NAME} ${ARG_SOURCES})

    # Ensure Qt auto-generation (moc/uic/rcc) runs for test targets
    set_target_properties(${TEST_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    if(NOT ARG_NO_QT)
        find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
        target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Test)
        if(ARG_QT_MODULES)
            foreach(qt_module ${ARG_QT_MODULES})
                target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${qt_module})
            endforeach()
        else()
            target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
        endif()
        if(ARG_GUI)
            target_link_libraries(${TEST_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets)
        endif()
    endif()
    if(ARG_DEPENDS)
        target_link_libraries(${TEST_NAME} PRIVATE ${ARG_DEPENDS})
    endif()
    if(ARG_LIBRARIES)
        target_link_libraries(${TEST_NAME} PRIVATE ${ARG_LIBRARIES})
    endif()
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${TEST_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    # Use CMake to run the test with Qt runtime directory on PATH so
    # the Qt DLLs are found when tests execute on Windows.
    if(NOT ARG_NO_QT)
        add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_COMMAND} -E env "PATH=$<TARGET_FILE_DIR:Qt${QT_VERSION_MAJOR}::Core>;$ENV{PATH}" $<TARGET_FILE:${TEST_NAME}>)
    else()
        add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
    endif()
endfunction()
