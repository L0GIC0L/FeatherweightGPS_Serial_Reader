cmake_minimum_required(VERSION 3.5)
project(featherweightgps_serial_reader)

set(CMAKE_CXX_STANDARD 20)

set(OpenGL_GL_PREFERENCE "GLVND")

set(featherweightgps_serial_reader_sources
        src/fwgui.cpp imgui/imgui.cpp imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp imgui/imgui_widgets.cpp implot/implot.cpp
        implot/implot_items.cpp backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp common/Fonts/RobotoMonoRegular.cpp
)


find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

add_executable(featherweightgps_serial_reader ${featherweightgps_serial_reader_sources})

target_include_directories(featherweightgps_serial_reader PUBLIC
        imgui implot backends 3rdparty/glad/include common/Fonts
)

target_link_libraries(featherweightgps_serial_reader
        glfw curl GLEW::glew GL
)

