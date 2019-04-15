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

	RendererCommandViewportSet* write_viewport_set() override;

	RendererCommandScissorEnable* write_scissor_enable() override;
	RendererCommandScissorSetBox* write_scissor_set_box() override;

	RendererCommandCullingEnabled* write_culling_enable() override;

	RendererCommandDepthSetTest* write_depth_set_test() override;
	RendererCommandDepthSetWrite* write_depth_set_write() override;

	RendererCommandBlendingEnable* write_blending_enable() override;

	RendererCommandFogEnable* write_fog_enable() override;
	RendererCommandFogSetColor* write_fog_set_color() override;
	RendererCommandFogSetDistances* write_fog_set_distances() override;

	RendererCommandMatrixSetModel* write_matrix_set_model() override;
	RendererCommandMatrixSetView* write_matrix_set_view() override;
	RendererCommandMatrixSetModelView* write_matrix_set_model_view() override;
	RendererCommandMatrixSetProjection* write_matrix_set_projection() override;

	RendererCommandTextureSet* write_texture_set() override;
	RendererCommandSamplerSet* write_sampler_set() override;

	RendererCommandVertexInputSet* write_vertex_input_set() override;

	RendererCommandDrawQuads* write_draw_quads() override;


	void read_begin() override;

	void read_end() override;

	RendererCommandId read_command_id() override;

	const RendererCommandViewportSet* read_viewport_set() override;

	const RendererCommandScissorEnable* read_scissor_enable() override;
	const RendererCommandScissorSetBox* read_scissor_set_box() override;

	const RendererCommandCullingEnabled* read_culling_enable() override;

	const RendererCommandDepthSetTest* read_depth_set_test() override;
	const RendererCommandDepthSetWrite* read_depth_set_write() override;

	const RendererCommandBlendingEnable* read_blending_enable() override;

	const RendererCommandFogEnable* read_fog_enable() override;
	const RendererCommandFogSetColor* read_fog_set_color() override;
	const RendererCommandFogSetDistances* read_fog_set_distances() override;

	const RendererCommandMatrixSetModel* read_matrix_set_model() override;
	const RendererCommandMatrixSetView* read_matrix_set_view() override;
	const RendererCommandMatrixSetModelView* read_matrix_set_model_view() override;
	const RendererCommandMatrixSetProjection* read_matrix_set_projection() override;

	const RendererCommandTextureSet* read_texture_set() override;
	const RendererCommandSamplerSet* read_sampler_set() override;

	const RendererCommandVertexInputSet* read_vertex_input_set() override;

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

RendererCommandViewportSet* RendererCommandBufferImpl::write_viewport_set()
{
	return write<RendererCommandViewportSet>(RendererCommandId::viewport_set);
}

RendererCommandScissorEnable* RendererCommandBufferImpl::write_scissor_enable()
{
	return write<RendererCommandScissorEnable>(RendererCommandId::scissor_enable);
}

RendererCommandScissorSetBox* RendererCommandBufferImpl::write_scissor_set_box()
{
	return write<RendererCommandScissorSetBox>(RendererCommandId::scissor_set_box);
}

RendererCommandCullingEnabled* RendererCommandBufferImpl::write_culling_enable()
{
	return write<RendererCommandCullingEnabled>(RendererCommandId::culling_enable);
}

RendererCommandDepthSetTest* RendererCommandBufferImpl::write_depth_set_test()
{
	return write<RendererCommandDepthSetTest>(RendererCommandId::depth_set_test);
}

RendererCommandDepthSetWrite* RendererCommandBufferImpl::write_depth_set_write()
{
	return write<RendererCommandDepthSetWrite>(RendererCommandId::depth_set_write);
}

RendererCommandBlendingEnable* RendererCommandBufferImpl::write_blending_enable()
{
	return write<RendererCommandBlendingEnable>(RendererCommandId::blending_enable);
}

RendererCommandFogEnable* RendererCommandBufferImpl::write_fog_enable()
{
	return write<RendererCommandFogEnable>(RendererCommandId::fog_enable);
}

RendererCommandFogSetColor* RendererCommandBufferImpl::write_fog_set_color()
{
	return write<RendererCommandFogSetColor>(RendererCommandId::fog_set_color);
}

