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
// Ren3dCmdBufferImplInitException
//

class Ren3dCmdBufferImplInitException :
	public Exception
{
public:
	explicit Ren3dCmdBufferImplInitException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_CMD_BUF_INIT] "} + message}
	{
	}
}; // Ren3dCmdBufferImplInitException

//
// Ren3dCmdBufferImplInitException
// ==========================================================================


// ==========================================================================
// Ren3dCmdBufferImplReadException
//

class Ren3dCmdBufferImplReadException :
	public Exception
{
public:
	explicit Ren3dCmdBufferImplReadException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_CMD_BUF_READ] "} +message}
	{
	}
}; // Ren3dCmdBufferImplReadException

//
// Ren3dCmdBufferImplReadException
// ==========================================================================


// ==========================================================================
// Ren3dCmdBufferImplWriteException
//

class Ren3dCmdBufferImplWriteException :
	public Exception
{
public:
	explicit Ren3dCmdBufferImplWriteException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_CMD_BUF_WRITE] "} +message}
	{
	}
}; // Ren3dCmdBufferImplWriteException

//
// Ren3dCmdBufferImplWriteException
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
	static constexpr int get_min_initial_size()
	{
		return 4096;
	}

	static constexpr int get_min_resize_delta_size()
	{
		return 4096;
	}

	static constexpr int get_command_id_size()
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


	void validate(
		const Ren3dCreateCmdBufferParam& param);

	void resize_if_necessary(
		const int dst_delta_size);

	template<typename T>
	T* write(
		const Ren3dCmdId command_id)
	{
		if (is_reading_ || !is_writing_)
		{
			throw Ren3dCmdBufferImplWriteException("Invalid state.");
		}

		if (command_id == Ren3dCmdId::none)
		{
			throw Ren3dCmdBufferImplWriteException("Invalid command id.");
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

	template<typename T>
	const T* read()
	{
		if (!is_reading_ || is_writing_)
		{
			throw Ren3dCmdBufferImplReadException{"Invalid state."};
		}

		const auto command_size = static_cast<int>(sizeof(T));

		if ((size_ - read_offset_) < command_size)
		{
			throw Ren3dCmdBufferImplReadException{"Data underflow."};
		}

		auto command = reinterpret_cast<const T*>(&data_[read_offset_]);

		read_offset_ += command_size;

		return command;
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
{
	if (is_reading_)
	{
		throw Ren3dCmdBufferImplWriteException{"Already reading."};
	}

	if (is_writing_)
	{
		throw Ren3dCmdBufferImplWriteException{"Already writing."};
	}

	is_writing_ = true;
	write_offset_ = 0;
	command_count_ = 0;
}

void Ren3dCmdBufferImpl::end_write()
{
	if (is_reading_)
	{
		throw Ren3dCmdBufferImplWriteException{"Already reading."};
	}

	if (!is_writing_)
	{
		throw Ren3dCmdBufferImplWriteException{"Not writing."};
	}

	is_writing_ = false;
}

Ren3dClearCmd* Ren3dCmdBufferImpl::write_clear()
{
	return write<Ren3dClearCmd>(Ren3dCmdId::clear);
}

Ren3dSetViewportCmd* Ren3dCmdBufferImpl::write_set_viewport()
{
	return write<Ren3dSetViewportCmd>(Ren3dCmdId::set_viewport);
}

Ren3dEnableScissorCmd* Ren3dCmdBufferImpl::write_enable_scissor()
{
	return write<Ren3dEnableScissorCmd>(Ren3dCmdId::enable_scissor);
}

Ren3dSetScissorBoxCmd* Ren3dCmdBufferImpl::write_set_scissor_box()
{
	return write<Ren3dSetScissorBoxCmd>(Ren3dCmdId::set_scissor_box);
}

Ren3dEnableCullingCmd* Ren3dCmdBufferImpl::write_enable_culling()
{
	return write<Ren3dEnableCullingCmd>(Ren3dCmdId::enable_culling);
}

Ren3dEnableDepthTestCmd* Ren3dCmdBufferImpl::write_enable_depth_test()
{
	return write<Ren3dEnableDepthTestCmd>(Ren3dCmdId::enable_depth_test);
}

Ren3dEnableDepthWriteCmd* Ren3dCmdBufferImpl::write_enable_depth_write()
{
	return write<Ren3dEnableDepthWriteCmd>(Ren3dCmdId::enable_depth_write);
}

Ren3dEnableBlendingCmd* Ren3dCmdBufferImpl::write_enable_blending()
{
	return write<Ren3dEnableBlendingCmd>(Ren3dCmdId::enable_blending);
}

Ren3dSetBlendingFuncCmd* Ren3dCmdBufferImpl::write_set_blending_func()
{
	return write<Ren3dSetBlendingFuncCmd>(Ren3dCmdId::set_blending_func);
}

Ren3dSetTextureCmd* Ren3dCmdBufferImpl::write_set_texture()
{
	return write<Ren3dSetTextureCmd>(Ren3dCmdId::set_texture);
}

Ren3dSetSamplerCmd* Ren3dCmdBufferImpl::write_set_sampler()
{
	return write<Ren3dSetSamplerCmd>(Ren3dCmdId::set_sampler);
}

Ren3dSetVertexInputCmd* Ren3dCmdBufferImpl::write_set_vertex_input()
{
	return write<Ren3dSetVertexInputCmd>(Ren3dCmdId::set_vertex_input);
}

Ren3dSetShaderStageCmd* Ren3dCmdBufferImpl::write_set_shader_stage()
{
	return write<Ren3dSetShaderStageCmd>(Ren3dCmdId::set_shader_stage);
}

Ren3dSetInt32UniformCmd* Ren3dCmdBufferImpl::write_set_int32_uniform()
{
	return write<Ren3dSetInt32UniformCmd>(Ren3dCmdId::set_int32_uniform);
}

Ren3dSetFloat32UniformCmd* Ren3dCmdBufferImpl::write_set_float32_uniform()
{
	return write<Ren3dSetFloat32UniformCmd>(Ren3dCmdId::set_float32_uniform);
}

Ren3dSetVec2UniformCmd* Ren3dCmdBufferImpl::write_set_vec2_uniform()
{
	return write<Ren3dSetVec2UniformCmd>(Ren3dCmdId::set_vec2_uniform);
}

Ren3dSetVec4UniformCmd* Ren3dCmdBufferImpl::write_set_vec4_uniform()
{
	return write<Ren3dSetVec4UniformCmd>(Ren3dCmdId::set_vec4_uniform);
}

Ren3dSetMat4UniformCmd* Ren3dCmdBufferImpl::write_set_mat4_uniform()
{
	return write<Ren3dSetMat4UniformCmd>(Ren3dCmdId::set_mat4_uniform);
}

Ren3dSetSampler2dUniformCmd* Ren3dCmdBufferImpl::write_set_sampler_2d_uniform()
{
	return write<Ren3dSetSampler2dUniformCmd>(Ren3dCmdId::set_sampler_2d_uniform);
}

Ren3dDrawIndexedCmd* Ren3dCmdBufferImpl::write_draw_indexed()
{
	return write<Ren3dDrawIndexedCmd>(Ren3dCmdId::draw_indexed);
}

void Ren3dCmdBufferImpl::begin_read()
{
	if (is_reading_)
	{
		throw Ren3dCmdBufferImplReadException{"Already reading."};
	}

	if (is_writing_)
	{
		throw Ren3dCmdBufferImplReadException{"Already writing."};
	}

	is_reading_ = true;
	read_offset_ = 0;
}

void Ren3dCmdBufferImpl::end_read()
{
	if (!is_reading_)
	{
		throw Ren3dCmdBufferImplReadException{"Not reading."};
	}

	if (is_writing_)
	{
		throw Ren3dCmdBufferImplReadException{"Already writing."};
	}

	if (write_offset_ != read_offset_)
	{
		throw Ren3dCmdBufferImplReadException{"Offset mismatch."};
	}

	is_reading_ = false;
}

Ren3dCmdId Ren3dCmdBufferImpl::read_command_id()
{
	return *read<Ren3dCmdId>();
}

const Ren3dClearCmd* Ren3dCmdBufferImpl::read_clear()
{
	return read<Ren3dClearCmd>();
}

const Ren3dSetViewportCmd* Ren3dCmdBufferImpl::read_set_viewport()
{
	return read<Ren3dSetViewportCmd>();
}

const Ren3dEnableScissorCmd* Ren3dCmdBufferImpl::read_enable_scissor()
{
	return read<Ren3dEnableScissorCmd>();
}

const Ren3dSetScissorBoxCmd* Ren3dCmdBufferImpl::read_set_scissor_box()
{
	return read<Ren3dSetScissorBoxCmd>();
}

const Ren3dEnableCullingCmd* Ren3dCmdBufferImpl::read_enable_culling()
{
	return read<Ren3dEnableCullingCmd>();
}

const Ren3dEnableDepthTestCmd* Ren3dCmdBufferImpl::read_enable_depth_test()
{
	return read<Ren3dEnableDepthTestCmd>();
}

const Ren3dEnableDepthWriteCmd* Ren3dCmdBufferImpl::read_enable_depth_write()
{
	return read<Ren3dEnableDepthWriteCmd>();
}

const Ren3dEnableBlendingCmd* Ren3dCmdBufferImpl::read_enable_blending()
{
	return read<Ren3dEnableBlendingCmd>();
}

const Ren3dSetBlendingFuncCmd* Ren3dCmdBufferImpl::read_set_blending_func()
{
	return read<Ren3dSetBlendingFuncCmd>();
}

const Ren3dSetTextureCmd* Ren3dCmdBufferImpl::read_set_texture()
{
	return read<Ren3dSetTextureCmd>();
}

const Ren3dSetSamplerCmd* Ren3dCmdBufferImpl::read_set_sampler()
{
	return read<Ren3dSetSamplerCmd>();
}

const Ren3dSetVertexInputCmd* Ren3dCmdBufferImpl::read_set_vertex_input()
{
	return read<Ren3dSetVertexInputCmd>();
}

const Ren3dSetShaderStageCmd* Ren3dCmdBufferImpl::read_set_shader_stage()
{
	return read<Ren3dSetShaderStageCmd>();
}

const Ren3dSetInt32UniformCmd* Ren3dCmdBufferImpl::read_set_int32_uniform()
{
	return read<Ren3dSetInt32UniformCmd>();
}

const Ren3dSetFloat32UniformCmd* Ren3dCmdBufferImpl::read_set_float32_uniform()
{
	return read<Ren3dSetFloat32UniformCmd>();
}

const Ren3dSetVec2UniformCmd* Ren3dCmdBufferImpl::read_set_vec2_uniform()
{
	return read<Ren3dSetVec2UniformCmd>();
}

const Ren3dSetVec4UniformCmd* Ren3dCmdBufferImpl::read_set_vec4_uniform()
{
	return read<Ren3dSetVec4UniformCmd>();
}

const Ren3dSetMat4UniformCmd* Ren3dCmdBufferImpl::read_set_mat4_uniform()
{
	return read<Ren3dSetMat4UniformCmd>();
}

const Ren3dSetSampler2dUniformCmd* Ren3dCmdBufferImpl::read_set_sampler_2d_uniform()
{
	return read<Ren3dSetSampler2dUniformCmd>();
}

const Ren3dDrawIndexedCmd* Ren3dCmdBufferImpl::read_draw_indexed()
{
	return read<Ren3dDrawIndexedCmd>();
}

void Ren3dCmdBufferImpl::validate(
	const Ren3dCreateCmdBufferParam& param)
{
	if (param.initial_size_ <= 0)
	{
		throw Ren3dCmdBufferImplInitException{"Initial size out of range."};
	}

	if (param.resize_delta_size_ < 0)
	{
		throw Ren3dCmdBufferImplInitException{"Resize delta out of range."};
	}
}

void Ren3dCmdBufferImpl::resize_if_necessary(
	const int dst_delta_size)
{
	if ((size_ - write_offset_) >= dst_delta_size)
	{
		return;
	}
	else if (dst_delta_size == 0)
	{
		// Fixed-size buffer.
		//
		throw Ren3dCmdBufferImplWriteException{"Data overflow."};
	}

	size_ += resize_delta_size_;

	data_.resize(size_);
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
