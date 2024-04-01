macro(setup_binary_locations)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endmacro()

function(setup_solution_folders)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endfunction()

macro (setup_multicore_compilation)
    # This is only for Visual Studio. On Linux we should pass -j$(nproc) into make
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
endmacro()

macro(setup_static_crt)
    if (MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endmacro()
