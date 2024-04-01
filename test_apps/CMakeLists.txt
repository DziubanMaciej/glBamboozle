include("../third_party/glad.cmake")
include("../third_party/gl_lib.cmake")

function(add_app NAME SOURCE_FILE)
    set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_app.h
        ${CMAKE_CURRENT_SOURCE_DIR}/render.h
    )
    source_group("" FILES ${SOURCES})
    add_executable(${NAME} WIN32 ${SOURCES})
    add_dependencies(${NAME} glBamboozle)
    set_target_properties(${NAME} PROPERTIES FOLDER test_apps)
    target_link_libraries(${NAME} PRIVATE Glad OpenGL::GL)
endfunction()

add_app(Opengl3 opengl3.cpp)
add_app(SeparateWindowThread separate_window_thread.cpp)
