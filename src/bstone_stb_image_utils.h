/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_STB_IMAGE_UTILS_INCLUDED
#define BSTONE_STB_IMAGE_UTILS_INCLUDED


#include <cstddef>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void* cpp_malloc(
	std::size_t size) noexcept;

void cpp_free(
	void* ptr) noexcept;

void* cpp_realloc(
	void* ptr,
	std::size_t new_size) noexcept;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_STB_IMAGE_UTILS_INCLUDED
