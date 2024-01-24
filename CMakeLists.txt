cmake_minimum_required(VERSION 3.5)
project(prsl)

set(CMAKE_CXX_STANDARD 23)

# -- LLVM

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

# -- Boost.ProgramOptions

find_package(Boost 1.74 COMPONENTS program_options REQUIRED)

add_executable(
    ${PROJECT_NAME}
    prsl/main.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} ${llvm_libs} Boost::program_options)