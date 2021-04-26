cmake_minimum_required(VERSION 3.9)
project(ambersdk)
set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(include)

set(CMAKE_BUILD_TYPE Debug)

add_library(
        ambersdk
        SHARED
        amber_sdk.cpp
)

target_link_libraries(
        ambersdk
        PUBLIC
        curl
)

add_executable(
    connect-example
    examples/connect_example.cpp
)
target_link_libraries(connect-example ambersdk curl)

add_executable(
        full-example
        examples/full_example.cpp
)
target_link_libraries(full-example ambersdk curl)

add_executable(
        stream-advanced
        examples/stream_advanced.cpp
)
target_link_libraries(stream-advanced ambersdk curl)
