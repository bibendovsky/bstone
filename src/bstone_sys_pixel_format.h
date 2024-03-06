/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Pixel format.

#ifndef BSTONE_SYS_PIXEL_FORMAT_INCLUDED
#define BSTONE_SYS_PIXEL_FORMAT_INCLUDED

namespace bstone {
namespace sys {

enum class PixelFormat
{
	none,
	b8g8r8a8, // non-packed
	r8g8b8,
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_PIXEL_FORMAT_INCLUDED
