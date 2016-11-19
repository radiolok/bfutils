cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0015 NEW)

project(bfpp)

set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set(COMMON_LIB ${PROJECT_SOURCE_DIR}/../common)

include_directories(${COMMON_LIB})

add_library(cmd ${COMMON_LIB}/cmd.cpp)

add_library(bfutils ${COMMON_LIB}/bfutils.cpp)

add_library(image ${COMMON_LIB}/Image.cpp ${COMMON_LIB}/bfutils.cpp ${COMMON_LIB}/cmd.cpp) 

add_library(bf Bf.cpp)

add_executable(bfpp bfpp.cpp)

target_link_libraries(bfpp bf cmd  image)

