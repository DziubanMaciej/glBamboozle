set(IMGUI_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_internal.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_truetype.h
)
add_library(DearImgui STATIC ${IMGUI_SOURCES})
target_include_directories(DearImgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)
# target_link_libraries(Imgui PUBLIC glfw OpenGL::GL)
target_compile_definitions(DearImgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/imgui FILES ${IMGUI_SOURCES})
