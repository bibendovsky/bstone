/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Renderer (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_renderer.h"
#include <algorithm>


namespace bstone
{


// ==========================================================================
// Shader
//

RendererShaderVariable::RendererShaderVariable() = default;

RendererShaderVariable::~RendererShaderVariable() = default;


RendererShaderVariableInt32::RendererShaderVariableInt32() = default;

RendererShaderVariableInt32::~RendererShaderVariableInt32() = default;


RendererShaderVariableFloat32::RendererShaderVariableFloat32() = default;

RendererShaderVariableFloat32::~RendererShaderVariableFloat32() = default;


RendererShaderVariableVec2::RendererShaderVariableVec2() = default;

RendererShaderVariableVec2::~RendererShaderVariableVec2() = default;


RendererShaderVariableVec4::RendererShaderVariableVec4() = default;

RendererShaderVariableVec4::~RendererShaderVariableVec4() = default;


RendererShaderVariableMat4::RendererShaderVariableMat4() = default;

RendererShaderVariableMat4::~RendererShaderVariableMat4() = default;


RendererShader::RendererShader() = default;

RendererShader::~RendererShader() = default;


RendererShaderStage::RendererShaderStage() = default;

RendererShaderStage::~RendererShaderStage() = default;

//
// Shader
// ==========================================================================


// ==========================================================================
// RendererCommandBufferImpl
//

class RendererCommandBufferImpl final :
	public RendererCommandBuffer
{
public:
	RendererCommandBufferImpl();

	RendererCommandBufferImpl(
		RendererCommandBufferImpl&& rhs);

	~RendererCommandBufferImpl() override;


	int get_command_count() const override;


	bool is_enabled() const override;

	void enable(
		const bool is_enabled) override;


	void write_begin() override;

	void write_end() override;

	RendererCommandViewport* write_viewport() override;

	RendererCommandScissor* write_scissor() override;
	RendererCommandScissorBox* write_scissor_box() override;

	RendererCommandCulling* write_culling() override;

	RendererCommandDepthTest* write_depth_test() override;
	RendererCommandDepthWrite* write_depth_write() override;

	RendererCommandBlending* write_blending() override;
	RendererCommandBlendingFunction* write_blending_function() override;

	RendererCommandTexture* write_texture() override;
	RendererCommandSampler* write_sampler() override;

	RendererCommandVertexInput* write_vertex_input() override;

	RendererCommandShaderStage* write_shader_stage() override;

	RendererCommandShaderVariableInt32* write_shader_variable_int32() override;
	RendererCommandShaderVariableFloat32* write_shader_variable_float32() override;
	RendererCommandShaderVariableVec2* write_shader_variable_vec2() override;
	RendererCommandShaderVariableVec4* write_shader_variable_vec4() override;
	RendererCommandShaderVariableMat4* write_shader_variable_mat4() override;
	RendererCommandShaderVariableSampler2d* write_shader_variable_sampler_2d() override;

	RendererCommandDrawQuads* write_draw_quads() override;


	void read_begin() override;

	void read_end() override;

	RendererCommandId read_command_id() override;

	const RendererCommandViewport* read_viewport() override;

	const RendererCommandScissor* read_scissor() override;
	const RendererCommandScissorBox* read_scissor_box() override;

	const RendererCommandCulling* read_culling() override;

	const RendererCommandDepthTest* read_depth_test() override;
	const RendererCommandDepthWrite* read_depth_write() override;

	const RendererCommandBlending* read_blending() override;
	const RendererCommandBlendingFunction* read_blending_function() override;

	const RendererCommandTexture* read_texture() override;
	const RendererCommandSampler* read_sampler() override;

	const RendererCommandVertexInput* read_vertex_input() override;

	const RendererCommandShaderStage* read_shader_stage() override;

	const RendererCommandShaderVariableInt32* read_shader_variable_int32() override;
	const RendererCommandShaderVariableFloat32* read_shader_variable_float32() override;
	const RendererCommandShaderVariableVec2* read_shader_variable_vec2() override;
	const RendererCommandShaderVariableVec4* read_shader_variable_vec4() override;
	const RendererCommandShaderVariableMat4* read_shader_variable_mat4() override;
	const RendererCommandShaderVariableSampler2d* read_shader_variable_sampler_2d() override;

	const RendererCommandDrawQuads* read_draw_quads() override;


	void initialize(
		const RendererCommandManagerBufferAddParam& param);


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
		return static_cast<int>(sizeof(RendererCommandId));
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


	void resize_if_necessary(
		const int dst_delta_size);

	template<typename T>
	T* write(
		const RendererCommandId command_id)
	{
		if (is_reading_ || !is_writing_)
		{
			assert(!"Invalid state.");

			return nullptr;
		}

		if (command_id == RendererCommandId::none)
		{
			assert(!"Invalid command id.");

			return nullptr;
		}

		const auto command_size = static_cast<int>(sizeof(T));

		const auto block_size = get_command_id_size() + command_size;

		resize_if_necessary(block_size);

		auto block = reinterpret_cast<RendererCommandId*>(&data_[write_offset_]);
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
			assert(!"Invalid state.");

			return nullptr;
		}

		const auto command_size = static_cast<int>(sizeof(T));

		if ((size_ - read_offset_) < command_size)
		{
			return nullptr;
		}

		auto command = reinterpret_cast<const T*>(&data_[read_offset_]);

		read_offset_ += command_size;

		return command;
	}
}; // RendererCommandBufferImpl

using RendererCommandBufferImplPtr = RendererCommandBufferImpl*;
using RendererCommandBufferImplUPtr = std::unique_ptr<RendererCommandBufferImpl>;

//
// RendererCommandBufferImpl
// ==========================================================================


// ==========================================================================
// RendererCommandManagerImpl
//

class RendererCommandManagerImpl final :
	public RendererCommandManager
{
public:
	RendererCommandManagerImpl();

	~RendererCommandManagerImpl() override;


	int buffer_get_count() const override;

	RendererCommandBufferPtr buffer_add(
		const RendererCommandManagerBufferAddParam& param) override;

	void buffer_remove(
		RendererCommandBufferPtr buffer) override;

	RendererCommandBufferPtr buffer_get(
		const int index) override;


private:
	static constexpr auto reserved_buffer_count = 8;


	using Buffers = std::vector<RendererCommandBufferImplUPtr>;


	Buffers buffers_;


	static bool validate_param(
		const RendererCommandManagerBufferAddParam& param);
}; // RendererCommandManager

using RendererCommandManagerImplPtr = RendererCommandManagerImpl*;
using RendererCommandManagerImplUPtr = std::unique_ptr<RendererCommandManagerImpl>;

//
// RendererCommandManagerImpl
// ==========================================================================


// ==========================================================================
// RendererCommandBufferImpl
//

RendererCommandBufferImpl::RendererCommandBufferImpl()
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
}

