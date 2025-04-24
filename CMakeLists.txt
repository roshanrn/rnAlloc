cmake_minimum_required(VERSION 3.10)
project(rnBench)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} rnBench.cpp ${SOURCES})

include_directories(src)
