/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// 3D renderer's command buffer (implementation).
//


#include "bstone_ren_3d_cmd_buffer.h"

#include <algorithm>

#include "bstone_exception.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dCmdBufferException
//

class Ren3dCmdBufferException :
	public Exception
{
public:
	explicit Ren3dCmdBufferException(
		const char* message) noexcept
		:
		Exception{"REN_3D_CMD_BUFFER", message}
	{
	}
}; // Ren3dCmdBufferException

//
// Ren3dCmdBufferException
// ==========================================================================


// ==========================================================================
// Ren3dCmdBufferImpl
//

class Ren3dCmdBufferImpl final :
	public Ren3dCmdBuffer
{
public:
	Ren3dCmdBufferImpl(
		const Ren3dCreateCmdBufferParam& param);

	~Ren3dCmdBufferImpl() override;


	int get_count() const noexcept override;


	bool is_enabled() const noexcept override;

	void enable(
		const bool is_enabled) override;


	void begin_write() override;

	void end_write() override;

	Ren3dClearCmd* write_clear() override;

	Ren3dSetViewportCmd* write_set_viewport() override;

	Ren3dEnableScissorCmd* write_enable_scissor() override;
	Ren3dSetScissorBoxCmd* write_set_scissor_box() override;

	Ren3dEnableCullingCmd* write_enable_culling() override;

	Ren3dEnableDepthTestCmd* write_enable_depth_test() override;
	Ren3dEnableDepthWriteCmd* write_enable_depth_write() override;

	Ren3dEnableBlendingCmd* write_enable_blending() override;
	Ren3dSetBlendingFuncCmd* write_set_blending_func() override;

	Ren3dSetTextureCmd* write_set_texture() override;
	Ren3dSetSamplerCmd* write_set_sampler() override;

	Ren3dSetVertexInputCmd* write_set_vertex_input() override;

	Ren3dSetShaderStageCmd* write_set_shader_stage() override;

	Ren3dSetInt32UniformCmd* write_set_int32_uniform() override;
	Ren3dSetFloat32UniformCmd* write_set_float32_uniform() override;
	Ren3dSetVec2UniformCmd* write_set_vec2_uniform() override;
	Ren3dSetVec4UniformCmd* write_set_vec4_uniform() override;
	Ren3dSetMat4UniformCmd* write_set_mat4_uniform() override;
	Ren3dSetSampler2dUniformCmd* write_set_sampler_2d_uniform() override;

	Ren3dDrawIndexedCmd* write_draw_indexed() override;


	void begin_read() override;

	void end_read() override;

	Ren3dCmdId read_command_id() override;

	const Ren3dClearCmd* read_clear() override;

	const Ren3dSetViewportCmd* read_set_viewport() override;

	const Ren3dEnableScissorCmd* read_enable_scissor() override;
	const Ren3dSetScissorBoxCmd* read_set_scissor_box() override;

	const Ren3dEnableCullingCmd* read_enable_culling() override;

	const Ren3dEnableDepthTestCmd* read_enable_depth_test() override;
	const Ren3dEnableDepthWriteCmd* read_enable_depth_write() override;

	const Ren3dEnableBlendingCmd* read_enable_blending() override;
	const Ren3dSetBlendingFuncCmd* read_set_blending_func() override;

	const Ren3dSetTextureCmd* read_set_texture() override;
	const Ren3dSetSamplerCmd* read_set_sampler() override;

	const Ren3dSetVertexInputCmd* read_set_vertex_input() override;

	const Ren3dSetShaderStageCmd* read_set_shader_stage() override;

	const Ren3dSetInt32UniformCmd* read_set_int32_uniform() override;
	const Ren3dSetFloat32UniformCmd* read_set_float32_uniform() override;
	const Ren3dSetVec2UniformCmd* read_set_vec2_uniform() override;
	const Ren3dSetVec4UniformCmd* read_set_vec4_uniform() override;
	const Ren3dSetMat4UniformCmd* read_set_mat4_uniform() override;
	const Ren3dSetSampler2dUniformCmd* read_set_sampler_2d_uniform() override;

	const Ren3dDrawIndexedCmd* read_draw_indexed() override;


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
		return static_cast<int>(sizeof(Ren3dCmdId));
	}


	using Data = std::vector<std::uint8_t>;

	bool is_enabled_;
	bool is_reading_;
	bool is_writing_;

	int size_;
	int write_offset_;
	int read_offset_;
	int resize_delta_size_;
	int command_count_;

	Data data_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	void validate(
		const Ren3dCreateCmdBufferParam& param);

	void resize_if_necessary(
		const int dst_delta_size);

	template<typename T>
	T* write(
		const Ren3dCmdId command_id)
	try
	{
		if (is_reading_ || !is_writing_)
		{
			fail("Invalid state.");
		}

		if (command_id == Ren3dCmdId::none)
		{
			fail("Invalid command id.");
		}

		const auto command_size = static_cast<int>(sizeof(T));

		const auto block_size = get_command_id_size() + command_size;

		resize_if_necessary(block_size);

		auto block = reinterpret_cast<Ren3dCmdId*>(&data_[write_offset_]);
		*block = command_id;

		write_offset_ += block_size;
		++command_count_;

		return reinterpret_cast<T*>(block + 1);
	}
	catch (...)
	{
		fail_nested(__func__);
	}

	template<typename T>
	const T* read()
	try
	{
		if (!is_reading_ || is_writing_)
		{
			fail("Invalid state.");
		}

		const auto command_size = static_cast<int>(sizeof(T));

		if ((size_ - read_offset_) < command_size)
		{
			fail("Data underflow.");
		}

		auto command = reinterpret_cast<const T*>(&data_[read_offset_]);

		read_offset_ += command_size;

		return command;
	}
	catch (...)
	{
		fail_nested(__func__);
	}
}; // Ren3dCmdBufferImpl