RendererCommandBufferImpl::RendererCommandBufferImpl(
	RendererCommandBufferImpl&& rhs)
	:
	is_enabled_{std::move(rhs.is_enabled_)},
	is_reading_{std::move(rhs.is_reading_)},
	is_writing_{std::move(rhs.is_writing_)},
	size_{std::move(rhs.size_)},
	write_offset_{std::move(rhs.write_offset_)},
	read_offset_{std::move(rhs.read_offset_)},
	resize_delta_size_{std::move(rhs.resize_delta_size_)},
	command_count_{std::move(rhs.command_count_)},
	data_{std::move(rhs.data_)}
{
}

RendererCommandBufferImpl::~RendererCommandBufferImpl()
{
}

int RendererCommandBufferImpl::get_command_count() const
{
	return command_count_;
}

bool RendererCommandBufferImpl::is_enabled() const
{
	return is_enabled_;
}

void RendererCommandBufferImpl::enable(
	const bool is_enabled)
{
	is_enabled_ = is_enabled;
}

void RendererCommandBufferImpl::write_begin()
{
	if (is_reading_ || is_writing_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_writing_ = true;
	write_offset_ = 0;
	command_count_ = 0;
}

void RendererCommandBufferImpl::write_end()
{
	if (is_reading_ || !is_writing_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_writing_ = false;
}

RendererCommandViewport* RendererCommandBufferImpl::write_viewport()
{
	return write<RendererCommandViewport>(RendererCommandId::viewport_set);
}

RendererCommandScissor* RendererCommandBufferImpl::write_scissor()
{
	return write<RendererCommandScissor>(RendererCommandId::scissor_enable);
}

RendererCommandScissorBox* RendererCommandBufferImpl::write_scissor_box()
{
	return write<RendererCommandScissorBox>(RendererCommandId::scissor_set_box);
}

RendererCommandCulling* RendererCommandBufferImpl::write_culling()
{
	return write<RendererCommandCulling>(RendererCommandId::culling_enable);
}

RendererCommandDepthTest* RendererCommandBufferImpl::write_depth_test()
{
	return write<RendererCommandDepthTest>(RendererCommandId::depth_set_test);
}

RendererCommandDepthWrite* RendererCommandBufferImpl::write_depth_write()
{
	return write<RendererCommandDepthWrite>(RendererCommandId::depth_set_write);
}

RendererCommandBlending* RendererCommandBufferImpl::write_blending()
{
	return write<RendererCommandBlending>(RendererCommandId::blending_enable);
}

RendererCommandBlendingFunction* RendererCommandBufferImpl::write_blending_function()
{
	return write<RendererCommandBlendingFunction>(RendererCommandId::blending_function);
}

RendererCommandTexture* RendererCommandBufferImpl::write_texture()
{
	return write<RendererCommandTexture>(RendererCommandId::texture_set);
}

RendererCommandSampler* RendererCommandBufferImpl::write_sampler()
{
	return write<RendererCommandSampler>(RendererCommandId::sampler_set);
}

RendererCommandVertexInput* RendererCommandBufferImpl::write_vertex_input()
{
	return write<RendererCommandVertexInput>(RendererCommandId::vertex_input_set);
}

RendererCommandShaderStage* RendererCommandBufferImpl::write_shader_stage()
{
	return write<RendererCommandShaderStage>(RendererCommandId::shader_stage);
}

RendererCommandShaderVariableInt32* RendererCommandBufferImpl::write_shader_variable_int32()
{
	return write<RendererCommandShaderVariableInt32>(RendererCommandId::shader_variable_int32);
}

RendererCommandShaderVariableFloat32* RendererCommandBufferImpl::write_shader_variable_float32()
{
	return write<RendererCommandShaderVariableFloat32>(RendererCommandId::shader_variable_float32);
}

RendererCommandShaderVariableVec2* RendererCommandBufferImpl::write_shader_variable_vec2()
{
	return write<RendererCommandShaderVariableVec2>(RendererCommandId::shader_variable_vec2);
}

RendererCommandShaderVariableVec4* RendererCommandBufferImpl::write_shader_variable_vec4()
{
	return write<RendererCommandShaderVariableVec4>(RendererCommandId::shader_variable_vec4);
}

RendererCommandShaderVariableMat4* RendererCommandBufferImpl::write_shader_variable_mat4()
{
	return write<RendererCommandShaderVariableMat4>(RendererCommandId::shader_variable_mat4);
}

RendererCommandShaderVariableSampler2d* RendererCommandBufferImpl::write_shader_variable_sampler_2d()
{
	return write<RendererCommandShaderVariableSampler2d>(RendererCommandId::shader_variable_sampler2d);
}

RendererCommandDrawQuads* RendererCommandBufferImpl::write_draw_quads()
{
	return write<RendererCommandDrawQuads>(RendererCommandId::draw_quads);
}

void RendererCommandBufferImpl::read_begin()
{
	if (is_reading_ || is_writing_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_reading_ = true;
	read_offset_ = 0;
}

void RendererCommandBufferImpl::read_end()
{
	if (!is_reading_ || is_writing_)
	{
		assert(!"Invalid state.");

		return;
	}

	assert(write_offset_ == read_offset_);

	is_reading_ = false;
}

RendererCommandId RendererCommandBufferImpl::read_command_id()
{
	const auto command_id = read<RendererCommandId>();

	if (!command_id)
	{
		return RendererCommandId::none;
	}

	return *command_id;
}

const RendererCommandViewport* RendererCommandBufferImpl::read_viewport()
{
	return read<RendererCommandViewport>();
}

const RendererCommandScissor* RendererCommandBufferImpl::read_scissor()
{
	return read<RendererCommandScissor>();
}

const RendererCommandScissorBox* RendererCommandBufferImpl::read_scissor_box()
{
	return read<RendererCommandScissorBox>();
}

const RendererCommandCulling* RendererCommandBufferImpl::read_culling()
{
	return read<RendererCommandCulling>();
}

const RendererCommandDepthTest* RendererCommandBufferImpl::read_depth_test()
{
	return read<RendererCommandDepthTest>();
}

const RendererCommandDepthWrite* RendererCommandBufferImpl::read_depth_write()
{
	return read<RendererCommandDepthWrite>();
}

const RendererCommandBlending* RendererCommandBufferImpl::read_blending()
{
	return read<RendererCommandBlending>();
}

const RendererCommandBlendingFunction* RendererCommandBufferImpl::read_blending_function()
{
	return read<RendererCommandBlendingFunction>();
}

const RendererCommandTexture* RendererCommandBufferImpl::read_texture()
{
	return read<RendererCommandTexture>();
}

const RendererCommandSampler* RendererCommandBufferImpl::read_sampler()
{
	return read<RendererCommandSampler>();
}

const RendererCommandVertexInput* RendererCommandBufferImpl::read_vertex_input()
{
	return read<RendererCommandVertexInput>();
}

const RendererCommandShaderStage* RendererCommandBufferImpl::read_shader_stage()
{
	return read<RendererCommandShaderStage>();
}

const RendererCommandShaderVariableInt32* RendererCommandBufferImpl::read_shader_variable_int32()
{
	return read<RendererCommandShaderVariableInt32>();
}

const RendererCommandShaderVariableFloat32* RendererCommandBufferImpl::read_shader_variable_float32()
{
	return read<RendererCommandShaderVariableFloat32>();
}

const RendererCommandShaderVariableVec2* RendererCommandBufferImpl::read_shader_variable_vec2()
{
	return read<RendererCommandShaderVariableVec2>();
}

const RendererCommandShaderVariableVec4* RendererCommandBufferImpl::read_shader_variable_vec4()
{
	return read<RendererCommandShaderVariableVec4>();
}

const RendererCommandShaderVariableMat4* RendererCommandBufferImpl::read_shader_variable_mat4()
{
	return read<RendererCommandShaderVariableMat4>();
}

const RendererCommandShaderVariableSampler2d* RendererCommandBufferImpl::read_shader_variable_sampler_2d()
{
	return read<RendererCommandShaderVariableSampler2d>();
}

const RendererCommandDrawQuads* RendererCommandBufferImpl::read_draw_quads()
{
	return read<RendererCommandDrawQuads>();
}

void RendererCommandBufferImpl::initialize(
	const RendererCommandManagerBufferAddParam& param)
{
	is_reading_ = false;
	is_writing_ = false;

	size_ = std::max(param.initial_size_, get_min_initial_size());
	write_offset_ = 0;
	read_offset_ = 0;
	resize_delta_size_ = std::max(param.resize_delta_size_, get_min_resize_delta_size());

	data_.resize(size_);
}

void RendererCommandBufferImpl::resize_if_necessary(
	const int dst_delta_size)
{
	assert(dst_delta_size > 0);

	if ((size_ - write_offset_) >= dst_delta_size)
	{
		return;
	}

	size_ += resize_delta_size_;

	data_.resize(size_);
}

//
// RendererCommandBufferImpl
// ==========================================================================


// ==========================================================================
// RendererCommandManagerImpl
//

RendererCommandManagerImpl::RendererCommandManagerImpl()
	:
	buffers_{}
{
	buffers_.reserve(reserved_buffer_count);
}

RendererCommandManagerImpl::~RendererCommandManagerImpl()
{
}

int RendererCommandManagerImpl::buffer_get_count() const
{
	return static_cast<int>(buffers_.size());
}

RendererCommandBufferPtr RendererCommandManagerImpl::buffer_add(
	const RendererCommandManagerBufferAddParam& param)
{
	if (!validate_param(param))
	{
		return nullptr;
	}

	auto buffer = RendererCommandBufferImplUPtr{new RendererCommandBufferImpl{}};

	buffer->initialize(param);

	buffers_.push_back(std::move(buffer));

	return buffers_.back().get();
}

void RendererCommandManagerImpl::buffer_remove(
	RendererCommandBufferPtr buffer)
{
	if (!buffer)
	{
		assert(!"Null buffer.");

		return;
	}

	std::remove_if(
		buffers_.begin(),
		buffers_.end(),
		[&](const auto& item)
		{
			return item.get() == buffer;
		}
	);
}

RendererCommandBufferPtr RendererCommandManagerImpl::buffer_get(
	const int index)
{
	if (index < 0 || index >= buffer_get_count())
	{
		return nullptr;
	}

	return buffers_[index].get();
}

bool RendererCommandManagerImpl::validate_param(
	const RendererCommandManagerBufferAddParam& param)
{
	if (param.initial_size_ < 0)
	{
		return false;
	}

	if (param.resize_delta_size_ < 0)
	{
		return false;
	}

	return true;
}

//
// RendererCommandManagerImpl
// ==========================================================================


// ==========================================================================
// RendererCommandManagerFactory
//

RendererCommandManagerUPtr RendererCommandManagerFactory::create()
{
	return RendererCommandManagerImplUPtr{new RendererCommandManagerImpl{}};
}

//
// RendererCommandManagerFactory
// ==========================================================================


} // bstone
