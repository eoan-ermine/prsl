function(add_pvs_studio_target)
    include(FetchContent)
    FetchContent_Declare(
        PVS_CMakeModule
        GIT_REPOSITORY "https://github.com/viva64/pvs-studio-cmake-module.git"
        GIT_TAG        "master" 
    )
    FetchContent_MakeAvailable(PVS_CMakeModule)
    include("${pvs_cmakemodule_SOURCE_DIR}/PVS-Studio.cmake")
    pvs_studio_add_target(TARGET pvs-studio OUTPUT FORMAT json LOG prsl.err.json COMPILE_COMMANDS MODE GA OP)
    message(STATUS "Static analyse the project using the `pvs-studio` target (i.e: cmake --build build --target pvs-studio).")
endfunction()