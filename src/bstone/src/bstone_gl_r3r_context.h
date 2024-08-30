/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Context

#ifndef BSTONE_GL_R3R_CONTEXT_INCLUDED
#define BSTONE_GL_R3R_CONTEXT_INCLUDED

#include <memory>
#include "bstone_sys_color.h"
#include "bstone_r3r_types.h"
#include "bstone_r3r_r2_texture.h"
#include "bstone_r3r_shader_stage.h"
#include "bstone_gl_r3r_device_features.h"

namespace bstone {

struct R3rBufferInitParam;
struct R3rShaderInitParam;

class GlR3rBuffer;
using GlR3rBufferUPtr = std::unique_ptr<GlR3rBuffer>;

class GlR3rShader;
using GlR3rShaderUPtr = std::unique_ptr<GlR3rShader>;

class GlR3rSamplerMgr;
class GlR3rTextureMgr;
class GlR3rVertexInput;
class GlR3rVertexInputMgr;
class GlR3rSampler;
class GlR3rR2Texture;
class GlR3rShaderMgr;
class GlR3rShaderStage;
class GlR3rShaderStageMgr;

// =========================================================================

class GlR3rContext
{
public:
	GlR3rContext() noexcept;
	virtual ~GlR3rContext();

	virtual const R3rDeviceFeatures& get_device_features() const noexcept = 0;
	virtual const GlR3rDeviceFeatures& get_gl_device_features() const noexcept = 0;

	virtual GlR3rBufferUPtr create_buffer(const R3rBufferInitParam& param) = 0;

	virtual GlR3rSamplerMgr& get_sampler_manager() const noexcept = 0;
	virtual GlR3rVertexInputMgr& get_vertex_input_manager() const noexcept = 0;

	virtual GlR3rShaderUPtr create_shader(const R3rShaderInitParam& param) = 0;

	virtual GlR3rSampler* get_sampler() const noexcept = 0;
	virtual void set_sampler(GlR3rSampler* sampler) noexcept = 0;

	virtual R3rR2TextureUPtr create_r2_texture(const R3rR2TextureInitParam& param) = 0;
	virtual void bind_r2_texture(GlR3rR2Texture* r2_texture) = 0;
	virtual GlR3rR2Texture* get_r2_texture() const noexcept = 0;
	virtual void set_r2_texture(GlR3rR2Texture* r2_texture) noexcept = 0;

	virtual GlR3rVertexInput* get_vertex_input() const noexcept = 0;
	virtual void set_vertex_input(GlR3rVertexInput* vertex_input) noexcept = 0;

	virtual R3rShaderStageUPtr create_shader_stage(const R3rShaderStageInitParam& param) = 0;
	virtual void bind_shader_stage(R3rShaderStage* shader_stage) = 0;
	virtual GlR3rShaderStage* get_shader_stage() const noexcept = 0;
	virtual void set_shader_stage(GlR3rShaderStage* shader_stage) noexcept = 0;

	virtual void clear(const sys::Color& color) = 0;

	virtual void set_viewport(const R3rViewport& viewport) = 0;
	virtual void enable_scissor(bool is_enable) = 0;
	virtual void set_scissor_box(const R3rScissorBox& scissor_box) = 0;

	virtual void enable_culling(bool is_enable) = 0;
	virtual void enable_depth_test(bool is_enable) = 0;
	virtual void enable_depth_write(bool is_enable) = 0;

	virtual void enable_blending(bool is_enable) = 0;
	virtual void set_blending_func(const R3rBlendingFunc& func) = 0;
};

// =========================================================================

using GlR3rContextUPtr = std::unique_ptr<GlR3rContext>;

GlR3rContextUPtr make_gl_r3r_context(
	R3rType renderer_type,
	const R3rDeviceFeatures& device_features,
	const GlR3rDeviceFeatures& gl_device_features);

} // namespace bstone

#endif // BSTONE_GL_R3R_CONTEXT_INCLUDED
