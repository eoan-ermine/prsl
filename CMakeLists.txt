cmake_minimum_required(VERSION 3.5)
project(prsl)

set(CMAKE_CXX_STANDARD 23)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(
    prsl
    prsl/main.cpp
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_include_directories(prsl PRIVATE .)
target_link_libraries(prsl ${llvm_libs})