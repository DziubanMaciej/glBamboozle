set(GL_HEADERS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/gl_headers/GL/glext.h
    ${CMAKE_CURRENT_LIST_DIR}/gl_headers/KHR/khrplatform.h
)
add_library(GlHeaders INTERFACE ${GL_HEADERS_SOURCES})
target_include_directories(GlHeaders INTERFACE ${CMAKE_CURRENT_LIST_DIR}/gl_headers)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${GL_HEADERS_SOURCES})
