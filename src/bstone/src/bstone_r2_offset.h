/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_R2_OFFSET_T_INCLUDED
#define BSTONE_R2_OFFSET_T_INCLUDED

namespace bstone {

template<typename T>
struct R2OffsetT
{
	using Value = T;

	Value x{};
	Value y{};

	R2OffsetT() = default;

	constexpr R2OffsetT(Value x, Value y) noexcept
		:
		x{x},
		y{y}
	{}
};

// ==========================================================================

using R2OffsetI = R2OffsetT<int>;

} // namespace bstone

#endif // BSTONE_R2_OFFSET_T_INCLUDED
