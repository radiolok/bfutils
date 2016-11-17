cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0015 NEW)

project(bfpp)

set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set(COMMON_LIB ${PROJECT_SOURCE_DIR}/../common)

include_directories(${COMMON_LIB})

add_library(Console ${COMMON_LIB}/Console.cpp)

add_executable(bfrun bfrun.cpp)

target_link_libraries(bfrun Console)

