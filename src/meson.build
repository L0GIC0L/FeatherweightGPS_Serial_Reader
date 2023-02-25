featherweightgps_serial_reader_sources = [
  'demo.cpp', '../imgui/imgui.cpp', '../imgui/imgui_demo.cpp', '../imgui/imgui_draw.cpp', '../imgui/imgui_tables.cpp', '../imgui/imgui_widgets.cpp','../implot/implot.cpp',
  '../implot/implot_items.cpp', '../backends/imgui_impl_glfw.cpp', '../backends/imgui_impl_opengl3.cpp', '../common/App.cpp', '../imgui/imgui_stdlib.cpp', '../3rdparty/glad/src/glad.c', '../common/Fonts/FontAwesome5.cpp',
  '../common/Fonts/FontAwesome5Brands.cpp','../common/Fonts/RobotoBold.cpp', '../common/Fonts/RobotoItalic.cpp', '../common/Fonts/RobotoMonoBold.cpp', '../common/Fonts/RobotoMonoRegular.cpp', '../common/Fonts/RobotoRegular.cpp',
  '../common/Fonts/RobotoMonoBold.cpp', '../common/Fonts/RobotoMonoItalic.cpp',
]

glfw_dep = dependency('glfw3', 'gl')

executable('featherweightgps_serial_reader', featherweightgps_serial_reader_sources,
  dependencies : [glfw_dep],
  link_args: ['-lGL' ,'-lglfw', '-lGLEW', '-lcurl'],
  install: true,
)


