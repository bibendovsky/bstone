/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context

#ifndef BSTONE_SYS_GL_CONTEXT_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_INCLUDED

#include "bstone_sys_gl_context_attributes.h"
#include <memory>

namespace bstone::sys {

class GlContext
{
public:
	GlContext() = default;
	virtual ~GlContext() = default;

	const GlContextAttributes& get_attributes() const;

private:
	virtual const GlContextAttributes& do_get_attributes() const = 0;
};

// ======================================

using GlContextUPtr = std::unique_ptr<GlContext>;

} // namespace bstone::sys

#endif // BSTONE_SYS_GL_CONTEXT_INCLUDED
