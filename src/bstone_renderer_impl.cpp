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


	void allocate_begin() override;

	void allocate_end() override;

	RendererCommand::ViewportSet* allocate_viewport_set() override;

	RendererCommand::ScissorEnable* allocate_scissor_enable() override;
	RendererCommand::ScissorSetBox* allocate_scissor_set_box() override;

	RendererCommand::CullingEnabled* allocate_culling_enable() override;

	RendererCommand::DepthSetTest* allocate_depth_set_test() override;
	RendererCommand::DepthSetWrite* allocate_depth_set_write() override;

	RendererCommand::BlendingEnable* allocate_blending_enable() override;

	RendererCommand::FogEnable* allocate_fog_enable() override;
	RendererCommand::FogSetColor* allocate_fog_set_color() override;
	RendererCommand::FogSetDistances* allocate_fog_set_distances() override;

	RendererCommand::MatrixSetModel* allocate_matrix_set_model() override;
	RendererCommand::MatrixSetView* allocate_matrix_set_view() override;
	RendererCommand::MatrixSetModelView* allocate_matrix_set_model_view() override;
	RendererCommand::MatrixSetProjection* allocate_matrix_set_projection() override;

	RendererCommand::TextureSet* allocate_texture_set() override;
	RendererCommand::SamplerSet* allocate_sampler_set() override;

	RendererCommand::VertexInputSet* allocate_vertex_input_set() override;

	RendererCommand::DrawQuads* allocate_draw_quads() override;


	void read_begin() override;

	void read_end() override;

	RendererCommandId read_command_id() override;

	const RendererCommand::ViewportSet* read_viewport_set() override;

	const RendererCommand::ScissorEnable* read_scissor_enable() override;
	const RendererCommand::ScissorSetBox* read_scissor_set_box() override;

	const RendererCommand::CullingEnabled* read_culling_enable() override;

	const RendererCommand::DepthSetTest* read_depth_set_test() override;
	const RendererCommand::DepthSetWrite* read_depth_set_write() override;

	const RendererCommand::BlendingEnable* read_blending_enable() override;

	const RendererCommand::FogEnable* read_fog_enable() override;
	const RendererCommand::FogSetColor* read_fog_set_color() override;
	const RendererCommand::FogSetDistances* read_fog_set_distances() override;

	const RendererCommand::MatrixSetModel* read_matrix_set_model() override;
	const RendererCommand::MatrixSetView* read_matrix_set_view() override;
	const RendererCommand::MatrixSetModelView* read_matrix_set_model_view() override;
	const RendererCommand::MatrixSetProjection* read_matrix_set_projection() override;

	const RendererCommand::TextureSet* read_texture_set() override;
	const RendererCommand::SamplerSet* read_sampler_set() override;

	const RendererCommand::VertexInputSet* read_vertex_input_set() override;

	const RendererCommand::DrawQuads* read_draw_quads() override;


	void initialize(
		const RendererCommandManagerBufferAddParam& param);


