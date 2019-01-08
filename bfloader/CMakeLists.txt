cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0015 NEW)

project(bfloader)

set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set(COMMON_LIB ${PROJECT_SOURCE_DIR}/../common)

include_directories(${COMMON_LIB})

set (SOURCES
    bfloader.cpp
    rs232.c 
    rs232.h
)

IF (WIN32)
    set (SOURCES ${SOURCES} 
    ${COMMON_LIB}/getopt.cpp
    ${COMMON_LIB}/getopt.h
    )
ENDIF()

add_executable(bfloader 
    ${SOURCES}
)

