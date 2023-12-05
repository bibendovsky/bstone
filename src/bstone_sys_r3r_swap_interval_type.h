/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D renderer swap interval type.

#if !defined(BSTONE_SYS_R3R_SWAP_INTERVAL_TYPE_INCLUDED)
#define BSTONE_SYS_R3R_SWAP_INTERVAL_TYPE_INCLUDED

namespace bstone {
namespace sys {

enum class R3rSwapIntervalType
{
	none,
	standard,
	adaptive,
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_R3R_SWAP_INTERVAL_TYPE_INCLUDED
