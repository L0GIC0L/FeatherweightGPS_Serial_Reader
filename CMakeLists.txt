cmake_minimum_required(VERSION 3.5)
project(featherweightgps_serial_reader_cli)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

set(featherweightgps_serial_reader_cli_sources
        src/main.cpp
        src/plotfuncs.h src/serial.h src/functions.h)

add_executable(featherweightgps_serial_reader_cli ${featherweightgps_serial_reader_cli_sources})

target_include_directories(featherweightgps_serial_reader_cli PUBLIC
        )

target_link_libraries(featherweightgps_serial_reader_cli
        )

