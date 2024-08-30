cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_lib_stb VERSION 1.0.0 LANGUAGES NONE)

message(STATUS "[bstone_lib_stb] v${PROJECT_VERSION}")

add_library(bstone_lib_stb INTERFACE)
add_library(bstone::lib::stb ALIAS bstone_lib_stb)

target_include_directories(bstone_lib_stb INTERFACE include)
