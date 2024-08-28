/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context.

#include "bstone_sys_gl_context.h"

namespace bstone {
namespace sys {

GlContext::GlContext() = default;

GlContext::~GlContext() = default;

const GlContextAttributes& GlContext::get_attributes() const noexcept
{
	return do_get_attributes();
}

} // namespace sys
} // namespace bstone
