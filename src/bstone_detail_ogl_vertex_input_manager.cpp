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
// OpenGL vertex input manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_vertex_input_manager.h"

#include "bstone_exception.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_state.h"
#include "bstone_detail_ogl_vertex_input.h"
#include "bstone_detail_ogl_vao_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglVertexInputManager
//

OglVertexInputManager::OglVertexInputManager() = default;

OglVertexInputManager::~OglVertexInputManager() = default;

//
// OglVertexInputManager
// ==========================================================================


// ==========================================================================
// GenericOglVertexInputManager
//

class GenericOglVertexInputManager :
	public OglVertexInputManager
{
public:
	GenericOglVertexInputManager(
		const OglStatePtr ogl_state);

	~GenericOglVertexInputManager() override;


	RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) override;

	void vertex_input_destroy(
		const RendererVertexInputPtr vertex_input) override;

	void vertex_input_set(
		const RendererVertexInputPtr vertex_input) override;

	RendererVertexInputPtr vertex_input_get_current() const noexcept override;


	void vertex_input_location_enable(
		const int location,
		const bool is_enabled) override;

	void vertex_input_location_assign_begin() override;

	void vertex_input_location_assign_end() override;


private:
	const OglStatePtr ogl_state_;


	using VertexInputs = UPtrResourceList<OglVertexInput, OglVertexInputFactory, Exception>;
	OglVertexInputPtr vertex_input_current_;
	VertexInputs vertex_inputs_;

	bool vertex_input_location_is_assigning_;
	using VertexInputAssignedLocations = std::vector<bool>;
	VertexInputAssignedLocations vertex_input_assigned_locations_;


	void initialize_vertex_input_locations();

	void initialize();

	void vertex_input_set();
}; // GenericOglVertexInputManager

using GenericOglVaoManagerPtr = GenericOglVertexInputManager*;
using GenericOglVaoManagerUPtr = std::unique_ptr<GenericOglVertexInputManager>;

//
// GenericOglVertexInputManager
// ==========================================================================


// ==========================================================================
// GenericOglVertexInputManager
//

GenericOglVertexInputManager::GenericOglVertexInputManager(
	const OglStatePtr ogl_state)
	:
	ogl_state_{ogl_state},
	vertex_input_current_{},
	vertex_inputs_{},
	vertex_input_location_is_assigning_{},
	vertex_input_assigned_locations_{}
{
	initialize();
}

GenericOglVertexInputManager::~GenericOglVertexInputManager() = default;

RendererVertexInputPtr GenericOglVertexInputManager::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	return vertex_inputs_.add(ogl_state_, param);
}

void GenericOglVertexInputManager::vertex_input_destroy(
	const RendererVertexInputPtr vertex_input)
{
	vertex_inputs_.remove(vertex_input);
}

void GenericOglVertexInputManager::vertex_input_set(
	const RendererVertexInputPtr vertex_input)
{
	if (vertex_input_current_ == vertex_input)
	{
		return;
	}

	vertex_input_current_ = static_cast<OglVertexInputPtr>(vertex_input);
	vertex_input_set();
}

RendererVertexInputPtr GenericOglVertexInputManager::vertex_input_get_current() const noexcept
{
	return vertex_input_current_;
}

void GenericOglVertexInputManager::vertex_input_location_enable(
	const int location,
	const bool is_enabled)
{
	if (vertex_input_location_is_assigning_)
	{
		vertex_input_assigned_locations_[location] = true;

		return;
	}

	const auto vao_manager = ogl_state_->vao_get_manager();

	vao_manager->enable_location(location, is_enabled);
}

void GenericOglVertexInputManager::vertex_input_location_assign_begin()
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

void GenericOglVertexInputManager::vertex_input_location_assign_end()
{
	if (!vertex_input_location_is_assigning_)
	{
		throw Exception{"Not assigning."};
	}

	vertex_input_location_is_assigning_ = false;

	const auto& device_features = ogl_state_->get_device_features();

	for (int i = 0; i < device_features.vertex_input_max_locations_; ++i)
	{
		vertex_input_location_enable(i, vertex_input_assigned_locations_[i]);
	}
}

void GenericOglVertexInputManager::initialize_vertex_input_locations()
{
	const auto& device_features = ogl_state_->get_device_features();

	vertex_input_assigned_locations_.resize(device_features.vertex_input_max_locations_);

	for (int i = 0; i < device_features.vertex_input_max_locations_; ++i)
	{
		vertex_input_location_enable(i, false);
	}
}

void GenericOglVertexInputManager::initialize()
{
	if (!ogl_state_)
	{
		throw Exception{"Null OpenGL state."};
	}

	initialize_vertex_input_locations();
}

void GenericOglVertexInputManager::vertex_input_set()
{
	if (!vertex_input_current_)
	{
		return;
	}

	vertex_input_current_->bind();
}

//
// GenericOglVertexInputManager
// ==========================================================================


// ==========================================================================
// OglVaoManagerFactory
//

OglVertexInputManagerUPtr OglVertexInputManagerFactory::create(
	const OglStatePtr ogl_state)
{
	return std::make_unique<GenericOglVertexInputManager>(ogl_state);
}

//
// OglVaoManagerFactory
// ==========================================================================


} // detail
} // bstone
