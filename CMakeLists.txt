cmake_minimum_required(VERSION 3.16)
project(amber_cpp_sdk)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp amber_sdk.cpp amber_models.h)
include_directories(json-3.9.1/single_include)
include_directories(.)
link_libraries(curl)
add_executable(amber_cpp_sdk ${SOURCE_FILES})
