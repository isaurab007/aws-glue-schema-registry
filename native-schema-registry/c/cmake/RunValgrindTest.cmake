# CMake script to run valgrind test as part of the build process
# This bypasses the regular test system and runs our standalone valgrind test

message(STATUS "=== Running GSR Valgrind Memory Analysis ===")

# Get the correct source directory path
get_filename_component(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
set(SRC_DIR ${PROJECT_ROOT_DIR}/src)
set(VALGRIND_TEST_SOURCE ${SRC_DIR}/valgrind_gsr_test.c)

message(STATUS "Source directory: ${SRC_DIR}")
message(STATUS "Valgrind test source: ${VALGRIND_TEST_SOURCE}")

# Check if the source file exists
if(NOT EXISTS ${VALGRIND_TEST_SOURCE})
    message(FATAL_ERROR "Valgrind test source file not found: ${VALGRIND_TEST_SOURCE}")
endif()

# Compile the standalone valgrind test
execute_process(
    COMMAND gcc -o valgrind_gsr_test ${VALGRIND_TEST_SOURCE} -g
    WORKING_DIRECTORY ${SRC_DIR}
    RESULT_VARIABLE COMPILE_RESULT
    OUTPUT_VARIABLE COMPILE_OUTPUT
    ERROR_VARIABLE COMPILE_ERROR
)

if(NOT COMPILE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile valgrind test: ${COMPILE_ERROR}")
endif()

message(STATUS "Valgrind test compiled successfully")
message(STATUS "Running valgrind memory analysis on GSR serializer lifecycle...")

# Run valgrind on the test
execute_process(
    COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./valgrind_gsr_test
    WORKING_DIRECTORY ${SRC_DIR}
    RESULT_VARIABLE VALGRIND_RESULT
    OUTPUT_VARIABLE VALGRIND_OUTPUT
    ERROR_VARIABLE VALGRIND_ERROR
)

# Print valgrind output
message(STATUS "${VALGRIND_OUTPUT}")
if(VALGRIND_ERROR)
    message(STATUS "${VALGRIND_ERROR}")
endif()

if(NOT VALGRIND_RESULT EQUAL 0)
    message(FATAL_ERROR "Valgrind detected memory issues (exit code: ${VALGRIND_RESULT})")
endif()

message(STATUS "=== GSR Valgrind Analysis Complete - No Memory Issues Detected ===")
