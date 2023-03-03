cmake_minimum_required(VERSION 3.5)
project(featherweightgps_serial_reader)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

set(glfw3_DIR "/var/run/host/usr/lib64/cmake/glfw3/")

set(GLEW_INCLUDE_DIR "/usr/include")
set(GLEW_LIBRARIES "/usr/lib64")
set(OpenGL_GL_PREFERENCE "GLVND")

set(featherweightgps_serial_reader_sources
        src/demo.cpp imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp imgui/imgui_widgets.cpp implot/implot.cpp
        implot/implot_items.cpp backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp common/App.cpp imgui/imgui_stdlib.cpp
        3rdparty/glad/src/glad.c common/Fonts/FontAwesome5.cpp
        common/Fonts/FontAwesome5Brands.cpp common/Fonts/RobotoBold.cpp
        common/Fonts/RobotoItalic.cpp common/Fonts/RobotoMonoBold.cpp
        common/Fonts/RobotoMonoRegular.cpp common/Fonts/RobotoRegular.cpp
        common/Fonts/RobotoMonoBold.cpp common/Fonts/RobotoMonoItalic.cpp
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_package(GLEW 2.2.0 REQUIRED)

add_executable(featherweightgps_serial_reader ${featherweightgps_serial_reader_sources})

target_include_directories(featherweightgps_serial_reader PUBLIC
        imgui implot backends 3rdparty/glad/include common/Fonts
)

target_link_libraries(featherweightgps_serial_reader
        ${GLFW_LIBRARIES} glfw ${CMAKE_DL_LIBS} curl
        GLEW::glew GL
)

