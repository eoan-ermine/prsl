cmake_minimum_required(VERSION 3.5)
project(prsl)

set(CMAKE_CXX_STANDARD 23)

add_executable(
    prsl
    prsl/main.cpp
)
target_include_directories(prsl PRIVATE .)