#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
]]


cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
project(bstone_solution VERSION 1.0.0 LANGUAGES NONE)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


add_subdirectory (src)
