/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context.

#ifndef BSTONE_SYS_GL_CONTEXT_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_INCLUDED

#include <memory>

#include "bstone_sys_gl_context_attributes.h"

namespace bstone {
namespace sys {

class GlContext
{
public:
	GlContext();
	virtual ~GlContext();

	const GlContextAttributes& get_attributes() const noexcept;

private:
	virtual const GlContextAttributes& do_get_attributes() const noexcept = 0;
};

// ==========================================================================

using GlContextUPtr = std::unique_ptr<GlContext>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_CONTEXT_INCLUDED
