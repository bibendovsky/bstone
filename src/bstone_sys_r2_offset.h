/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_R2_OFFSET_INCLUDED)
#define BSTONE_SYS_R2_OFFSET_INCLUDED

namespace bstone {
namespace sys {

struct R2Offset
{
	int x{};
	int y{};

	R2Offset() = default;

	constexpr R2Offset(int x, int y) noexcept
		:
		x{x},
		y{y}
	{}
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_R2_OFFSET_INCLUDED
