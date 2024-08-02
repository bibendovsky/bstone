/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context attributes.

#ifndef BSTONE_SYS_GL_CONTEXT_ATTRIBUTES_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_ATTRIBUTES_INCLUDED

#include "bstone_sys_gl_context_profile.h"

namespace bstone {
namespace sys {

struct GlContextAttributes
{
	bool is_accelerated;
	GlContextProfile profile;
	int major_version;
	int minor_version;
	int multisample_buffer_count;
	int multisample_sample_count;
	int red_bit_count;
	int green_bit_count;
	int blue_bit_count;
	int alpha_bit_count;
	int depth_bit_count;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_CONTEXT_ATTRIBUTES_INCLUDED
