cmake_minimum_required(VERSION 3.5)
project(prsl VERSION 0.0.1)

include(cmake/ClangFormat.cmake)
include(cmake/Doxygen.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AST_SOURCES
    prsl/AST/ASTVisitor.hpp 
    prsl/AST/NodeTypes.cpp prsl/AST/NodeTypes.hpp
    prsl/AST/TreeWalkerVisitor.hpp
)

set(COMPILER_SOURCES
    prsl/Compiler/Codegen/Codegen.cpp prsl/Compiler/Codegen/Codegen.hpp
    prsl/Compiler/Common/Environment.hpp prsl/Compiler/Common/FunctionsManager.hpp
    prsl/Compiler/Interpreter/Interpreter.cpp prsl/Compiler/Interpreter/Interpreter.hpp
    prsl/Compiler/Interpreter/Objects.cpp prsl/Compiler/Interpreter/Objects.hpp
    prsl/Compiler/Compiler.cpp prsl/Compiler/Compiler.hpp
    prsl/Compiler/CompilerFlags.cpp prsl/Compiler/CompilerFlags.hpp
    prsl/Compiler/Executor.hpp
)

set(DEBUG_SOURCES
    prsl/Debug/ErrorReporter.cpp prsl/Debug/ErrorReporter.hpp
    prsl/Debug/RuntimeError.cpp prsl/Debug/RuntimeError.hpp
)

set(PARSER_SOURCES
    prsl/Parser/Parser.cpp prsl/Parser/Parser.hpp
    prsl/Parser/Scanner.cpp prsl/Parser/Scanner.hpp
    prsl/Parser/Token.hpp
)

SET(SEMANTICS_SOURCES
    prsl/Semantics/Semantics.cpp prsl/Semantics/Semantics.hpp
)

set(UTILS_SOURCES
    prsl/Utils/Utils.hpp
)

set(ALL_SOURCES
    ${AST_SOURCES}
    ${COMPILER_SOURCES}
    ${DEBUG_SOURCES}
    ${PARSER_SOURCES}
    ${SEMANTICS_SOURCES}
    ${UTILS_SOURCES}
)

if (NOT DISABLE_BUILDING_EXECUTABLE)
    add_executable(${PROJECT_NAME} prsl/main.cpp ${ALL_SOURCES})

    # -- LLVM
    # The magic is for CMake to find the newest version of LLVM, not the oldest
    SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
    SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    add_definitions(${LLVM_DEFINITIONS})
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    llvm_map_components_to_libnames(llvm_libs core executionengine irreader nativecodegen support passes ${LLVM_TARGETS_TO_BUILD})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${llvm_libs})

    # -- Boost
    # Please, do not remove the magic trick below
    # It will broke the CI/CD
    find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

    target_include_directories(${PROJECT_NAME} PRIVATE .)
    if (TARGET Boost::boost)
        target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)
        target_link_libraries(${PROJECT_NAME} PUBLIC Boost::program_options)
    else()
        target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    endif()

    configure_file(prsl/config.hpp.in config.hpp @ONLY)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    install(TARGETS ${PROJECT_NAME})
endif()

add_custom_target(check-all
    COMMAND lit -v -D edir=${CMAKE_CURRENT_BINARY_DIR} ../test
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_clang_format_target()
add_docs_target()