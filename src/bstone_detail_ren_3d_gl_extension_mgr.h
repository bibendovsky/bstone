/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL extension manager (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_EXTENSION_MGR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_EXTENSION_MGR_INCLUDED


#include <memory>
#include <string>

#include "bstone_detail_ren_3d_gl_version.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlExtensionMgr
//

enum class Ren3dGlExtensionId
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
}; // Ren3dGlExtensionId


class Ren3dGlExtensionMgr
{
public:
	Ren3dGlExtensionMgr() noexcept = default;

	virtual ~Ren3dGlExtensionMgr() = default;


	virtual int get_count() const noexcept = 0;

	virtual const std::string& get_name(
		const int extension_index) const noexcept = 0;


	virtual const Ren3dGlVersion& get_gl_version() const noexcept = 0;

	virtual const Ren3dGlVersion& get_glsl_version() const noexcept = 0;


	virtual void probe(
		const Ren3dGlExtensionId extension_id) = 0;


	virtual bool has(
		const Ren3dGlExtensionId extension_id) const noexcept = 0;

	virtual bool operator[](
		const Ren3dGlExtensionId extension_id) const noexcept = 0;
}; // Ren3dGlExtensionMgr

using Ren3dGlExtensionMgrPtr = Ren3dGlExtensionMgr*;
using Ren3dGlExtensionMgrUPtr = std::unique_ptr<Ren3dGlExtensionMgr>;

//
// Ren3dGlExtensionMgr
// ==========================================================================


// ==========================================================================
// Ren3dGlExtensionMgrFactory
//

struct Ren3dGlExtensionMgrFactory
{
	static Ren3dGlExtensionMgrUPtr create();
}; // Ren3dGlExtensionMgrFactory

//
// Ren3dGlExtensionMgrFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_EXTENSION_MGR_INCLUDED
