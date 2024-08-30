/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: 2D Texture

#ifndef BSTONE_GL_R3R_R2_TEXTURE_INCLUDED
#define BSTONE_GL_R3R_R2_TEXTURE_INCLUDED

#include "bstone_r3r_r2_texture.h"
#include "bstone_r3r_sampler.h"

namespace bstone {

class GlR3rContext;

// =========================================================================

class GlR3rR2Texture : public R3rR2Texture
{
protected:
	GlR3rR2Texture();

public:
	~GlR3rR2Texture() override;

	virtual void set() = 0;
	virtual void bind() = 0;

	virtual void update_sampler_state(const R3rSamplerState& new_sampler_state) = 0;
};

// =========================================================================

using GlR3rR2TextureUPtr = std::unique_ptr<GlR3rR2Texture>;

GlR3rR2TextureUPtr make_gl_r3r_r2_texture(GlR3rContext& context, const R3rR2TextureInitParam& param);

} // bstone

#endif // BSTONE_GL_R3R_R2_TEXTURE_INCLUDED
