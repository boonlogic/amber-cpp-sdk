cmake_minimum_required(VERSION 3.9)
project(ambersdk)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb -Wall -Wextra -Wpedantic -Werror")

set(CMAKE_CXX_FLAGS)

set(CMAKE_CTEST_ARGUMENTS --output-on-failure --gtest_break_on_failure)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(include /usr/local/Cellar/curl/7.78.0/include)

## Google test framework ##
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)

FetchContent_MakeAvailable(googletest)

###########

find_package(ZLIB)

add_library(
        ambersdk
        SHARED
        amber_sdk.cpp
)

target_link_libraries(
        ambersdk
        PUBLIC
        curl
	ZLIB::ZLIB
)

add_executable(
    connect-example
    examples/connect_example.cpp
)
target_link_libraries(connect-example ambersdk curl ZLIB::ZLIB)

add_executable(
        full-example
        examples/full_example.cpp
)
target_link_libraries(full-example ambersdk curl ZLIB::ZLIB)

add_executable(
        stream-advanced
        examples/stream_advanced.cpp
)
target_link_libraries(stream-advanced ambersdk curl ZLIB::ZLIB)

## GTEST ##
# ctest doesn't play nicely with gtest SetUpTestSuite, disable built-in testing targets
# enable_testing()

add_executable(
  test_client
  test/test_init.cpp
  test/test_authenticate.cpp
  test/test_endpoints.cpp
  test/secrets.cpp
)
target_link_libraries(test_client gtest_main ambersdk)

include(GoogleTest)
gtest_discover_tests(test_client)
###########

#set (Coverage_REPORT ${PROJECT_BINARY_DIR}/coverage.info)
#set (Coverage_DIR    ${PROJECT_BINARY_DIR}/coverage)
#add_custom_command (
#    OUTPUT  ${Coverage_REPORT}
#    COMMAND lcov -q -c -f -b . -d ${PROJECT_BINARY_DIR}/test -o ${Coverage_REPORT}
#    COMMAND lcov -e ${Coverage_REPORT} '${PROJECT_SOURCE_DIR}/src/*' -o ${Coverage_REPORT}
#    COMMAND genhtml ${Coverage_REPORT} --legend --demangle-cpp -f -q -o ${Coverage_DIR}
#    DEPENDS logic_test
#)
#add_custom_target (coverage DEPENDS ${Coverage_REPORT})
