cmake_minimum_required(VERSION 3.15)
project(mattd_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_executable(mattd_gui
    src/main.cpp
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_demo.cpp        # Add this line
    extern/imgui/backends/imgui_impl_glfw.cpp
    extern/imgui/backends/imgui_impl_opengl2.cpp
)
target_include_directories(mattd_gui PRIVATE
    extern/imgui
    extern/imgui/backends
)
target_compile_definitions(mattd_gui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

target_link_libraries(mattd_gui PRIVATE
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    dl pthread GL
)