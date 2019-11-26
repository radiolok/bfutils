cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0015 NEW)
project (bfutils)


set(CMAKE_BUILD_TYPE Debug)
IF (WIN32)
    set(CMAKE_CXX_FLAGS "/O2")        ## Optimize
ELSE()
    set(CMAKE_CXX_FLAGS "-O2")        ## Optimize

ENDIF()

set (CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/common)


include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(dpcrun)
add_subdirectory(bfpp)
add_subdirectory(bfrun)
add_subdirectory(bfloader)
