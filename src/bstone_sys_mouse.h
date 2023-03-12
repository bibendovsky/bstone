/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_MOUSE_INCLUDED)
#define BSTONE_SYS_MOUSE_INCLUDED

namespace bstone {
namespace sys {

struct Mouse
{
	static constexpr auto left_button_index = 0;
	static constexpr auto right_button_index = 1;
	static constexpr auto middle_button_index = 2;
	static constexpr auto x1_button_index = 3;
	static constexpr auto x2_button_index = 4;

	static constexpr auto left_button_mask = 1U << left_button_index;
	static constexpr auto right_button_mask = 1U << right_button_index;
	static constexpr auto middle_button_mask = 1U << middle_button_index;
	static constexpr auto x1_button_mask = 1U << x1_button_index;
	static constexpr auto x2_button_mask = 1U << x2_button_index;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_MOUSE_INCLUDED
