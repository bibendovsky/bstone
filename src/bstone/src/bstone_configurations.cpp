/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_configurations.h"

namespace bstone {

extern const std::intptr_t posix_string_capacity = 16 * 1024;
extern const std::intptr_t win32_wstring_capacity = 16 * 1024;

const std::intptr_t sys_max_gl_contexts = 1;
const std::intptr_t sys_max_gl_shared_libraries = 1;
const std::intptr_t sys_max_textures = 2;
const std::intptr_t sys_max_texture_locks = 1;

} // namespace bstone
