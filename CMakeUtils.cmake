function(target_common_setup TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    if (MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
    endif ()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endfunction()

macro(add_subdirectories)
    if (UNIX)
        set(FORBIDDEN_DIR_NAMES "windows")
    elseif (WIN32)
        set(FORBIDDEN_DIR_NAMES "linux")
    else ()
        message(FATAL_ERROR "Unsupported OS")
    endif ()

    file(GLOB SUB_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach (SUB_DIR ${SUB_DIRS})
        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/CMakeLists.txt)
            continue()
        endif ()

        list(FIND FORBIDDEN_DIR_NAMES ${SUB_DIR} INDEX)
        if (NOT INDEX EQUAL -1)
            continue()
        endif ()

        add_subdirectory(${SUB_DIR})
    endforeach ()
endmacro()
