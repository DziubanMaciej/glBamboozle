cmake_minimum_required(VERSION 3.16.0)
project(glBamboozle)

set(GLBAMBOOZLE_BUILD_TEST_APPS OFF CACHE BOOL "If enabled, a set of test apps useful for development will be built.")
set(GLBAMBOOZLE_ENABLE_LOGS OFF CACHE BOOL "If enabled, each entry point call in opengl32.dll will be logged. Note this does not include extensions.")
set(GLBAMBOOZLE_ENABLE_LOGS_END OFF CACHE BOOL "If enabled, end of each entry point call in opengl32.dll will be logged. Only meaningful with GLBAMBOOZLE_ENABLE_LOGS")
set(GLBAMBOOZLE_LOG_TO_CONSOLE OFF CACHE BOOL "If enabled, all logs will be directed to console. Otherwise, it will be directed to the debugger.")

include(CMakeGlobalSetup.cmake)
setup_binary_locations()
setup_solution_folders()
setup_multicore_compilation()
setup_static_crt()

include(CMakeUtils.cmake)
include(third_party/gl_headers.cmake)

add_subdirectory(src)
if(GLBAMBOOZLE_BUILD_TEST_APPS)
    add_subdirectory(test_apps)
endif()
