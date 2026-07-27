cmake_minimum_required(VERSION 3.14)
project(chat)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE LIB_SOURCES "src/core/*.cpp" "src/server/*.cpp")
add_library(chat_lib ${LIB_SOURCES})
target_include_directories(chat_lib PUBLIC include)

add_executable(chat src/main.cpp)
target_link_libraries(chat PRIVATE chat_lib)

enable_testing()
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
add_executable(chat_test ${TEST_SOURCES})
target_link_libraries(chat_test PRIVATE chat_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(chat_test)

