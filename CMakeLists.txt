cmake_minimum_required(VERSION 3.5)
project(prsl)

include(cmake/ClangFormat.cmake)
include(cmake/Doxygen.cmake)

set(CMAKE_CXX_STANDARD 23)

set(sources
    prsl/main.cpp
    prsl/AST/NodeTypes.cpp
    prsl/Codegen/Codegen.cpp
    prsl/Debug/ErrorReporter.cpp prsl/Debug/RuntimeError.cpp
    prsl/Evaluator/Evaluator.cpp prsl/Evaluator/Objects.cpp
    prsl/Parser/Parser.cpp
    prsl/Scanner/Scanner.cpp
    prsl/Semantics/Semantics.cpp
)

set(headers
    prsl/AST/NodeTypes.hpp
    prsl/Codegen/Codegen.hpp
    prsl/Debug/ErrorReporter.hpp prsl/Debug/RuntimeError.hpp
    prsl/Evaluator/Environment.hpp prsl/Evaluator/Evaluator.hpp prsl/Evaluator/Objects.hpp
    prsl/Parser/Parser.hpp
    prsl/Scanner/Scanner.hpp
    prsl/Semantics/Semantics.hpp
    prsl/Types/Token.hpp
)


if (NOT FORMAT_CI)
    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    llvm_map_components_to_libnames(llvm_libs support core irreader)

    find_package(Boost 1.74 COMPONENTS program_options REQUIRED)

    add_executable(${PROJECT_NAME} ${sources})

    target_include_directories(${PROJECT_NAME} PRIVATE .)
    target_link_libraries(${PROJECT_NAME} ${llvm_libs} Boost::program_options)
    install(TARGETS ${PROJECT_NAME})
endif()

add_custom_target(check-all
    COMMAND lit -v -D edir=${CMAKE_CURRENT_BINARY_DIR} ../test
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_clang_format_target()
add_docs_target()