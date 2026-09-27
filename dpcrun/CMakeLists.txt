cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0015 NEW)

project(dpcrun)

set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# The DekatronPC golden model as a library. The DekatronPC Verilator testbench
# builds the same dpcrun.cpp without EXEC (see rtl/run/run_tests.sh there).
add_library(dpcmodel STATIC dpcrun.cpp dpcrun.h)
target_include_directories(dpcmodel PUBLIC ${PROJECT_SOURCE_DIR})

# Standalone runner: main() is only compiled with EXEC.
add_executable(dpcrun dpcrun.cpp)
target_compile_definitions(dpcrun PRIVATE EXEC)

# Unit tests
add_executable(test_dpcrun test/test_dpcrun.cpp)
target_link_libraries(test_dpcrun dpcmodel)
target_compile_definitions(test_dpcrun PRIVATE
    BFUTILS_COMMON="${PROJECT_SOURCE_DIR}/../common")
add_test(NAME dpcrun_unit COMMAND test_dpcrun)
