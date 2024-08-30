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

	enable_scissor,
	set_scissor_box,

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

struct R3rEnableScissorCmd
{
	bool is_enable;
};

struct R3rSetScissorBoxCmd
{
	R3rScissorBox scissor_box;
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
	R3rShaderInt32Var* var;
	int value;
};

struct R3rSetFloat32UniformCmd
{
	R3rShaderFloat32Var* var;
	float value;
};

struct R3rSetVec2UniformCmd
{
	R3rShaderVec2Var* var;
	R3rVec2 value;
};

struct R3rSetVec4UniformCmd
{
	R3rShaderVec4Var* var;
	R3rVec4 value;
};

struct R3rSetMat4UniformCmd
{
	R3rShaderMat4Var* var;
	R3rMat4 value;
};

struct R3rSetR2SamplerUniformCmd
{
	R3rShaderR2SamplerVar* var;
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
	R3rCmdBuffer() noexcept;
	virtual ~R3rCmdBuffer();

public:
	int get_count() const noexcept;
	bool is_enabled() const noexcept;
	void enable(bool is_enabled);

	void begin_write();
	void end_write();

	R3rClearCmd& write_clear();

	R3rSetViewportCmd& write_set_viewport();

	R3rEnableScissorCmd& write_enable_scissor();
	R3rSetScissorBoxCmd& write_set_scissor_box();

	R3rEnableCullingCmd& write_enable_culling();

	R3rEnableDepthTestCmd& write_enable_depth_test();
	R3rEnableDepthWriteCmd& write_enable_depth_write();

	R3rEnableBlendingCmd& write_enable_blending();
	R3rSetBlendingFuncCmd& write_set_blending_func();

	R3rSetTextureCmd& write_set_texture();
	R3rSetSamplerCmd& write_set_sampler();

	R3rSetVertexInputCmd& write_set_vertex_input();

	R3rSetShaderStageCmd& write_set_shader_stage();

	R3rSetInt32UniformCmd& write_set_i32_uniform();
	R3rSetFloat32UniformCmd& write_set_f32_uniform();
	R3rSetVec2UniformCmd& write_set_vec2_uniform();
	R3rSetVec4UniformCmd& write_set_vec4_uniform();
	R3rSetMat4UniformCmd& write_set_mat4_uniform();
	R3rSetR2SamplerUniformCmd& write_set_r2_sampler_uniform();

	R3rDrawIndexedCmd& write_draw_indexed();

	void begin_read();
	void end_read();

	R3rCmdId read_command_id();

	const R3rClearCmd& read_clear();

	const R3rSetViewportCmd& read_set_viewport();

	const R3rEnableScissorCmd& read_enable_scissor();
	const R3rSetScissorBoxCmd& read_set_scissor_box();

	const R3rEnableCullingCmd& read_enable_culling();

	const R3rEnableDepthTestCmd& read_enable_depth_test();
	const R3rEnableDepthWriteCmd& read_enable_depth_write();

	const R3rEnableBlendingCmd& read_enable_blending();
	const R3rSetBlendingFuncCmd& read_set_blending_func();

	const R3rSetTextureCmd& read_set_texture();
	const R3rSetSamplerCmd& read_set_sampler();

	const R3rSetVertexInputCmd& read_set_vertex_input();

	const R3rSetShaderStageCmd& read_set_shader_stage();

	const R3rSetInt32UniformCmd& read_set_int32_uniform();
	const R3rSetFloat32UniformCmd& read_set_float32_uniform();
	const R3rSetVec2UniformCmd& read_set_vec2_uniform();
	const R3rSetVec4UniformCmd& read_set_vec4_uniform();
	const R3rSetMat4UniformCmd& read_set_mat4_uniform();
	const R3rSetR2SamplerUniformCmd& read_set_r2_sampler_uniform();

	const R3rDrawIndexedCmd& read_draw_indexed();

private:
	virtual int do_get_count() const noexcept = 0;
	virtual bool do_is_enabled() const noexcept = 0;
	virtual void do_enable(bool is_enabled) = 0;

	virtual void do_begin_write() = 0;
	virtual void do_end_write() = 0;

