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
// OpenGL state (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_state.h"

#include <list>
#include <vector>

#include "bstone_exception.h"
#include "bstone_ogl_unique_resources.h"

#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_ogl_buffer_manager.h"
#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler.h"
#include "bstone_detail_ogl_texture_2d.h"
#include "bstone_detail_ogl_vao_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglState
//

OglState::OglState() = default;

OglState::~OglState() = default;

//
// OglState
// ==========================================================================


// ==========================================================================
// GenericOglState
//

class GenericOglState :
	public OglState
{
public:
	GenericOglState(
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);

	GenericOglState(
		const GenericOglState& rhs) = delete;

	~GenericOglState() override;


	RendererIndexBufferPtr index_buffer_create(
		const RendererIndexBufferCreateParam& param) override;

	RendererVertexBufferPtr vertex_buffer_create(
		const RendererVertexBufferCreateParam& param) override;


	void buffer_destroy(
		const RendererBufferPtr buffer) override;

	bool buffer_set_current(
		const RendererBufferKind buffer_kind,
		const RendererBufferPtr buffer) override;


	OglSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) override;

	void sampler_destroy(
		const RendererSamplerPtr sampler) override;

	void sampler_set(
		const RendererSamplerPtr sampler) override;


	void texture_2d_enable(
		const bool is_enable) override;

	RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) override;

	void texture_2d_destroy(
		const RendererTexture2dPtr texture_2d) override;

	void texture_2d_set(
		const RendererTexture2dPtr texture_2d) override;

	OglTexture2dPtr texture_2d_get_current() noexcept override;


	OglVaoPtr vao_create() override;

	void vao_destroy(
		const OglVaoPtr vao) override;

	void vao_bind(
		const OglVaoPtr vao) override;

	void vao_push_current_set_default() override;

	void vao_pop() override;


	OglVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) override;

	void vertex_input_destroy(
		const RendererVertexInputPtr vertex_input) override;

	void vertex_input_set(
		const OglVertexInputPtr vertex_input) override;

	OglVertexInputPtr vertex_input_get_current() const noexcept override;


	void vertex_input_location_enable(
		const int location,
		const bool is_enabled) override;

	void vertex_input_location_assign_begin() override;

	void vertex_input_location_assign_end() override;


private:
	const RendererDeviceFeatures& device_features_;
	const OglDeviceFeatures& ogl_device_features_;

	OglVaoManagerUPtr vao_manager_;

	OglBufferManagerUPtr buffer_manager_;


	using Samplers = std::list<OglSamplerUPtr>;
	OglSamplerPtr sampler_current_;
	OglSamplerUPtr sampler_default_;
	Samplers samplers_;

	using Textures2d = std::list<OglTexture2dUPtr>;
	bool texture_2d_is_enabled_;
	OglTexture2dPtr texture_2d_current_;
	Textures2d textures_2d_;

	using VertexInputs = std::list<OglVertexInputUPtr>;
	OglVertexInputPtr vertex_input_current_;
	VertexInputs vertex_inputs_;

	bool vertex_input_location_is_assigning_;
	using VertexInputAssignedLocations = std::vector<bool>;
	VertexInputAssignedLocations vertex_input_assigned_locations_;


	void initialize();

	void initialize_vao_manager();

	void initialize_buffer_manager();

	void mipmap_set_max_quality();

	void sampler_create_default();

	void sampler_set_defaults();

	void sampler_sw_force_update_current_texture();

	void texture_2d_enable();

	void texture_2d_set();

	void texture_2d_set_defaults();

	void vertex_input_set_defaults();

	void vertex_input_set();

	void vertex_input_location_set_defaults();
}; // GenericOglState

using GenericOglStatePtr = GenericOglState*;
using GenericOglStateUPtr = std::unique_ptr<GenericOglState>;

//
// GenericOglState
// ==========================================================================


// ==========================================================================
// GenericOglState
//

