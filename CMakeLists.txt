cmake_minimum_required(VERSION 4.3)
project(chat)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(chat ${SOURCES})
target_include_directories(chat PRIVATE include)


