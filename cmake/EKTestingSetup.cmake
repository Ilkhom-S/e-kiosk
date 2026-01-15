# EKTestingSetup.cmake - Testing configuration for EKiosk

# Enable CTest at top-level so tests added in subdirectories are registered
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)
ek_enable_testing()

# Optional: enable coverage build and reporting (Linux/Clang/GNU)
option(ENABLE_COVERAGE "Enable coverage build and reporting" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Coverage enabled: adding --coverage flags")
        set(COVERAGE_FLAGS "--coverage -O0 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
    else()
        message(WARNING "Coverage build requested but compiler does not support gcov/llvm-cov flags")
    endif()
endif()

# Coverage target: generate lcov info and HTML report
if(ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)
    if(NOT LCOV_EXEC)
        message(WARNING "lcov not found: coverage target will fail without lcov installed")
    endif()
    if(NOT GENHTML_EXEC)
        message(WARNING "genhtml not found: coverage target will fail without genhtml installed")
    endif()
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test -- -j
        COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${LCOV_EXEC} --remove ${CMAKE_BINARY_DIR}/coverage.info '/usr/*' --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${GENHTML_EXEC} ${CMAKE_BINARY_DIR}/coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage-report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report (requires lcov/genhtml)"
        VERBATIM
    )
endif()
