/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_gl_context_attributes.h"

namespace bstone {
namespace sys {

GlContextAttributes GlContextAttributes::make_default() noexcept
{
	auto window_gl_attribs = GlContextAttributes{};
	window_gl_attribs.is_accelerated = true;
	window_gl_attribs.profile = GlContextProfile::compatibility;
	window_gl_attribs.major_version = 1;
	window_gl_attribs.minor_version = 1;
	window_gl_attribs.multisample_buffer_count = 0;
	window_gl_attribs.multisample_sample_count = 0;
	window_gl_attribs.red_bit_count = 0;
	window_gl_attribs.green_bit_count = 0;
	window_gl_attribs.blue_bit_count = 0;
	window_gl_attribs.alpha_bit_count = 0;
	window_gl_attribs.depth_bit_count = 0;
	return window_gl_attribs;
}

} // namespace sys
} // namespace bstone
