/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Sampler Manager

#ifndef BSTONE_GL_R3R_SAMPLER_MGR_INCLUDED
#define BSTONE_GL_R3R_SAMPLER_MGR_INCLUDED

#include <memory>
#include "bstone_r3r_sampler.h"

namespace bstone {

class GlR3rContext;

// ==========================================================================

class GlR3rSamplerMgr
{
public:
	GlR3rSamplerMgr() noexcept;
	virtual ~GlR3rSamplerMgr();

	virtual R3rSamplerUPtr create(const R3rSamplerInitParam& param) = 0;

	virtual void notify_destroy(const R3rSampler* sampler) noexcept = 0;

	virtual void set(R3rSampler* sampler) = 0;

	virtual const R3rSamplerState& get_current_state() const noexcept = 0;
};

using GlR3rSamplerMgrUPtr = std::unique_ptr<GlR3rSamplerMgr>;

// ==========================================================================

GlR3rSamplerMgrUPtr make_gl_r3r_sampler_mgr(GlR3rContext& context);

} // namespace bstone

#endif // BSTONE_GL_R3R_SAMPLER_MGR_INCLUDED