using Ren3dCmdBufferImplPtr = Ren3dCmdBufferImpl*;
using Ren3dCmdBufferImplUPtr = std::unique_ptr<Ren3dCmdBufferImpl>;

//
// Ren3dCmdBufferImpl
// ==========================================================================


// ==========================================================================
// Ren3dCmdBufferImpl
//

Ren3dCmdBufferImpl::Ren3dCmdBufferImpl(
	const Ren3dCreateCmdBufferParam& param)
try
	:
	is_enabled_{},
	is_reading_{},
	is_writing_{},
	size_{},
	write_offset_{},
	read_offset_{},
	resize_delta_size_{},
	command_count_{},
	data_{}
{
	validate(param);

	size_ = std::max(param.initial_size_, get_min_initial_size());
	resize_delta_size_ = std::max(param.resize_delta_size_, get_min_resize_delta_size());

	data_.resize(size_);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dCmdBufferImpl::~Ren3dCmdBufferImpl() = default;

int Ren3dCmdBufferImpl::get_count() const noexcept
{
	return command_count_;
}

bool Ren3dCmdBufferImpl::is_enabled() const noexcept
{
	return is_enabled_;
}

void Ren3dCmdBufferImpl::enable(
	const bool is_enabled)
{
	is_enabled_ = is_enabled;
}

void Ren3dCmdBufferImpl::begin_write()
try
{
	if (is_reading_)
	{
		fail("Already reading.");
	}

	if (is_writing_)
	{
		fail("Already writing.");
	}

	is_writing_ = true;
	write_offset_ = 0;
	command_count_ = 0;
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dCmdBufferImpl::end_write()
try
{
	if (is_reading_)
	{
		fail("Already reading.");
	}

	if (!is_writing_)
	{
		fail("Not writing.");
	}

	is_writing_ = false;
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dClearCmd* Ren3dCmdBufferImpl::write_clear()
try
{
	return write<Ren3dClearCmd>(Ren3dCmdId::clear);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetViewportCmd* Ren3dCmdBufferImpl::write_set_viewport()
try
{
	return write<Ren3dSetViewportCmd>(Ren3dCmdId::set_viewport);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dEnableScissorCmd* Ren3dCmdBufferImpl::write_enable_scissor()
try
{
	return write<Ren3dEnableScissorCmd>(Ren3dCmdId::enable_scissor);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetScissorBoxCmd* Ren3dCmdBufferImpl::write_set_scissor_box()
try
{
	return write<Ren3dSetScissorBoxCmd>(Ren3dCmdId::set_scissor_box);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dEnableCullingCmd* Ren3dCmdBufferImpl::write_enable_culling()
try
{
	return write<Ren3dEnableCullingCmd>(Ren3dCmdId::enable_culling);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dEnableDepthTestCmd* Ren3dCmdBufferImpl::write_enable_depth_test()
try
{
	return write<Ren3dEnableDepthTestCmd>(Ren3dCmdId::enable_depth_test);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dEnableDepthWriteCmd* Ren3dCmdBufferImpl::write_enable_depth_write()
try
{
	return write<Ren3dEnableDepthWriteCmd>(Ren3dCmdId::enable_depth_write);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dEnableBlendingCmd* Ren3dCmdBufferImpl::write_enable_blending()
try
{
	return write<Ren3dEnableBlendingCmd>(Ren3dCmdId::enable_blending);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetBlendingFuncCmd* Ren3dCmdBufferImpl::write_set_blending_func()
try
{
	return write<Ren3dSetBlendingFuncCmd>(Ren3dCmdId::set_blending_func);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetTextureCmd* Ren3dCmdBufferImpl::write_set_texture()
try
{
	return write<Ren3dSetTextureCmd>(Ren3dCmdId::set_texture);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetSamplerCmd* Ren3dCmdBufferImpl::write_set_sampler()
try
{
	return write<Ren3dSetSamplerCmd>(Ren3dCmdId::set_sampler);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetVertexInputCmd* Ren3dCmdBufferImpl::write_set_vertex_input()
try
{
	return write<Ren3dSetVertexInputCmd>(Ren3dCmdId::set_vertex_input);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetShaderStageCmd* Ren3dCmdBufferImpl::write_set_shader_stage()
try
{
	return write<Ren3dSetShaderStageCmd>(Ren3dCmdId::set_shader_stage);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetInt32UniformCmd* Ren3dCmdBufferImpl::write_set_int32_uniform()
try
{
	return write<Ren3dSetInt32UniformCmd>(Ren3dCmdId::set_int32_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetFloat32UniformCmd* Ren3dCmdBufferImpl::write_set_float32_uniform()
try
{
	return write<Ren3dSetFloat32UniformCmd>(Ren3dCmdId::set_float32_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetVec2UniformCmd* Ren3dCmdBufferImpl::write_set_vec2_uniform()
try
{
	return write<Ren3dSetVec2UniformCmd>(Ren3dCmdId::set_vec2_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetVec4UniformCmd* Ren3dCmdBufferImpl::write_set_vec4_uniform()
try
{
	return write<Ren3dSetVec4UniformCmd>(Ren3dCmdId::set_vec4_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetMat4UniformCmd* Ren3dCmdBufferImpl::write_set_mat4_uniform()
try
{
	return write<Ren3dSetMat4UniformCmd>(Ren3dCmdId::set_mat4_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dSetSampler2dUniformCmd* Ren3dCmdBufferImpl::write_set_sampler_2d_uniform()
try
{
	return write<Ren3dSetSampler2dUniformCmd>(Ren3dCmdId::set_sampler_2d_uniform);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dDrawIndexedCmd* Ren3dCmdBufferImpl::write_draw_indexed()
try
{
	return write<Ren3dDrawIndexedCmd>(Ren3dCmdId::draw_indexed);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dCmdBufferImpl::begin_read()
try
{
	if (is_reading_)
	{
		fail("Already reading.");
	}

	if (is_writing_)
	{
		fail("Already writing.");
	}

	is_reading_ = true;
	read_offset_ = 0;
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dCmdBufferImpl::end_read()
try
{
	if (!is_reading_)
	{
		fail("Not reading.");
	}

	if (is_writing_)
	{
		fail("Already writing.");
	}

	if (write_offset_ != read_offset_)
	{
		fail("Offset mismatch.");
	}

	is_reading_ = false;
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dCmdId Ren3dCmdBufferImpl::read_command_id()
try
{
	return *read<Ren3dCmdId>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dClearCmd* Ren3dCmdBufferImpl::read_clear()
try
{
	return read<Ren3dClearCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetViewportCmd* Ren3dCmdBufferImpl::read_set_viewport()
try
{
	return read<Ren3dSetViewportCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dEnableScissorCmd* Ren3dCmdBufferImpl::read_enable_scissor()
try
{
	return read<Ren3dEnableScissorCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetScissorBoxCmd* Ren3dCmdBufferImpl::read_set_scissor_box()
try
{
	return read<Ren3dSetScissorBoxCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dEnableCullingCmd* Ren3dCmdBufferImpl::read_enable_culling()
try
{
	return read<Ren3dEnableCullingCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dEnableDepthTestCmd* Ren3dCmdBufferImpl::read_enable_depth_test()
try
{
	return read<Ren3dEnableDepthTestCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dEnableDepthWriteCmd* Ren3dCmdBufferImpl::read_enable_depth_write()
try
{
	return read<Ren3dEnableDepthWriteCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dEnableBlendingCmd* Ren3dCmdBufferImpl::read_enable_blending()
try
{
	return read<Ren3dEnableBlendingCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetBlendingFuncCmd* Ren3dCmdBufferImpl::read_set_blending_func()
try
{
	return read<Ren3dSetBlendingFuncCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetTextureCmd* Ren3dCmdBufferImpl::read_set_texture()
try
{
	return read<Ren3dSetTextureCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetSamplerCmd* Ren3dCmdBufferImpl::read_set_sampler()
try
{
	return read<Ren3dSetSamplerCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetVertexInputCmd* Ren3dCmdBufferImpl::read_set_vertex_input()
try
{
	return read<Ren3dSetVertexInputCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetShaderStageCmd* Ren3dCmdBufferImpl::read_set_shader_stage()
try
{
	return read<Ren3dSetShaderStageCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetInt32UniformCmd* Ren3dCmdBufferImpl::read_set_int32_uniform()
try
{
	return read<Ren3dSetInt32UniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetFloat32UniformCmd* Ren3dCmdBufferImpl::read_set_float32_uniform()
try
{
	return read<Ren3dSetFloat32UniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetVec2UniformCmd* Ren3dCmdBufferImpl::read_set_vec2_uniform()
try
{
	return read<Ren3dSetVec2UniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetVec4UniformCmd* Ren3dCmdBufferImpl::read_set_vec4_uniform()
try
{
	return read<Ren3dSetVec4UniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetMat4UniformCmd* Ren3dCmdBufferImpl::read_set_mat4_uniform()
try
{
	return read<Ren3dSetMat4UniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSetSampler2dUniformCmd* Ren3dCmdBufferImpl::read_set_sampler_2d_uniform()
try
{
	return read<Ren3dSetSampler2dUniformCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dDrawIndexedCmd* Ren3dCmdBufferImpl::read_draw_indexed()
try
{
	return read<Ren3dDrawIndexedCmd>();
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void Ren3dCmdBufferImpl::fail(
	const char* message)
{
	throw Ren3dCmdBufferException{message};
}

[[noreturn]]
void Ren3dCmdBufferImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dCmdBufferException{message});
}

void Ren3dCmdBufferImpl::validate(
	const Ren3dCreateCmdBufferParam& param)
try
{
	if (param.initial_size_ <= 0)
	{
		fail("Initial size out of range.");
	}

	if (param.resize_delta_size_ < 0)
	{
		fail("Resize delta out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dCmdBufferImpl::resize_if_necessary(
	const int dst_delta_size)
try
{
	if ((size_ - write_offset_) >= dst_delta_size)
	{
		return;
	}
	else if (dst_delta_size == 0)
	{
		// Fixed-size buffer.
		//
		fail("Data overflow.");
	}

	size_ += resize_delta_size_;

	data_.resize(size_);
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dCmdBufferImpl
// ==========================================================================


} // detail


// ==========================================================================
// Ren3dCmdBufferFactory
//

Ren3dCmdBufferUPtr Ren3dCmdBufferFactory::create(
	const Ren3dCreateCmdBufferParam& param)
{
	return std::make_unique<detail::Ren3dCmdBufferImpl>(param);
}

//
// Ren3dCmdBufferFactory
// ==========================================================================


} // bstone
