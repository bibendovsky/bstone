/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_R2_EXTENT_INCLUDED)
#define BSTONE_SYS_R2_EXTENT_INCLUDED

namespace bstone {
namespace sys {

struct R2Extent
{
	int width{};
	int height{};

	R2Extent() = default;

	constexpr R2Extent(int width, int height) noexcept
		:
		width{width},
		height{height}
	{}
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_R2_EXTENT_INCLUDED
