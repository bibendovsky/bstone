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
// OpenGL vertex array object (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_vao.h"

#include "bstone_exception.h"
#include "bstone_ogl_api.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ogl_index_buffer.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_vao_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglVao
//

OglVao::OglVao() = default;

OglVao::~OglVao() = default;

//
// OglVao
// ==========================================================================


// ==========================================================================
// GenericOglVao
//

class GenericOglVao :
	public OglVao
{
public:
	GenericOglVao(
		const OglVaoManagerPtr manager);

	~GenericOglVao() override;


	void bind() override;


	bool index_buffer_set_current(
		const RendererBufferPtr index_buffer) override;

	void enable_location(
		const int location,
		const bool is_enable) override;


private:
	const OglVaoManagerPtr manager_;


	static void ogl_deleter(
		const GLuint& ogl_name) noexcept;

	using VaoResource = UniqueResource<GLuint, ogl_deleter>;
	VaoResource vao_resource_;

	RendererBufferPtr index_buffer_;


	using EnabledLocations = std::vector<bool>;
	EnabledLocations enabled_locations_;


	void enable_location(
		const int location);

	void initialize_resource();

	void initialize_locations();

	void initialize();
}; // GenericOglVao

using GenericOglVaoPtr = GenericOglVao*;
using GenericOglVaoUPtr = std::unique_ptr<GenericOglVao>;

//
// GenericOglVao
// ==========================================================================


// ==========================================================================
// GenericOglVao
//

GenericOglVao::GenericOglVao(
	const OglVaoManagerPtr manager)
	:
	manager_{manager},
	vao_resource_{},
	index_buffer_{},
	enabled_locations_{}
{
	initialize();
}

GenericOglVao::~GenericOglVao() = default;

void GenericOglVao::bind()
{
	if (!vao_resource_)
	{
		return;
	}

	::glBindVertexArray(vao_resource_);
	assert(!OglRendererUtils::was_errors());
}

bool GenericOglVao::index_buffer_set_current(
	const RendererBufferPtr index_buffer)
{
	if (index_buffer_ == index_buffer)
	{
		return false;
	}

	index_buffer_ = index_buffer;

	return true;
}

void GenericOglVao::enable_location(
	const int location,
	const bool is_enable)
{
	if (enabled_locations_[location] == is_enable)
	{
		return;
	}

	enabled_locations_[location] = is_enable;
	enable_location(location);
}

void GenericOglVao::ogl_deleter(
	const GLuint& ogl_name) noexcept
{
	::glDeleteVertexArrays(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglVao::enable_location(
	const int location)
{
	const auto ogl_function = (
		enabled_locations_[location] ?
		::glEnableVertexAttribArray :
		::glDisableVertexAttribArray
	);

	ogl_function(location);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglVao::initialize_resource()
{
	if (!manager_->get_ogl_device_features().vao_is_available_)
	{
		return;
	}

	auto ogl_name = GLuint{};

	::glGenVertexArrays(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		throw Exception{"Failed to create OpenGL vertex array object."};
	}

	vao_resource_.reset(ogl_name);
}

void GenericOglVao::initialize_locations()
{
	const auto& device_features = manager_->get_device_features();

	enabled_locations_.resize(device_features.vertex_input_max_locations_);
}

void GenericOglVao::initialize()
{
	if (!manager_)
	{
		throw Exception{"Null manager."};
	}

	initialize_resource();
	initialize_locations();
}

//
// GenericOglVao
// ==========================================================================


// ==========================================================================
// OglVaoDeleter
//

OglVaoDeleter::OglVaoDeleter(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context}
{
}

void OglVaoDeleter::operator()(
	const OglVaoPtr resource)
{
	ogl_context_->vao_destroy(resource);
}

//
// OglVaoDeleter
// ==========================================================================


// ==========================================================================
// OglVaoFactory
//

OglVaoUPtr OglVaoFactory::create(
	const OglVaoManagerPtr manager)
{
	return std::make_unique<GenericOglVao>(manager);
}

//
// OglVaoFactory
// ==========================================================================


} // detail
} // bstone
