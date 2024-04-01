include(../third_party/imgui.cmake)

file(GLOB SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
source_group("" FILES ${SOURCE_FILES})

add_library(glBamboozle SHARED ${SOURCE_FILES})
target_common_setup(glBamboozle)
target_include_directories(glBamboozle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(glBamboozle PRIVATE
    DearImgui
    Comctl32.lib
    GlHeaders
)
target_compile_definitions(glBamboozle PRIVATE
    -DGLBAMBOOZLE_LOG_TO_CONSOLE=$<BOOL:${GLBAMBOOZLE_LOG_TO_CONSOLE}>
)
set_target_properties(glBamboozle PROPERTIES OUTPUT_NAME "opengl32")

include(codegen/codegen.cmake)
