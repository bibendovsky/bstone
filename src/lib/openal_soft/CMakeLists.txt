#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_lib_openal_soft VERSION 1.0.0 LANGUAGES NONE)

message(STATUS "[bstone_lib_openal_soft] v${PROJECT_VERSION}")

add_library(bstone_lib_openal_soft INTERFACE)
add_library(bstone::lib::openal_soft ALIAS bstone_lib_openal_soft)

target_include_directories(bstone_lib_openal_soft INTERFACE include/AL)
