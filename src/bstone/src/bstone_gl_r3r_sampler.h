/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Sampler

#ifndef BSTONE_GL_R3R_SAMPLER_INCLUDED
#define BSTONE_GL_R3R_SAMPLER_INCLUDED

#include "bstone_r3r_sampler.h"

namespace bstone {

class GlR3rContext;

// =========================================================================

class GlR3rSampler : public R3rSampler
{
protected:
	GlR3rSampler();

public:
	~GlR3rSampler() override;

	virtual void set() = 0;
};

using GlR3rSamplerUPtr = std::unique_ptr<GlR3rSampler>;

// =========================================================================

GlR3rSamplerUPtr make_gl_r3r_sampler(GlR3rContext& context, const R3rSamplerInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_SAMPLER_INCLUDED
