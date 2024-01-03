/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context attributes.

#include "bstone_sys_gl_context_attributes.h"

namespace bstone {
namespace sys {

GlContextAttributes GlContextAttributes::make_default() noexcept
{
	auto gl_attributes = GlContextAttributes{};
	gl_attributes.is_accelerated = true;
	gl_attributes.profile = GlContextProfile::compatibility;
	gl_attributes.major_version = 1;
	gl_attributes.minor_version = 1;
	return gl_attributes;
}

} // namespace sys
} // namespace bstone
