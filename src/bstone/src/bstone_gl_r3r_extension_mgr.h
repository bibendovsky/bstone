/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Extension Manager

#ifndef BSTONE_GL_R3R_EXTENSION_MGR_INCLUDED
#define BSTONE_GL_R3R_EXTENSION_MGR_INCLUDED

#include <memory>
#include <string>
#include "bstone_gl_r3r_version.h"
#include "bstone_sys_gl_symbol_resolver.h"

namespace bstone {

enum class GlR3rExtensionId
{
	// Virtual extensions.
	//

	// glGetIntegerv.
	essentials,

	// OpenGL 2.0.
	v2_0,

	// OpenGL 3.2 (core).
	v3_2_core,

	// OpenGL ES 2.0
	es_v2_0,


	// Real extensions.
	//

	arb_buffer_storage,
	arb_direct_state_access,
	arb_framebuffer_object,
	arb_sampler_objects,
	arb_separate_shader_objects,
	arb_texture_filter_anisotropic,
	arb_texture_non_power_of_two,
	arb_vertex_array_object,

	ext_framebuffer_blit,
	ext_framebuffer_multisample,
	ext_framebuffer_object,
	ext_packed_depth_stencil,
	ext_texture_filter_anisotropic,

	oes_texture_npot,

	// Item count.
	count_,
};

class GlR3rExtensionMgr
{
public:
	GlR3rExtensionMgr() noexcept;
	virtual ~GlR3rExtensionMgr();

	virtual int get_count() const noexcept = 0;
	virtual const std::string& get_name(int extension_index) const noexcept = 0;
	virtual const GlR3rVersion& get_gl_version() const noexcept = 0;
	virtual const GlR3rVersion& get_glsl_version() const noexcept = 0;

	virtual void probe(GlR3rExtensionId extension_id) = 0;

	virtual bool has(GlR3rExtensionId extension_id) const noexcept = 0;
	virtual bool operator[](GlR3rExtensionId extension_id) const noexcept = 0;
};

// ==========================================================================

using GlR3rExtensionMgrUPtr = std::unique_ptr<GlR3rExtensionMgr>;

GlR3rExtensionMgrUPtr make_gl_r3r_extension_mgr(const sys::GlSymbolResolver& symbol_resolver);

} // namespace bstone

#endif // BSTONE_GL_R3R_EXTENSION_MGR_INCLUDED
