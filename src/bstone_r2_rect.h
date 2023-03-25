/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_R2_RECT_T_INCLUDED)
#define BSTONE_R2_RECT_T_INCLUDED

namespace bstone {

template<typename T>
struct R2RectT
{
	using Value = T;

	Value x{};
	Value y{};

	Value width{};
	Value height{};

	R2RectT() = default;

	constexpr R2RectT(Value x, Value y, Value width, Value height) noexcept
		:
		x{x},
		y{y},
		width{width},
		height{height}
	{}
};

// ==========================================================================

using R2RectI = R2RectT<int>;

} // namespace bstone

#endif // BSTONE_R2_RECT_T_INCLUDED
