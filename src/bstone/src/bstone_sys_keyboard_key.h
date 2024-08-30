/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Keyboard keys.

#ifndef BSTONE_SYS_KEYBOARD_KEY_INCLUDED
#define BSTONE_SYS_KEYBOARD_KEY_INCLUDED

namespace bstone {
namespace sys {

enum class KeyboardKey
{
	none,

	n0,
	n1,
	n2,
	n3,
	n4,
	n5,
	n6,
	n7,
	n8,
	n9,

	a,
	b,
	c,
	d,
	e,
	f,
	g,
	h,
	i,
	j,
	k,
	l,
	m,
	n,
	o,
	p,
	q,
	r,
	s,
	t,
	u,
	v,
	w,
	x,
	y,
	z,

	f1,
	f2,
	f3,
	f4,
	f5,
	f6,
	f7,
	f8,
	f9,
	f10,
	f11,
	f12,

	left,
	right,
	down,
	up,

	insert,
	del,
	home,
	end,
	page_up,
	page_down,

	kp_0,
	kp_1,
	kp_2,
	kp_3,
	kp_4,
	kp_6,
	kp_7,
	kp_8,
	kp_9,
	kp_a,
	kp_b,
	kp_c,
	kp_d,
	kp_e,
	kp_f,

	kp_tab,
	kp_space,
	kp_plus,
	kp_minus,
	kp_left_brace,
	kp_right_brace,
	kp_enter,
	kp_divide,
	kp_comma,
	kp_backspace,

	caps_lock,
	scroll_lock,
	num_lock,

	left_alt,
	right_alt,
	left_ctrl,
	right_ctrl,
	left_shift,
	right_shift,
	left_gui,
	right_gui,

	backslash,
	backspace,
	backtick,
	comma,
	equals,
	esc,
	minus,
	pause,
	period,
	prt_scr,
	quote,
	left_bracket,
	right_bracket,
	enter,
	semicolon,
	slash,
	space,
	tab,
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_KEYBOARD_KEY_INCLUDED
