#
# Add a target for generating docs for the project using `doxygen` (i.e: cmake --build build --target docs)
#

function(add_docs_target)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
    find_package(Doxygen REQUIRED dot)
    doxygen_add_docs(docs ${PROJECT_SOURCE_DIR})
    message(STATUS "Doxygen has been setup and docs target is now available.")
endfunction()