GenericOglState::GenericOglState(
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
	:
	device_features_{device_features},
	ogl_device_features_{ogl_device_features},
	vao_manager_{},
	buffer_manager_{},
	sampler_current_{},
	sampler_default_{},
	samplers_{},
	texture_2d_is_enabled_{},
	texture_2d_current_{},
	textures_2d_{},
	vertex_input_current_{},
	vertex_inputs_{},
	vertex_input_location_is_assigning_{},
	vertex_input_assigned_locations_{}
{
	initialize();
}

GenericOglState::~GenericOglState() = default;

void GenericOglState::initialize_buffer_manager()
{
	buffer_manager_ = OglBufferManagerFactory::create(
		this,
		vao_manager_.get()
	);
}

void GenericOglState::initialize_vao_manager()
{
	vao_manager_ = OglVaoManagerFactory::create(
		this,
		device_features_,
		ogl_device_features_
	);
}

void GenericOglState::mipmap_set_max_quality()
{
	if (!device_features_.mipmap_is_available_)
	{
		return;
	}

	if (ogl_device_features_.context_kind_ == OglContextKind::core)
	{
		return;
	}

	::glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglState::sampler_create_default()
{
	const auto param = RendererSamplerCreateParam{};

	sampler_default_ = OglSamplerFactory::create(this, device_features_, ogl_device_features_, param);
}

void GenericOglState::sampler_set_defaults()
{
	if (device_features_.sampler_is_available_)
	{
		return;
	}

	sampler_create_default();
	sampler_current_ = sampler_default_.get();
}

void GenericOglState::texture_2d_enable()
{
	const auto ogl_function = (texture_2d_is_enabled_ ? ::glEnable : ::glDisable);

	ogl_function(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglState::texture_2d_set()
{
	if (texture_2d_current_)
	{
		texture_2d_current_->bind();
	}
	else
	{
		OglRendererUtils::texture_2d_unbind();
	}

	if (!device_features_.sampler_is_available_)
	{
		sampler_sw_force_update_current_texture();
	}
}

void GenericOglState::texture_2d_set_defaults()
{
	texture_2d_is_enabled_ = false;
	texture_2d_enable();

	texture_2d_current_ = nullptr;
	texture_2d_set();
}

void GenericOglState::initialize()
{
	mipmap_set_max_quality();
	sampler_set_defaults();
	texture_2d_set_defaults();
	initialize_vao_manager();
	initialize_buffer_manager();
	vertex_input_set_defaults();
	vertex_input_location_set_defaults();

	::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	assert(!detail::OglRendererUtils::was_errors());

	::glBindBuffer(GL_ARRAY_BUFFER, 0);
	assert(!detail::OglRendererUtils::was_errors());
}

RendererIndexBufferPtr GenericOglState::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	return buffer_manager_->index_buffer_create(param);
}

RendererVertexBufferPtr GenericOglState::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	return buffer_manager_->vertex_buffer_create(param);
}

void GenericOglState::buffer_destroy(
	const RendererBufferPtr buffer)
{
	buffer_manager_->buffer_destroy(buffer);
}

bool GenericOglState::buffer_set_current(
	const RendererBufferKind buffer_kind,
	const RendererBufferPtr buffer)
{
	return buffer_manager_->buffer_set_current(buffer_kind, buffer);
}

void GenericOglState::vertex_input_set_defaults()
{
	vertex_input_current_ = nullptr;
	vertex_input_set();
}

void GenericOglState::vertex_input_set()
{
	if (!vertex_input_current_)
	{
		return;
	}

	vertex_input_current_->bind();
}

void GenericOglState::vertex_input_location_set_defaults()
{
	vertex_input_assigned_locations_.resize(device_features_.vertex_input_max_locations_);

	for (int i = 0; i < device_features_.vertex_input_max_locations_; ++i)
	{
		vertex_input_location_enable(i, false);
	}
}

OglSamplerPtr GenericOglState::sampler_create(
	const RendererSamplerCreateParam& param)
{
	auto sampler = OglSamplerFactory::create(this, device_features_, ogl_device_features_, param);

	samplers_.emplace_back(std::move(sampler));

	return samplers_.back().get();
}

void GenericOglState::sampler_destroy(
	const RendererSamplerPtr sampler)
{
	if (!sampler)
	{
		throw Exception{"Null sampler."};
	}

	if (sampler_current_ == sampler)
	{
		if (device_features_.sampler_is_available_)
		{
			sampler_current_->unbind_unit();
			sampler_current_ = nullptr;
		}
		else
		{
			sampler_current_ = sampler_default_.get();
		}
	}

	samplers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == sampler;
		}
	);
}

void GenericOglState::sampler_set(
	const RendererSamplerPtr sampler)
{
	if (!sampler)
	{
		throw Exception{"Null sampler."};
	}

	if (sampler_current_ == sampler)
	{
		return;
	}

	sampler_current_ = static_cast<OglSamplerPtr>(sampler);

	if (device_features_.sampler_is_available_)
	{
		sampler_current_->bind();
	}
	else
	{
		sampler_sw_force_update_current_texture();
	}
}

void GenericOglState::sampler_sw_force_update_current_texture()
{
	if (!texture_2d_current_)
	{
		return;
	}

	texture_2d_current_->update_sampler_state(sampler_current_->get_state());
}