RendererCommandFogSetDistances* RendererCommandBufferImpl::write_fog_set_distances()
{
	return write<RendererCommandFogSetDistances>(RendererCommandId::fog_set_distances);
}

RendererCommandMatrixSetModel* RendererCommandBufferImpl::write_matrix_set_model()
{
	return write<RendererCommandMatrixSetModel>(RendererCommandId::matrix_set_model);
}

RendererCommandMatrixSetView* RendererCommandBufferImpl::write_matrix_set_view()
{
	return write<RendererCommandMatrixSetView>(RendererCommandId::matrix_set_view);
}

RendererCommandMatrixSetModelView* RendererCommandBufferImpl::write_matrix_set_model_view()
{
	return write<RendererCommandMatrixSetModelView>(RendererCommandId::matrix_set_model_view);
}

RendererCommandMatrixSetProjection* RendererCommandBufferImpl::write_matrix_set_projection()
{
	return write<RendererCommandMatrixSetProjection>(RendererCommandId::matrix_set_projection);
}

RendererCommandTextureSet* RendererCommandBufferImpl::write_texture_set()
{
	return write<RendererCommandTextureSet>(RendererCommandId::texture_set);
}

RendererCommandSamplerSet* RendererCommandBufferImpl::write_sampler_set()
{
	return write<RendererCommandSamplerSet>(RendererCommandId::sampler_set);
}

RendererCommandVertexInputSet* RendererCommandBufferImpl::write_vertex_input_set()
{
	return write<RendererCommandVertexInputSet>(RendererCommandId::vertex_input_set);
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

const RendererCommandViewportSet* RendererCommandBufferImpl::read_viewport_set()
{
	return read<RendererCommandViewportSet>();
}

const RendererCommandScissorEnable* RendererCommandBufferImpl::read_scissor_enable()
{
	return read<RendererCommandScissorEnable>();
}

const RendererCommandScissorSetBox* RendererCommandBufferImpl::read_scissor_set_box()
{
	return read<RendererCommandScissorSetBox>();
}

const RendererCommandCullingEnabled* RendererCommandBufferImpl::read_culling_enable()
{
	return read<RendererCommandCullingEnabled>();
}

const RendererCommandDepthSetTest* RendererCommandBufferImpl::read_depth_set_test()
{
	return read<RendererCommandDepthSetTest>();
}

const RendererCommandDepthSetWrite* RendererCommandBufferImpl::read_depth_set_write()
{
	return read<RendererCommandDepthSetWrite>();
}

const RendererCommandBlendingEnable* RendererCommandBufferImpl::read_blending_enable()
{
	return read<RendererCommandBlendingEnable>();
}

const RendererCommandFogEnable* RendererCommandBufferImpl::read_fog_enable()
{
	return read<RendererCommandFogEnable>();
}

const RendererCommandFogSetColor* RendererCommandBufferImpl::read_fog_set_color()
{
	return read<RendererCommandFogSetColor>();
}

const RendererCommandFogSetDistances* RendererCommandBufferImpl::read_fog_set_distances()
{
	return read<RendererCommandFogSetDistances>();
}

const RendererCommandMatrixSetModel* RendererCommandBufferImpl::read_matrix_set_model()
{
	return read<RendererCommandMatrixSetModel>();
}

const RendererCommandMatrixSetView* RendererCommandBufferImpl::read_matrix_set_view()
{
	return read<RendererCommandMatrixSetView>();
}

const RendererCommandMatrixSetModelView* RendererCommandBufferImpl::read_matrix_set_model_view()
{
	return read<RendererCommandMatrixSetModelView>();
}

const RendererCommandMatrixSetProjection* RendererCommandBufferImpl::read_matrix_set_projection()
{
	return read<RendererCommandMatrixSetProjection>();
}

const RendererCommandTextureSet* RendererCommandBufferImpl::read_texture_set()
{
	return read<RendererCommandTextureSet>();
}

const RendererCommandSamplerSet* RendererCommandBufferImpl::read_sampler_set()
{
	return read<RendererCommandSamplerSet>();
}

const RendererCommandVertexInputSet* RendererCommandBufferImpl::read_vertex_input_set()
{
	return read<RendererCommandVertexInputSet>();
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
