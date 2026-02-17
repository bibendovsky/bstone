/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Command Buffer

#ifndef BSTONE_R3R_COMMAND_BUFFER_INCLUDED
#define BSTONE_R3R_COMMAND_BUFFER_INCLUDED

#include "bstone_r3r.h"

namespace bstone {

enum class R3rCmdId
{
	none = 0,

	clear,

	set_viewport,

	enable_culling,

	enable_depth_test,
	enable_depth_write,

	enable_blending,
	set_blending_func,

	set_texture,
	set_sampler,
	set_vertex_input,
	set_shader_stage,

	set_int32_uniform,
	set_float32_uniform,
	set_vec2_uniform,
	set_vec4_uniform,
	set_mat4_uniform,
	set_r2_sampler_uniform,

	draw_indexed,
};

// ==========================================================================

struct R3rClearCmd
{
	R3rClearParam clear;
};

struct R3rSetViewportCmd
{
	R3rViewport viewport;
};

struct R3rEnableCullingCmd
{
	bool is_enable;
};

struct R3rEnableBlendingCmd
{
	bool is_enable;
};

struct R3rSetBlendingFuncCmd
{
	R3rBlendingFunc blending_func;
};

struct R3rEnableDepthTestCmd
{
	bool is_enable;
};

struct R3rEnableDepthWriteCmd
{
	bool is_enable;
};

struct R3rSetTextureCmd
{
	R3rR2Texture* r2_texture;
};

struct R3rSetSamplerCmd
{
	R3rSampler* sampler;
};

struct R3rSetVertexInputCmd
{
	R3rVertexInput* vertex_input;
};

struct R3rSetShaderStageCmd
{
	R3rShaderStage* shader_stage;
};

struct R3rSetInt32UniformCmd
{
	R3rShaderVar* var;
	int value;
};

struct R3rSetFloat32UniformCmd
{
	R3rShaderVar* var;
	float value;
};

struct R3rSetVec2UniformCmd
{
	R3rShaderVar* var;
	R3rVec2 value;
};

struct R3rSetVec4UniformCmd
{
	R3rShaderVar* var;
	R3rVec4 value;
};

struct R3rSetMat4UniformCmd
{
	R3rShaderVar* var;
	R3rMat4 value;
};

struct R3rSetR2SamplerUniformCmd
{
	R3rShaderVar* var;
	int value;
};

struct R3rDrawIndexedCmd
{
	R3rDrawIndexedParam draw_indexed;
};

// ==========================================================================

struct R3rCmdBufferInitParam
{
	int initial_size;
	int resize_delta;
};

// ==========================================================================

class R3rCmdBuffer
{
public:
	R3rCmdBuffer() = default;
	virtual ~R3rCmdBuffer() = default;

	virtual int get_count() const = 0;
	virtual bool is_enabled() const = 0;
	virtual void enable(bool is_enabled) = 0;

	virtual void begin_write() = 0;
	virtual void end_write() = 0;

	virtual R3rClearCmd& write_clear() = 0;

	virtual R3rSetViewportCmd& write_set_viewport() = 0;

	virtual R3rEnableCullingCmd& write_enable_culling() = 0;

	virtual R3rEnableDepthTestCmd& write_enable_depth_test() = 0;
	virtual R3rEnableDepthWriteCmd& write_enable_depth_write() = 0;

	virtual R3rEnableBlendingCmd& write_enable_blending() = 0;
	virtual R3rSetBlendingFuncCmd& write_set_blending_func() = 0;

	virtual R3rSetTextureCmd& write_set_texture() = 0;
	virtual R3rSetSamplerCmd& write_set_sampler() = 0;

	virtual R3rSetVertexInputCmd& write_set_vertex_input() = 0;

	virtual R3rSetShaderStageCmd& write_set_shader_stage() = 0;

	virtual R3rSetInt32UniformCmd& write_set_i32_uniform() = 0;
	virtual R3rSetFloat32UniformCmd& write_set_f32_uniform() = 0;
	virtual R3rSetVec2UniformCmd& write_set_vec2_uniform() = 0;
	virtual R3rSetVec4UniformCmd& write_set_vec4_uniform() = 0;
	virtual R3rSetMat4UniformCmd& write_set_mat4_uniform() = 0;
	virtual R3rSetR2SamplerUniformCmd& write_set_r2_sampler_uniform() = 0;

	virtual R3rDrawIndexedCmd& write_draw_indexed() = 0;

	virtual void begin_read() = 0;
	virtual void end_read() = 0;

	virtual R3rCmdId read_command_id() = 0;

	virtual const R3rClearCmd& read_clear() = 0;

	virtual const R3rSetViewportCmd& read_set_viewport() = 0;

	virtual const R3rEnableCullingCmd& read_enable_culling() = 0;

	virtual const R3rEnableDepthTestCmd& read_enable_depth_test() = 0;
	virtual const R3rEnableDepthWriteCmd& read_enable_depth_write() = 0;

	virtual const R3rEnableBlendingCmd& read_enable_blending() = 0;
	virtual const R3rSetBlendingFuncCmd& read_set_blending_func() = 0;

	virtual const R3rSetTextureCmd& read_set_texture() = 0;
	virtual const R3rSetSamplerCmd& read_set_sampler() = 0;

	virtual const R3rSetVertexInputCmd& read_set_vertex_input() = 0;

	virtual const R3rSetShaderStageCmd& read_set_shader_stage() = 0;

	virtual const R3rSetInt32UniformCmd& read_set_int32_uniform() = 0;
	virtual const R3rSetFloat32UniformCmd& read_set_float32_uniform() = 0;
	virtual const R3rSetVec2UniformCmd& read_set_vec2_uniform() = 0;
	virtual const R3rSetVec4UniformCmd& read_set_vec4_uniform() = 0;
	virtual const R3rSetMat4UniformCmd& read_set_mat4_uniform() = 0;
	virtual const R3rSetR2SamplerUniformCmd& read_set_r2_sampler_uniform() = 0;

	virtual const R3rDrawIndexedCmd& read_draw_indexed() = 0;
};

// ==========================================================================

using R3rCmdBufferUPtr = std::unique_ptr<R3rCmdBuffer>;

R3rCmdBufferUPtr make_r3r_cmd_buffer(const R3rCmdBufferInitParam& param);

} // namespace bstone

#endif // BSTONE_R3R_COMMAND_BUFFER_INCLUDED
