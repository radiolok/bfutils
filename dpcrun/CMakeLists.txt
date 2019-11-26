cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0015 NEW)

project(dpcrun)

set (CMAKE_BUILD_FILES_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set(COMMON_LIB ${PROJECT_SOURCE_DIR}/../common)

include_directories(${COMMON_LIB})

set (SOURCES 
        dpcrun.cpp 
        ${COMMON_LIB}/Command.cpp 
        ${COMMON_LIB}/Command.h 
        ${COMMON_LIB}/Console.cpp 
        ${COMMON_LIB}/Console.h 
        ${COMMON_LIB}/cmd.cpp
        ${COMMON_LIB}/cmd.h
        ${COMMON_LIB}/Image.cpp 
        ${COMMON_LIB}/Image.h 
        ${COMMON_LIB}/bfutils.cpp
        ${COMMON_LIB}/bfutils.h
)

IF (WIN32)
    set (SOURCES ${SOURCES} 
    ${COMMON_LIB}/getopt.cpp
    ${COMMON_LIB}/getopt.h
    )
ENDIF()

add_executable(dpcrun 
    ${SOURCES}
)