private:
	static constexpr auto min_initial_size = 4096;
	static constexpr auto min_resize_delta_size = 4096;
	static constexpr auto command_id_size = static_cast<int>(sizeof(RendererCommandId));


	using Data = std::vector<std::uint8_t>;

	bool is_enabled_;
	bool is_reading_;
	bool is_allocating_;

	int size_;
	int allocate_offset_;
	int read_offset_;
	int resize_delta_size_;
	int command_count_;

	Data data_;


	void resize_if_necessary(
		const int dst_delta_size);

	template<typename T>
	T* allocate(
		const RendererCommandId command_id)
	{
		if (is_reading_ || !is_allocating_)
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

		auto block = reinterpret_cast<RendererCommandId*>(&data_[allocate_offset_]);
		*block = command_id;

		allocate_offset_ += block_size;
		++command_count_;

		return reinterpret_cast<T*>(block + 1);
	}

	template<typename T>
	const T* read()
	{
		if (!is_reading_ || is_allocating_)
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
	is_allocating_{},
	size_{},
	allocate_offset_{},
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
	is_allocating_{std::move(rhs.is_allocating_)},
	size_{std::move(rhs.size_)},
	allocate_offset_{std::move(rhs.allocate_offset_)},
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

void RendererCommandBufferImpl::allocate_begin()
{
	if (is_reading_ || is_allocating_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_allocating_ = true;
	allocate_offset_ = 0;
	command_count_ = 0;
}

void RendererCommandBufferImpl::allocate_end()
{
	if (is_reading_ || !is_allocating_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_allocating_ = false;
}

RendererCommand::ViewportSet* RendererCommandBufferImpl::allocate_viewport_set()
{
	return allocate<RendererCommand::ViewportSet>(RendererCommandId::viewport_set);
}

RendererCommand::ScissorEnable* RendererCommandBufferImpl::allocate_scissor_enable()
{
	return allocate<RendererCommand::ScissorEnable>(RendererCommandId::scissor_enable);
}

RendererCommand::ScissorSetBox* RendererCommandBufferImpl::allocate_scissor_set_box()
{
	return allocate<RendererCommand::ScissorSetBox>(RendererCommandId::scissor_set_box);
}

RendererCommand::CullingEnabled* RendererCommandBufferImpl::allocate_culling_enable()
{
	return allocate<RendererCommand::CullingEnabled>(RendererCommandId::culling_enable);
}

RendererCommand::DepthSetTest* RendererCommandBufferImpl::allocate_depth_set_test()
{
	return allocate<RendererCommand::DepthSetTest>(RendererCommandId::depth_set_test);
}

RendererCommand::DepthSetWrite* RendererCommandBufferImpl::allocate_depth_set_write()
{
	return allocate<RendererCommand::DepthSetWrite>(RendererCommandId::depth_set_write);
}

RendererCommand::BlendingEnable* RendererCommandBufferImpl::allocate_blending_enable()
{
	return allocate<RendererCommand::BlendingEnable>(RendererCommandId::blending_enable);
}

RendererCommand::FogEnable* RendererCommandBufferImpl::allocate_fog_enable()
{
	return allocate<RendererCommand::FogEnable>(RendererCommandId::fog_enable);
}

RendererCommand::FogSetColor* RendererCommandBufferImpl::allocate_fog_set_color()
{
	return allocate<RendererCommand::FogSetColor>(RendererCommandId::fog_set_color);
}

RendererCommand::FogSetDistances* RendererCommandBufferImpl::allocate_fog_set_distances()
{
	return allocate<RendererCommand::FogSetDistances>(RendererCommandId::fog_set_distances);
}

RendererCommand::MatrixSetModel* RendererCommandBufferImpl::allocate_matrix_set_model()
{
	return allocate<RendererCommand::MatrixSetModel>(RendererCommandId::matrix_set_model);
}

RendererCommand::MatrixSetView* RendererCommandBufferImpl::allocate_matrix_set_view()
{
	return allocate<RendererCommand::MatrixSetView>(RendererCommandId::matrix_set_view);
}

RendererCommand::MatrixSetModelView* RendererCommandBufferImpl::allocate_matrix_set_model_view()
{
	return allocate<RendererCommand::MatrixSetModelView>(RendererCommandId::matrix_set_model_view);
}

RendererCommand::MatrixSetProjection* RendererCommandBufferImpl::allocate_matrix_set_projection()
{
	return allocate<RendererCommand::MatrixSetProjection>(RendererCommandId::matrix_set_projection);
}

RendererCommand::TextureSet* RendererCommandBufferImpl::allocate_texture_set()
{
	return allocate<RendererCommand::TextureSet>(RendererCommandId::texture_set);
}

RendererCommand::SamplerSet* RendererCommandBufferImpl::allocate_sampler_set()
{
	return allocate<RendererCommand::SamplerSet>(RendererCommandId::sampler_set);
}

RendererCommand::VertexInputSet* RendererCommandBufferImpl::allocate_vertex_input_set()
{
	return allocate<RendererCommand::VertexInputSet>(RendererCommandId::vertex_input_set);
}

RendererCommand::DrawQuads* RendererCommandBufferImpl::allocate_draw_quads()
{
	return allocate<RendererCommand::DrawQuads>(RendererCommandId::draw_quads);
}

void RendererCommandBufferImpl::read_begin()
{
	if (is_reading_ || is_allocating_)
	{
		assert(!"Invalid state.");

		return;
	}

	is_reading_ = true;
	read_offset_ = 0;
}

void RendererCommandBufferImpl::read_end()
{
	if (!is_reading_ || is_allocating_)
	{
		assert(!"Invalid state.");

		return;
	}

	assert(allocate_offset_ == read_offset_);

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

const RendererCommand::ViewportSet* RendererCommandBufferImpl::read_viewport_set()
{
	return read<RendererCommand::ViewportSet>();
}

const RendererCommand::ScissorEnable* RendererCommandBufferImpl::read_scissor_enable()
{
	return read<RendererCommand::ScissorEnable>();
}

const RendererCommand::ScissorSetBox* RendererCommandBufferImpl::read_scissor_set_box()
{
	return read<RendererCommand::ScissorSetBox>();
}

const RendererCommand::CullingEnabled* RendererCommandBufferImpl::read_culling_enable()
{
	return read<RendererCommand::CullingEnabled>();
}

const RendererCommand::DepthSetTest* RendererCommandBufferImpl::read_depth_set_test()
{
	return read<RendererCommand::DepthSetTest>();
}

const RendererCommand::DepthSetWrite* RendererCommandBufferImpl::read_depth_set_write()
{
	return read<RendererCommand::DepthSetWrite>();
}

const RendererCommand::BlendingEnable* RendererCommandBufferImpl::read_blending_enable()
{
	return read<RendererCommand::BlendingEnable>();
}

const RendererCommand::FogEnable* RendererCommandBufferImpl::read_fog_enable()
{
	return read<RendererCommand::FogEnable>();
}

const RendererCommand::FogSetColor* RendererCommandBufferImpl::read_fog_set_color()
{
	return read<RendererCommand::FogSetColor>();
}

const RendererCommand::FogSetDistances* RendererCommandBufferImpl::read_fog_set_distances()
{
	return read<RendererCommand::FogSetDistances>();
}

const RendererCommand::MatrixSetModel* RendererCommandBufferImpl::read_matrix_set_model()
{
	return read<RendererCommand::MatrixSetModel>();
}

const RendererCommand::MatrixSetView* RendererCommandBufferImpl::read_matrix_set_view()
{
	return read<RendererCommand::MatrixSetView>();
}

const RendererCommand::MatrixSetModelView* RendererCommandBufferImpl::read_matrix_set_model_view()
{
	return read<RendererCommand::MatrixSetModelView>();
}

const RendererCommand::MatrixSetProjection* RendererCommandBufferImpl::read_matrix_set_projection()
{
	return read<RendererCommand::MatrixSetProjection>();
}

const RendererCommand::TextureSet* RendererCommandBufferImpl::read_texture_set()
{
	return read<RendererCommand::TextureSet>();
}

const RendererCommand::SamplerSet* RendererCommandBufferImpl::read_sampler_set()
{
	return read<RendererCommand::SamplerSet>();
}

const RendererCommand::VertexInputSet* RendererCommandBufferImpl::read_vertex_input_set()
{
	return read<RendererCommand::VertexInputSet>();
}

const RendererCommand::DrawQuads* RendererCommandBufferImpl::read_draw_quads()
{
	return read<RendererCommand::DrawQuads>();
}

void RendererCommandBufferImpl::initialize(
	const RendererCommandManagerBufferAddParam& param)
{
	is_reading_ = false;
	is_allocating_ = false;

	size_ = std::max(param.initial_size_, min_initial_size);
	allocate_offset_ = 0;
	read_offset_ = 0;
	resize_delta_size_ = std::max(param.resize_delta_size_, min_resize_delta_size);

	data_.resize(size_);
}

void RendererCommandBufferImpl::resize_if_necessary(
	const int dst_delta_size)
{
	assert(dst_delta_size > 0);

	if ((size_ - allocate_offset_) >= dst_delta_size)
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
