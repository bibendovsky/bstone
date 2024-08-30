/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CONFIGURATIONS_INCLUDED
#define BSTONE_CONFIGURATIONS_INCLUDED

#include <cstdint>

namespace bstone {

extern const std::intptr_t posix_string_capacity;
extern const std::intptr_t win32_wstring_capacity;

extern const std::intptr_t sys_max_gl_contexts;
extern const std::intptr_t sys_max_gl_shared_libraries;
extern const std::intptr_t sys_max_textures;
extern const std::intptr_t sys_max_texture_locks;

} // namespace bstone

#endif // BSTONE_CONFIGURATIONS_INCLUDED
