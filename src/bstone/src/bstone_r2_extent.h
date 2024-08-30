/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_R2_EXTENT_T_INCLUDED
#define BSTONE_R2_EXTENT_T_INCLUDED

namespace bstone {

template<typename T>
struct R2ExtentT
{
	using Value = T;

	Value width{};
	Value height{};

	R2ExtentT() = default;

	constexpr R2ExtentT(Value width, Value height) noexcept
		:
		width{width},
		height{height}
	{}
};

// ==========================================================================

using R2ExtentI = R2ExtentT<int>;

} // namespace bstone

#endif // BSTONE_R2_EXTENT_T_INCLUDED
