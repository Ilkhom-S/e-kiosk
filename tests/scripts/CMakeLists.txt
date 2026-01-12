# Tests for build-time scripts (Python)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(NAME include_reflow_pytest
    COMMAND ${Python3_EXECUTABLE} -m unittest test_include_reflow.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(include_reflow_pytest PROPERTIES TIMEOUT 10)
