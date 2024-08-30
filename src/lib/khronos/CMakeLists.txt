cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_lib_khronos VERSION 1.0.0 LANGUAGES NONE)

message(STATUS "[bstone_lib_khronos] v${PROJECT_VERSION}")

add_library(bstone_lib_khronos INTERFACE)
add_library(bstone::lib::khronos ALIAS bstone_lib_khronos)

target_include_directories(bstone_lib_khronos INTERFACE include)
