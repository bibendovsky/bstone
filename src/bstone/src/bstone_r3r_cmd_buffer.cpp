/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Command Buffer

#include <algorithm>
#include "bstone_exception.h"
#include "bstone_r3r_cmd_buffer.h"

namespace bstone {

R3rCmdBuffer::R3rCmdBuffer() noexcept = default;

R3rCmdBuffer::~R3rCmdBuffer() = default;

int R3rCmdBuffer::get_count() const noexcept
{
	return do_get_count();
}

bool R3rCmdBuffer::is_enabled() const noexcept
{
	return do_is_enabled();
}

void R3rCmdBuffer::enable(bool is_enabled)
try {
	do_enable(is_enabled);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBuffer::begin_write()
try {
	do_begin_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBuffer::end_write()
try {
	do_end_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rClearCmd& R3rCmdBuffer::write_clear()
try {
	return do_write_clear();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetViewportCmd& R3rCmdBuffer::write_set_viewport()
try {
	return do_write_set_viewport();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableScissorCmd& R3rCmdBuffer::write_enable_scissor()
try {
	return do_write_enable_scissor();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetScissorBoxCmd& R3rCmdBuffer::write_set_scissor_box()
try {
	return do_write_set_scissor_box();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableCullingCmd& R3rCmdBuffer::write_enable_culling()
try {
	return do_write_enable_culling();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableDepthTestCmd& R3rCmdBuffer::write_enable_depth_test()
try {
	return do_write_enable_depth_test();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableDepthWriteCmd& R3rCmdBuffer::write_enable_depth_write()
try {
	return do_write_enable_depth_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableBlendingCmd& R3rCmdBuffer::write_enable_blending()
try {
	return do_write_enable_blending();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetBlendingFuncCmd& R3rCmdBuffer::write_set_blending_func()
try {
	return do_write_set_blending_func();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetTextureCmd& R3rCmdBuffer::write_set_texture()
try {
	return do_write_set_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetSamplerCmd& R3rCmdBuffer::write_set_sampler()
try {
	return do_write_set_sampler();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVertexInputCmd& R3rCmdBuffer::write_set_vertex_input()
try {
	return do_write_set_vertex_input();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetShaderStageCmd& R3rCmdBuffer::write_set_shader_stage()
try {
	return do_write_set_shader_stage();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetInt32UniformCmd& R3rCmdBuffer::write_set_i32_uniform()
try {
	return do_write_set_i32_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetFloat32UniformCmd& R3rCmdBuffer::write_set_f32_uniform()
try {
	return do_write_set_f32_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVec2UniformCmd& R3rCmdBuffer::write_set_vec2_uniform()
try {
	return do_write_set_vec2_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVec4UniformCmd& R3rCmdBuffer::write_set_vec4_uniform()
try {
	return do_write_set_vec4_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetMat4UniformCmd& R3rCmdBuffer::write_set_mat4_uniform()
try {
	return do_write_set_mat4_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetR2SamplerUniformCmd& R3rCmdBuffer::write_set_r2_sampler_uniform()
try {
	return do_write_set_r2_sampler_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rDrawIndexedCmd& R3rCmdBuffer::write_draw_indexed()
try {
	return do_write_draw_indexed();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBuffer::begin_read()
try {
	do_begin_read();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBuffer::end_read()
try {
	do_end_read();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rCmdId R3rCmdBuffer::read_command_id()
try {
	return do_read_command_id();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rClearCmd& R3rCmdBuffer::read_clear()
try {
	return do_read_clear();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetViewportCmd& R3rCmdBuffer::read_set_viewport()
try {
	return do_read_set_viewport();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableScissorCmd& R3rCmdBuffer::read_enable_scissor()
try {
	return do_read_enable_scissor();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetScissorBoxCmd& R3rCmdBuffer::read_set_scissor_box()
try {
	return do_read_set_scissor_box();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableCullingCmd& R3rCmdBuffer::read_enable_culling()
try {
	return do_read_enable_culling();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableDepthTestCmd& R3rCmdBuffer::read_enable_depth_test()
try {
	return do_read_enable_depth_test();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableDepthWriteCmd& R3rCmdBuffer::read_enable_depth_write()
try {
	return do_read_enable_depth_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableBlendingCmd& R3rCmdBuffer::read_enable_blending()
try {
	return do_read_enable_blending();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetBlendingFuncCmd& R3rCmdBuffer::read_set_blending_func()
try {
	return do_read_set_blending_func();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetTextureCmd& R3rCmdBuffer::read_set_texture()
try {
	return do_read_set_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetSamplerCmd& R3rCmdBuffer::read_set_sampler()
try {
	return do_read_set_sampler();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVertexInputCmd& R3rCmdBuffer::read_set_vertex_input()
try {
	return do_read_set_vertex_input();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetShaderStageCmd& R3rCmdBuffer::read_set_shader_stage()
try {
	return do_read_set_shader_stage();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetInt32UniformCmd& R3rCmdBuffer::read_set_int32_uniform()
try {
	return do_read_set_int32_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetFloat32UniformCmd& R3rCmdBuffer::read_set_float32_uniform()
try {
	return do_read_set_float32_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVec2UniformCmd& R3rCmdBuffer::read_set_vec2_uniform()
try {
	return do_read_set_vec2_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVec4UniformCmd& R3rCmdBuffer::read_set_vec4_uniform()
try {
	return do_read_set_vec4_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetMat4UniformCmd& R3rCmdBuffer::read_set_mat4_uniform()
try {
	return do_read_set_mat4_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetR2SamplerUniformCmd& R3rCmdBuffer::read_set_r2_sampler_uniform()
try {
	return do_read_set_r2_sampler_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rDrawIndexedCmd& R3rCmdBuffer::read_draw_indexed()
try {
	return do_read_draw_indexed();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

namespace {

// ==========================================================================

class R3rCmdBufferImpl final : public R3rCmdBuffer
{
public:
	R3rCmdBufferImpl(const R3rCmdBufferInitParam& param);
	~R3rCmdBufferImpl() override;

private:
	int do_get_count() const noexcept override;

	bool do_is_enabled() const noexcept override;
	void do_enable(bool is_enabled) override;

	void do_begin_write() override;
	void do_end_write() override;

	R3rClearCmd& do_write_clear() override;

	R3rSetViewportCmd& do_write_set_viewport() override;

	R3rEnableScissorCmd& do_write_enable_scissor() override;
	R3rSetScissorBoxCmd& do_write_set_scissor_box() override;

	R3rEnableCullingCmd& do_write_enable_culling() override;

	R3rEnableDepthTestCmd& do_write_enable_depth_test() override;
	R3rEnableDepthWriteCmd& do_write_enable_depth_write() override;

	R3rEnableBlendingCmd& do_write_enable_blending() override;
	R3rSetBlendingFuncCmd& do_write_set_blending_func() override;

	R3rSetTextureCmd& do_write_set_texture() override;
	R3rSetSamplerCmd& do_write_set_sampler() override;

	R3rSetVertexInputCmd& do_write_set_vertex_input() override;

	R3rSetShaderStageCmd& do_write_set_shader_stage() override;

	R3rSetInt32UniformCmd& do_write_set_i32_uniform() override;
	R3rSetFloat32UniformCmd& do_write_set_f32_uniform() override;
	R3rSetVec2UniformCmd& do_write_set_vec2_uniform() override;
	R3rSetVec4UniformCmd& do_write_set_vec4_uniform() override;
	R3rSetMat4UniformCmd& do_write_set_mat4_uniform() override;
	R3rSetR2SamplerUniformCmd& do_write_set_r2_sampler_uniform() override;

	R3rDrawIndexedCmd& do_write_draw_indexed() override;

	void do_begin_read() override;
	void do_end_read() override;
	R3rCmdId do_read_command_id() override;

	const R3rClearCmd& do_read_clear() override;

	const R3rSetViewportCmd& do_read_set_viewport() override;

	const R3rEnableScissorCmd& do_read_enable_scissor() override;
	const R3rSetScissorBoxCmd& do_read_set_scissor_box() override;

	const R3rEnableCullingCmd& do_read_enable_culling() override;

	const R3rEnableDepthTestCmd& do_read_enable_depth_test() override;
	const R3rEnableDepthWriteCmd& do_read_enable_depth_write() override;

	const R3rEnableBlendingCmd& do_read_enable_blending() override;
	const R3rSetBlendingFuncCmd& do_read_set_blending_func() override;

	const R3rSetTextureCmd& do_read_set_texture() override;
	const R3rSetSamplerCmd& do_read_set_sampler() override;

	const R3rSetVertexInputCmd& do_read_set_vertex_input() override;

	const R3rSetShaderStageCmd& do_read_set_shader_stage() override;

	const R3rSetInt32UniformCmd& do_read_set_int32_uniform() override;
	const R3rSetFloat32UniformCmd& do_read_set_float32_uniform() override;
	const R3rSetVec2UniformCmd& do_read_set_vec2_uniform() override;
	const R3rSetVec4UniformCmd& do_read_set_vec4_uniform() override;
	const R3rSetMat4UniformCmd& do_read_set_mat4_uniform() override;
	const R3rSetR2SamplerUniformCmd& do_read_set_r2_sampler_uniform() override;

	const R3rDrawIndexedCmd& do_read_draw_indexed() override;

private:
	static constexpr int get_min_initial_size() noexcept
	{
		return 4096;
	}

	static constexpr int get_min_resize_delta_size() noexcept
	{
		return 4096;
	}

	static constexpr int get_command_id_size() noexcept
	{
		return static_cast<int>(sizeof(R3rCmdId));
	}

private:
	using Data = std::vector<std::uint8_t>;

private:
	bool is_enabled_{};
	bool is_reading_{};
	bool is_writing_{};

	int size_{};
	int write_offset_{};
	int read_offset_{};
	int resize_delta_{};
	int command_count_{};

	Data data_{};

private:
	void validate(const R3rCmdBufferInitParam& param);

	void resize_if_necessary(int dst_delta);

	template<typename TCommand>
	TCommand& write(R3rCmdId command_id)
	try {
		if (is_reading_ || !is_writing_)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid state.");
		}

		if (command_id == R3rCmdId::none)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid command id.");
		}

		constexpr auto command_size = static_cast<int>(sizeof(TCommand));
		constexpr auto block_size = get_command_id_size() + command_size;

		resize_if_necessary(block_size);

		auto block = &data_[write_offset_];
		*reinterpret_cast<R3rCmdId*>(block) = command_id;

		write_offset_ += block_size;
		++command_count_;

		return *reinterpret_cast<TCommand*>(block + get_command_id_size());
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

	template<typename TCommand>
	const TCommand& read()
	try {
		if (!is_reading_ || is_writing_)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid state.");
		}

		const auto command_size = static_cast<int>(sizeof(TCommand));

		if ((size_ - read_offset_) < command_size)
		{
			BSTONE_THROW_STATIC_SOURCE("Data underflow.");
		}

		const auto& command = *reinterpret_cast<const TCommand*>(&data_[read_offset_]);
		read_offset_ += command_size;
		return command;
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
};

// ==========================================================================

R3rCmdBufferImpl::R3rCmdBufferImpl(const R3rCmdBufferInitParam& param)
try {
	validate(param);

	size_ = std::max(param.initial_size, get_min_initial_size());
	resize_delta_ = std::max(param.resize_delta, get_min_resize_delta_size());

	data_.resize(size_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rCmdBufferImpl::~R3rCmdBufferImpl() = default;

int R3rCmdBufferImpl::do_get_count() const noexcept
{
	return command_count_;
}

bool R3rCmdBufferImpl::do_is_enabled() const noexcept
{
	return is_enabled_;
}

void R3rCmdBufferImpl::do_enable(bool is_enabled)
{
	is_enabled_ = is_enabled;
}

void R3rCmdBufferImpl::do_begin_write()
try {
	if (is_reading_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already reading.");
	}

	if (is_writing_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already writing.");
	}

	is_writing_ = true;
	write_offset_ = 0;
	command_count_ = 0;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBufferImpl::do_end_write()
try {
	if (is_reading_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already reading.");
	}

	if (!is_writing_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not writing.");
	}

	is_writing_ = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rClearCmd& R3rCmdBufferImpl::do_write_clear()
try {
	return write<R3rClearCmd>(R3rCmdId::clear);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetViewportCmd& R3rCmdBufferImpl::do_write_set_viewport()
try {
	return write<R3rSetViewportCmd>(R3rCmdId::set_viewport);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableScissorCmd& R3rCmdBufferImpl::do_write_enable_scissor()
try {
	return write<R3rEnableScissorCmd>(R3rCmdId::enable_scissor);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetScissorBoxCmd& R3rCmdBufferImpl::do_write_set_scissor_box()
try {
	return write<R3rSetScissorBoxCmd>(R3rCmdId::set_scissor_box);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableCullingCmd& R3rCmdBufferImpl::do_write_enable_culling()
try {
	return write<R3rEnableCullingCmd>(R3rCmdId::enable_culling);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableDepthTestCmd& R3rCmdBufferImpl::do_write_enable_depth_test()
try {
	return write<R3rEnableDepthTestCmd>(R3rCmdId::enable_depth_test);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableDepthWriteCmd& R3rCmdBufferImpl::do_write_enable_depth_write()
try {
	return write<R3rEnableDepthWriteCmd>(R3rCmdId::enable_depth_write);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rEnableBlendingCmd& R3rCmdBufferImpl::do_write_enable_blending()
try {
	return write<R3rEnableBlendingCmd>(R3rCmdId::enable_blending);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetBlendingFuncCmd& R3rCmdBufferImpl::do_write_set_blending_func()
try {
	return write<R3rSetBlendingFuncCmd>(R3rCmdId::set_blending_func);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetTextureCmd& R3rCmdBufferImpl::do_write_set_texture()
try {
	return write<R3rSetTextureCmd>(R3rCmdId::set_texture);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetSamplerCmd& R3rCmdBufferImpl::do_write_set_sampler()
try {
	return write<R3rSetSamplerCmd>(R3rCmdId::set_sampler);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVertexInputCmd& R3rCmdBufferImpl::do_write_set_vertex_input()
try {
	return write<R3rSetVertexInputCmd>(R3rCmdId::set_vertex_input);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetShaderStageCmd& R3rCmdBufferImpl::do_write_set_shader_stage()
try {
	return write<R3rSetShaderStageCmd>(R3rCmdId::set_shader_stage);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetInt32UniformCmd& R3rCmdBufferImpl::do_write_set_i32_uniform()
try {
	return write<R3rSetInt32UniformCmd>(R3rCmdId::set_int32_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetFloat32UniformCmd& R3rCmdBufferImpl::do_write_set_f32_uniform()
try {
	return write<R3rSetFloat32UniformCmd>(R3rCmdId::set_float32_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVec2UniformCmd& R3rCmdBufferImpl::do_write_set_vec2_uniform()
try {
	return write<R3rSetVec2UniformCmd>(R3rCmdId::set_vec2_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetVec4UniformCmd& R3rCmdBufferImpl::do_write_set_vec4_uniform()
try {
	return write<R3rSetVec4UniformCmd>(R3rCmdId::set_vec4_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetMat4UniformCmd& R3rCmdBufferImpl::do_write_set_mat4_uniform()
try {
	return write<R3rSetMat4UniformCmd>(R3rCmdId::set_mat4_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSetR2SamplerUniformCmd& R3rCmdBufferImpl::do_write_set_r2_sampler_uniform()
try {
	return write<R3rSetR2SamplerUniformCmd>(R3rCmdId::set_r2_sampler_uniform);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rDrawIndexedCmd& R3rCmdBufferImpl::do_write_draw_indexed()
try {
	return write<R3rDrawIndexedCmd>(R3rCmdId::draw_indexed);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBufferImpl::do_begin_read()
try {
	if (is_reading_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already reading.");
	}

	if (is_writing_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already writing.");
	}

	is_reading_ = true;
	read_offset_ = 0;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBufferImpl::do_end_read()
try {
	if (!is_reading_)
	{
		BSTONE_THROW_STATIC_SOURCE("Not reading.");
	}

	if (is_writing_)
	{
		BSTONE_THROW_STATIC_SOURCE("Already writing.");
	}

	if (write_offset_ != read_offset_)
	{
		BSTONE_THROW_STATIC_SOURCE("Offset mismatch.");
	}

	is_reading_ = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rCmdId R3rCmdBufferImpl::do_read_command_id()
try {
	return read<R3rCmdId>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rClearCmd& R3rCmdBufferImpl::do_read_clear()
try {
	return read<R3rClearCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetViewportCmd& R3rCmdBufferImpl::do_read_set_viewport()
try {
	return read<R3rSetViewportCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableScissorCmd& R3rCmdBufferImpl::do_read_enable_scissor()
try {
	return read<R3rEnableScissorCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetScissorBoxCmd& R3rCmdBufferImpl::do_read_set_scissor_box()
try {
	return read<R3rSetScissorBoxCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableCullingCmd& R3rCmdBufferImpl::do_read_enable_culling()
try {
	return read<R3rEnableCullingCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableDepthTestCmd& R3rCmdBufferImpl::do_read_enable_depth_test()
try {
	return read<R3rEnableDepthTestCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableDepthWriteCmd& R3rCmdBufferImpl::do_read_enable_depth_write()
try {
	return read<R3rEnableDepthWriteCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rEnableBlendingCmd& R3rCmdBufferImpl::do_read_enable_blending()
try {
	return read<R3rEnableBlendingCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetBlendingFuncCmd& R3rCmdBufferImpl::do_read_set_blending_func()
try {
	return read<R3rSetBlendingFuncCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetTextureCmd& R3rCmdBufferImpl::do_read_set_texture()
try {
	return read<R3rSetTextureCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetSamplerCmd& R3rCmdBufferImpl::do_read_set_sampler()
try {
	return read<R3rSetSamplerCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVertexInputCmd& R3rCmdBufferImpl::do_read_set_vertex_input()
try {
	return read<R3rSetVertexInputCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetShaderStageCmd& R3rCmdBufferImpl::do_read_set_shader_stage()
try {
	return read<R3rSetShaderStageCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetInt32UniformCmd& R3rCmdBufferImpl::do_read_set_int32_uniform()
try {
	return read<R3rSetInt32UniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetFloat32UniformCmd& R3rCmdBufferImpl::do_read_set_float32_uniform()
try {
	return read<R3rSetFloat32UniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVec2UniformCmd& R3rCmdBufferImpl::do_read_set_vec2_uniform()
try {
	return read<R3rSetVec2UniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetVec4UniformCmd& R3rCmdBufferImpl::do_read_set_vec4_uniform()
try {
	return read<R3rSetVec4UniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetMat4UniformCmd& R3rCmdBufferImpl::do_read_set_mat4_uniform()
try {
	return read<R3rSetMat4UniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSetR2SamplerUniformCmd& R3rCmdBufferImpl::do_read_set_r2_sampler_uniform()
try {
	return read<R3rSetR2SamplerUniformCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rDrawIndexedCmd& R3rCmdBufferImpl::do_read_draw_indexed()
try {
	return read<R3rDrawIndexedCmd>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBufferImpl::validate(const R3rCmdBufferInitParam& param)
try {
	if (param.initial_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Initial size out of range.");
	}

	if (param.resize_delta < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Resize delta out of range.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rCmdBufferImpl::resize_if_necessary(int dst_delta_size)
try {
	if ((size_ - write_offset_) >= dst_delta_size)
	{
		return;
	}
	else if (dst_delta_size == 0)
	{
		// Fixed-size buffer.
		//
		BSTONE_THROW_STATIC_SOURCE("Data overflow.");
	}

	size_ += resize_delta_;
	data_.resize(size_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

} // namespace

R3rCmdBufferUPtr make_r3r_cmd_buffer(const R3rCmdBufferInitParam& param)
try {
	return std::make_unique<R3rCmdBufferImpl>(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
