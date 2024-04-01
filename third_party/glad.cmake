set(GLAD_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/glad/include/glad/glad.h
    ${CMAKE_CURRENT_LIST_DIR}/glad/src/glad.c
    ${CMAKE_CURRENT_LIST_DIR}/glad_wgl/include/glad/glad_wgl.h
    ${CMAKE_CURRENT_LIST_DIR}/glad_wgl/src/glad_wgl.c
)
add_library(Glad STATIC ${GLAD_SOURCES})
target_include_directories(Glad PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/glad/include
    ${CMAKE_CURRENT_LIST_DIR}/glad_wgl/include
)
target_link_libraries(Glad PUBLIC GlHeaders)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${GLAD_SOURCES})
