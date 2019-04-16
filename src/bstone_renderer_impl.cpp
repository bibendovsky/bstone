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


namespace bstone
{


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

	RendererCommandFog* write_fog() override;
	RendererCommandFogColor* write_fog_color() override;
	RendererCommandFogDistances* write_fog_distances() override;

	RendererCommandMatrixModel* write_matrix_model() override;
	RendererCommandMatrixView* write_matrix_view() override;
	RendererCommandMatrixModelView* write_matrix_model_view() override;
	RendererCommandMatrixProjection* write_matrix_projection() override;

	RendererCommandTexture* write_texture() override;
	RendererCommandSampler* write_sampler() override;

	RendererCommandVertexInput* write_vertex_input() override;

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

	const RendererCommandFog* read_fog() override;
	const RendererCommandFogColor* read_fog_color() override;
	const RendererCommandFogDistances* read_fog_distances() override;

	const RendererCommandMatrixModel* read_matrix_model() override;
	const RendererCommandMatrixView* read_matrix_view() override;
	const RendererCommandMatrixModelView* read_matrix_model_view() override;
	const RendererCommandMatrixProjection* read_matrix_projection() override;

	const RendererCommandTexture* read_texture() override;
	const RendererCommandSampler* read_sampler() override;

	const RendererCommandVertexInput* read_vertex_input() override;

	const RendererCommandDrawQuads* read_draw_quads() override;


	void initialize(
		const RendererCommandManagerBufferAddParam& param);


private:
	static constexpr auto min_initial_size = 4096;
	static constexpr auto min_resize_delta_size = 4096;
	static constexpr auto command_id_size = static_cast<int>(sizeof(RendererCommandId));


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

		const auto block_size = command_id_size + command_size;

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
}; // RendererCommandBuffer

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

RendererCommandFog* RendererCommandBufferImpl::write_fog()
{
	return write<RendererCommandFog>(RendererCommandId::fog_enable);
}

RendererCommandFogColor* RendererCommandBufferImpl::write_fog_color()
{
	return write<RendererCommandFogColor>(RendererCommandId::fog_set_color);
}

RendererCommandFogDistances* RendererCommandBufferImpl::write_fog_distances()
{
	return write<RendererCommandFogDistances>(RendererCommandId::fog_set_distances);
}

RendererCommandMatrixModel* RendererCommandBufferImpl::write_matrix_model()
{
	return write<RendererCommandMatrixModel>(RendererCommandId::matrix_set_model);
}

RendererCommandMatrixView* RendererCommandBufferImpl::write_matrix_view()
{
	return write<RendererCommandMatrixView>(RendererCommandId::matrix_set_view);
}

RendererCommandMatrixModelView* RendererCommandBufferImpl::write_matrix_model_view()
{
	return write<RendererCommandMatrixModelView>(RendererCommandId::matrix_set_model_view);
}

RendererCommandMatrixProjection* RendererCommandBufferImpl::write_matrix_projection()
{
	return write<RendererCommandMatrixProjection>(RendererCommandId::matrix_set_projection);
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

const RendererCommandFog* RendererCommandBufferImpl::read_fog()
{
	return read<RendererCommandFog>();
}

const RendererCommandFogColor* RendererCommandBufferImpl::read_fog_color()
{
	return read<RendererCommandFogColor>();
}

const RendererCommandFogDistances* RendererCommandBufferImpl::read_fog_distances()
{
	return read<RendererCommandFogDistances>();
}

const RendererCommandMatrixModel* RendererCommandBufferImpl::read_matrix_model()
{
	return read<RendererCommandMatrixModel>();
}

const RendererCommandMatrixView* RendererCommandBufferImpl::read_matrix_view()
{
	return read<RendererCommandMatrixView>();
}

const RendererCommandMatrixModelView* RendererCommandBufferImpl::read_matrix_model_view()
{
	return read<RendererCommandMatrixModelView>();
}

const RendererCommandMatrixProjection* RendererCommandBufferImpl::read_matrix_projection()
{
	return read<RendererCommandMatrixProjection>();
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

const RendererCommandDrawQuads* RendererCommandBufferImpl::read_draw_quads()
{
	return read<RendererCommandDrawQuads>();
}

void RendererCommandBufferImpl::initialize(
	const RendererCommandManagerBufferAddParam& param)
{
	is_reading_ = false;
	is_writing_ = false;

	size_ = std::max(param.initial_size_, min_initial_size);
	write_offset_ = 0;
	read_offset_ = 0;
	resize_delta_size_ = std::max(param.resize_delta_size_, min_resize_delta_size);

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
