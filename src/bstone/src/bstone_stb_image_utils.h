/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STB_IMAGE_UTILS_INCLUDED
#define BSTONE_STB_IMAGE_UTILS_INCLUDED

#include <cstddef>

namespace bstone {

void* stb_cxx_malloc(std::size_t size) noexcept;
void stb_cxx_free(void* ptr) noexcept;
void* stb_cxx_realloc_sized(void* ptr, std::size_t old_size, std::size_t new_size) noexcept;

} // namespace bstone

#endif // BSTONE_STB_IMAGE_UTILS_INCLUDED
