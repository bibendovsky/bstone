/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// OpenGL context.
//


#ifndef BSTONE_DETAIL_REN_3D_GL_CONTEXT_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_CONTEXT_INCLUDED


#include <memory>

#include "bstone_ren_3d_types.h"

#include "bstone_detail_ren_3d_gl_device_features.h"


namespace bstone
{


struct Ren3dCreateBufferParam;
struct Ren3dCreateShaderParam;


namespace detail
{


class Ren3dGlVao;
using Ren3dGlVaoPtr = Ren3dGlVao*;

class Ren3dGlBuffer;
using Ren3dGlBufferUPtr = std::unique_ptr<Ren3dGlBuffer>;

class Ren3dGlShader;
using Ren3dGlShaderUPtr = std::unique_ptr<Ren3dGlShader>;

class Ren3dGlSamplerMgr;
using Ren3dGlSamplerMgrPtr = Ren3dGlSamplerMgr*;

class Ren3dGlTextureMgr;
using Ren3dGlTextureMgrPtr = Ren3dGlTextureMgr*;

class Ren3dGlVaoMgr;
using Ren3dGlVaoMgrPtr = Ren3dGlVaoMgr*;

class Ren3dGlVertexInput;
using Ren3dGlVertexInputPtr = Ren3dGlVertexInput*;

class Ren3dGlVertexInputMgr;
using Ren3dGlVertexInputMgrPtr = Ren3dGlVertexInputMgr*;

class Ren3dGlSampler;
using Ren3dGlSamplerPtr = Ren3dGlSampler*;

class Ren3dGlTexture2d;
using Ren3dGlTexture2dPtr = Ren3dGlTexture2d*;

class Ren3dGlShaderMgr;
using Ren3dGlShaderMgrPtr = Ren3dGlShaderMgr*;

class Ren3dGlShaderStage;
using Ren3dGlShaderStagePtr = Ren3dGlShaderStage*;

class Ren3dGlShaderStageMgr;
using Ren3dGlShaderStageMgrPtr = Ren3dGlShaderStageMgr*;


// =========================================================================
// Ren3dGlContext
//

class Ren3dGlContext
{
public:
	Ren3dGlContext() = default;

	virtual ~Ren3dGlContext() = default;


	virtual const Ren3dDeviceFeatures& get_device_features() const noexcept = 0;

	virtual const Ren3dGlDeviceFeatures& get_gl_device_features() const noexcept = 0;


	virtual Ren3dGlBufferUPtr create_buffer(
		const Ren3dCreateBufferParam& param) = 0;

	virtual Ren3dGlSamplerMgrPtr get_sampler_manager() const noexcept = 0;

	virtual Ren3dGlTextureMgrPtr get_texture_manager() const noexcept = 0;

	virtual Ren3dGlVertexInputMgrPtr get_vertex_input_manager() const noexcept = 0;

	virtual Ren3dGlShaderUPtr create_shader(
		const Ren3dCreateShaderParam& param) = 0;

	virtual Ren3dGlShaderStageMgrPtr get_shader_stage_manager() const noexcept = 0;


	virtual Ren3dGlSamplerPtr get_sampler() const noexcept = 0;

	virtual void set_sampler(
		const Ren3dGlSamplerPtr sampler) noexcept = 0;


	virtual Ren3dGlTexture2dPtr get_texture_2d() const noexcept = 0;

	virtual void set_texture_2d(
		const Ren3dGlTexture2dPtr texture_2d) noexcept = 0;


	virtual Ren3dGlVertexInputPtr get_vertex_input() const noexcept = 0;

	virtual void set_vertex_input(
		const Ren3dGlVertexInputPtr vertex_input) noexcept = 0;


	virtual Ren3dGlShaderStagePtr get_shader_stage() const noexcept = 0;

	virtual void set_shader_stage(
		const Ren3dGlShaderStagePtr shader_stage) noexcept = 0;


	virtual void clear(
		const Rgba8& rgba_8) = 0;


	virtual void set_viewport(
		const Ren3dViewport& viewport) = 0;


	virtual void enable_scissor(
		const bool is_enable) = 0;

	virtual void set_scissor_box(
		const Ren3dScissorBox& scissor_box) = 0;


	virtual void enable_culling(
		const bool is_enable) = 0;


	virtual void enable_depth_test(
		const bool is_enable) = 0;

	virtual void enable_depth_write(
		const bool is_enable) = 0;


	virtual void enable_blending(
		const bool is_enable) = 0;

	virtual void set_blending_func(
		const Ren3dBlendingFunc& func) = 0;
}; // Ren3dGlContext


using Ren3dGlContextPtr = Ren3dGlContext*;
using Ren3dGlContextUPtr = std::unique_ptr<Ren3dGlContext>;

//
// Ren3dGlContext
// =========================================================================


// =========================================================================
// Ren3dGlContextFactory
//

struct Ren3dGlContextFactory
{
	static Ren3dGlContextUPtr create(
		const Ren3dKind renderer_kind,
		const Ren3dDeviceFeatures& device_features,
		const Ren3dGlDeviceFeatures& gl_device_features);
}; // Ren3dGlContextFactory


//
// Ren3dGlContextFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_CONTEXT_INCLUDED
