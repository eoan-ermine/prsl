#
# Add a target for static analysing the project using `cppcheck` (i.e: cmake --build build --target cppcheck)
#

function(add_cppcheck_target)
    if(NOT ${PROJECT_NAME}_CPPCHECK_BINARY)
			find_program(${PROJECT_NAME}_CPPCHECK_BINARY cppcheck)
    endif()

    if(${PROJECT_NAME}_CPPCHECK_BINARY)
            add_custom_target(cppcheck
                    COMMAND ${${PROJECT_NAME}_CPPCHECK_BINARY}
                    prsl/
                    --suppress=missingInclude
                    --suppress=missingIncludeSystem
                    --enable=all
                    --force
                    --verbose
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
			message(STATUS "Static analyse the project using the `cppcheck` target (i.e: cmake --build build --target cppcheck).")
    endif()
endfunction()