	virtual R3rClearCmd& do_write_clear() = 0;

	virtual R3rSetViewportCmd& do_write_set_viewport() = 0;

	virtual R3rEnableScissorCmd& do_write_enable_scissor() = 0;
	virtual R3rSetScissorBoxCmd& do_write_set_scissor_box() = 0;

	virtual R3rEnableCullingCmd& do_write_enable_culling() = 0;

	virtual R3rEnableDepthTestCmd& do_write_enable_depth_test() = 0;
	virtual R3rEnableDepthWriteCmd& do_write_enable_depth_write() = 0;

	virtual R3rEnableBlendingCmd& do_write_enable_blending() = 0;
	virtual R3rSetBlendingFuncCmd& do_write_set_blending_func() = 0;

	virtual R3rSetTextureCmd& do_write_set_texture() = 0;
	virtual R3rSetSamplerCmd& do_write_set_sampler() = 0;

	virtual R3rSetVertexInputCmd& do_write_set_vertex_input() = 0;

	virtual R3rSetShaderStageCmd& do_write_set_shader_stage() = 0;

	virtual R3rSetInt32UniformCmd& do_write_set_i32_uniform() = 0;
	virtual R3rSetFloat32UniformCmd& do_write_set_f32_uniform() = 0;
	virtual R3rSetVec2UniformCmd& do_write_set_vec2_uniform() = 0;
	virtual R3rSetVec4UniformCmd& do_write_set_vec4_uniform() = 0;
	virtual R3rSetMat4UniformCmd& do_write_set_mat4_uniform() = 0;
	virtual R3rSetR2SamplerUniformCmd& do_write_set_r2_sampler_uniform() = 0;

	virtual R3rDrawIndexedCmd& do_write_draw_indexed() = 0;

	virtual void do_begin_read() = 0;
	virtual void do_end_read() = 0;

	virtual R3rCmdId do_read_command_id() = 0;

	virtual const R3rClearCmd& do_read_clear() = 0;

	virtual const R3rSetViewportCmd& do_read_set_viewport() = 0;

	virtual const R3rEnableScissorCmd& do_read_enable_scissor() = 0;
	virtual const R3rSetScissorBoxCmd& do_read_set_scissor_box() = 0;

	virtual const R3rEnableCullingCmd& do_read_enable_culling() = 0;

	virtual const R3rEnableDepthTestCmd& do_read_enable_depth_test() = 0;
	virtual const R3rEnableDepthWriteCmd& do_read_enable_depth_write() = 0;

	virtual const R3rEnableBlendingCmd& do_read_enable_blending() = 0;
	virtual const R3rSetBlendingFuncCmd& do_read_set_blending_func() = 0;

	virtual const R3rSetTextureCmd& do_read_set_texture() = 0;
	virtual const R3rSetSamplerCmd& do_read_set_sampler() = 0;

	virtual const R3rSetVertexInputCmd& do_read_set_vertex_input() = 0;

	virtual const R3rSetShaderStageCmd& do_read_set_shader_stage() = 0;

	virtual const R3rSetInt32UniformCmd& do_read_set_int32_uniform() = 0;
	virtual const R3rSetFloat32UniformCmd& do_read_set_float32_uniform() = 0;
	virtual const R3rSetVec2UniformCmd& do_read_set_vec2_uniform() = 0;
	virtual const R3rSetVec4UniformCmd& do_read_set_vec4_uniform() = 0;
	virtual const R3rSetMat4UniformCmd& do_read_set_mat4_uniform() = 0;
	virtual const R3rSetR2SamplerUniformCmd& do_read_set_r2_sampler_uniform() = 0;

	virtual const R3rDrawIndexedCmd& do_read_draw_indexed() = 0;
};

// ==========================================================================

using R3rCmdBufferUPtr = std::unique_ptr<R3rCmdBuffer>;

R3rCmdBufferUPtr make_r3r_cmd_buffer(const R3rCmdBufferInitParam& param);

} // namespace bstone

#endif // BSTONE_R3R_COMMAND_BUFFER_INCLUDED
