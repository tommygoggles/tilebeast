cmake_minimum_required(VERSION 2.8)

project(tilebeast)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB SOURCES "*.cpp")

add_executable(tilebeast ${SOURCES})
