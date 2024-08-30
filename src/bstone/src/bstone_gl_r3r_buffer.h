/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Buffer

#ifndef BSTONE_GL_R3R_BUFFER_INCLUDED
#define BSTONE_GL_R3R_BUFFER_INCLUDED

#include "bstone_r3r_buffer.h"

namespace bstone {

class GlR3rContext;

// =========================================================================

class GlR3rBuffer : public R3rBuffer
{
protected:
	GlR3rBuffer();

public:
	~GlR3rBuffer() override;

	virtual void set(bool is_set) = 0;
};

// =========================================================================

using GlR3rBufferUPtr = std::unique_ptr<GlR3rBuffer>;

GlR3rBufferUPtr make_gl_r3r_buffer(GlR3rContext& context, const R3rBufferInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_BUFFER_INCLUDED