void GenericOglState::texture_2d_enable(
	const bool is_enable)
{
	if (texture_2d_is_enabled_ == is_enable)
	{
		return;
	}

	texture_2d_is_enabled_ = is_enable;
	texture_2d_enable();
}

RendererTexture2dPtr GenericOglState::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	auto texture_2d = OglTexture2dFactory::create(this, device_features_, ogl_device_features_, param);

	textures_2d_.emplace_back(std::move(texture_2d));

	return textures_2d_.back().get();
}

void GenericOglState::texture_2d_destroy(
	const RendererTexture2dPtr texture_2d)
{
	if (!texture_2d)
	{
		throw Exception{"Null texture."};
	}

	textures_2d_.remove_if(
		[=](const auto& item)
		{
			return item.get() == texture_2d;
		}
	);

	if (texture_2d_current_ == texture_2d)
	{
		texture_2d_current_ = nullptr;
	}
}

void GenericOglState::texture_2d_set(
	const RendererTexture2dPtr texture_2d)
{
	if (texture_2d_current_ == texture_2d)
	{
		return;
	}

	texture_2d_current_ = static_cast<OglTexture2dPtr>(texture_2d);
	texture_2d_set();
}

OglTexture2dPtr GenericOglState::texture_2d_get_current() noexcept
{
	return texture_2d_current_;
}

OglVaoPtr GenericOglState::vao_create()
{
	return vao_manager_->create();
}

void GenericOglState::vao_destroy(
	const OglVaoPtr vao)
{
	vao_manager_->destroy(vao);
}

void GenericOglState::vao_bind(
	const OglVaoPtr vao)
{
	if (!vao)
	{
		throw Exception{"Null VAO."};
	}

	vao_manager_->bind(vao);
}

void GenericOglState::vao_push_current_set_default()
{
	vao_manager_->push_current_set_default();
}

void GenericOglState::vao_pop()
{
	vao_manager_->pop();
}

OglVertexInputPtr GenericOglState::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	auto vertex_input = OglVertexInputFactory::create(
		this,
		device_features_,
		ogl_device_features_,
		param
	);

	vertex_inputs_.emplace_back(std::move(vertex_input));

	return vertex_inputs_.back().get();
}

void GenericOglState::vertex_input_destroy(
	const RendererVertexInputPtr vertex_input)
{
	if (!vertex_input)
	{
		throw Exception{"Null vertex input."};
	}

	vertex_inputs_.remove_if(
		[=](const auto& item)
		{
			return item.get() == vertex_input;
		}
	);

	if (vertex_input_current_ == vertex_input)
	{
		vertex_input_current_ = nullptr;
	}
}

void GenericOglState::vertex_input_set(
	const OglVertexInputPtr vertex_input)
{
	if (vertex_input_current_ == vertex_input)
	{
		return;
	}

	vertex_input_current_ = vertex_input;
	vertex_input_set();
}

OglVertexInputPtr GenericOglState::vertex_input_get_current() const noexcept
{
	return vertex_input_current_;
}

void GenericOglState::vertex_input_location_enable(
	const int location,
	const bool is_enabled)
{
	if (vertex_input_location_is_assigning_)
	{
		vertex_input_assigned_locations_[location] = true;

		return;
	}

	vao_manager_->enable_location(location, is_enabled);
}

void GenericOglState::vertex_input_location_assign_begin()
{
	if (vertex_input_location_is_assigning_)
	{
		throw Exception{"Already assigning."};
	}

	vertex_input_location_is_assigning_ = true;

	std::fill(
		vertex_input_assigned_locations_.begin(),
		vertex_input_assigned_locations_.end(),
		false
	);
}

void GenericOglState::vertex_input_location_assign_end()
{
	if (!vertex_input_location_is_assigning_)
	{
		throw Exception{"Not assigning."};
	}

	vertex_input_location_is_assigning_ = false;

	for (int i = 0; i < device_features_.vertex_input_max_locations_; ++i)
	{
		vertex_input_location_enable(i, vertex_input_assigned_locations_[i]);
	}
}

//
// GenericOglState
// ==========================================================================


// =========================================================================
// OglStateFactory
//

OglStateUPtr OglStateFactory::create(
	const RendererKind renderer_kind,
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
			return GenericOglStateUPtr{new GenericOglState{device_features, ogl_device_features}};

		default:
			throw Exception{"Unsupported renderer kind."};
	}
}

//
// OglStateFactory
// =========================================================================


} // detail
} // bstone
