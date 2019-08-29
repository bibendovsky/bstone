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
// OpenGL vertex input (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_vertex_input.h"

#include "glm/gtc/type_ptr.hpp"

#include "bstone_exception.h"

#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_vao.h"
#include "bstone_detail_ogl_vertex_input_manager.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglVertexInput
//

OglVertexInput::OglVertexInput() = default;

OglVertexInput::~OglVertexInput() = default;

//
// OglVertexInput
// =========================================================================


// =========================================================================
// GenericOglVertexInput
//

class GenericOglVertexInput final :
	public OglVertexInput
{
public:
	GenericOglVertexInput(
		const OglVertexInputManagerPtr vertex_input_manager,
		const RendererVertexInputCreateParam& param);

	~GenericOglVertexInput() override;


	RendererIndexBufferPtr get_index_buffer() const noexcept override;

	void bind() override;


private:
	const OglVertexInputManagerPtr vertex_input_manager_;

	RendererIndexBufferPtr index_buffer_;
	RendererVertexAttributeDescriptions attribute_descriptions_;


	OglVaoResource ogl_resource_;


	void initialize_vao();

	void initialize(
		const RendererVertexInputCreateParam& param);

	void assign_default_attribute(
		const RendererVertexAttributeDescription& attribute_description);

	void assign_regular_attribute(
		const RendererVertexAttributeDescription& attribute_description);

	void assign_attribute(
		const RendererVertexAttributeDescription& attribute_description);

	void bind_internal();
}; // GenericOglVertexInput

using GenericOglVertexInputPtr = GenericOglVertexInput*;
using GenericOglVertexInputUPtr = std::unique_ptr<GenericOglVertexInput>;

//
// GenericOglVertexInput
// =========================================================================


// =========================================================================
// GenericOglVertexInput
//

GenericOglVertexInput::GenericOglVertexInput(
	const OglVertexInputManagerPtr vertex_input_manager,
	const RendererVertexInputCreateParam& param)
	:
	vertex_input_manager_{vertex_input_manager},
	index_buffer_{},
	attribute_descriptions_{},
	ogl_resource_{nullptr, OglVaoDeleter{vertex_input_manager->ogl_context_get()}}
{
	initialize(param);
}

GenericOglVertexInput::~GenericOglVertexInput()
{
	const auto ogl_context = vertex_input_manager_->ogl_context_get();

	ogl_context->vao_destroy(ogl_resource_.get());
}

RendererIndexBufferPtr GenericOglVertexInput::get_index_buffer() const noexcept
{
	return index_buffer_;
}

void GenericOglVertexInput::bind()
{
	const auto ogl_context = vertex_input_manager_->ogl_context_get();

	const auto& ogl_device_features = ogl_context->get_ogl_device_features();

	if (ogl_device_features.vao_is_available_)
	{
		for (const auto& attribute_description : attribute_descriptions_)
		{
			if (attribute_description.is_default_)
			{
				assign_default_attribute(attribute_description);
			}
		}

		ogl_context->vao_bind(ogl_resource_.get());
	}
	else
	{
		bind_internal();
	}
}

void GenericOglVertexInput::initialize_vao()
{
	const auto ogl_context = vertex_input_manager_->ogl_context_get();

	ogl_resource_.reset(ogl_context->vao_create());

	ogl_context->vao_bind(ogl_resource_.get());

	index_buffer_->bind(true);

	const auto& ogl_device_features = ogl_context->get_ogl_device_features();

	if (ogl_device_features.vao_is_available_)
	{
		for (const auto& attribute_description : attribute_descriptions_)
		{
			if (!attribute_description.is_default_)
			{
				assign_regular_attribute(attribute_description);
			}
		}
	}
}

void GenericOglVertexInput::initialize(
	const RendererVertexInputCreateParam& param)
{
	const auto ogl_context = vertex_input_manager_->ogl_context_get();
	const auto& device_features = ogl_context->get_device_features();

	const auto max_locations = device_features.vertex_input_max_locations_;

	RendererUtils::vertex_input_validate_param(max_locations, param);

	const auto is_location_out_of_range = std::any_of(
		param.attribute_descriptions_.cbegin(),
		param.attribute_descriptions_.cend(),
		[=](const auto& item)
		{
			return item.location_ < 0 || item.location_ >= max_locations;
		}
	);

	if (is_location_out_of_range)
	{
		throw Exception{"Location out of range."};
	}

	index_buffer_ = param.index_buffer_;
	attribute_descriptions_ = param.attribute_descriptions_;

	initialize_vao();
}

void GenericOglVertexInput::assign_default_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	::glVertexAttrib4fv(
		attribute_description.location_,
		glm::value_ptr(attribute_description.default_value_)
	);

	assert(!OglRendererUtils::was_errors());
}

void GenericOglVertexInput::assign_regular_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	auto ogl_component_count = GLint{};
	auto ogl_component_format = GLenum{};
	auto ogl_is_normalized = GLenum{};

	switch (attribute_description.format_)
	{
		case RendererVertexAttributeFormat::r8g8b8a8_unorm:
			ogl_is_normalized = true;
			ogl_component_count = 4;
			ogl_component_format = GL_UNSIGNED_BYTE;
			break;

		case RendererVertexAttributeFormat::r32g32_sfloat:
			ogl_component_count = 2;
			ogl_component_format = GL_FLOAT;
			break;

		case RendererVertexAttributeFormat::r32g32b32_sfloat:
			ogl_component_count = 3;
			ogl_component_format = GL_FLOAT;
			break;

		default:
			throw Exception{"Invalid format."};
	}

	const auto ogl_context = vertex_input_manager_->ogl_context_get();

	const auto vertex_input_manager = ogl_context->vertex_input_get_manager();

	vertex_input_manager->vertex_input_location_enable(attribute_description.location_, true);

	auto vertex_buffer = attribute_description.vertex_buffer_;

	vertex_buffer->bind(true);

	const auto vertex_buffer_data = reinterpret_cast<const void*>(static_cast<std::intptr_t>(attribute_description.offset_));

	::glVertexAttribPointer(
		attribute_description.location_,
		ogl_component_count,
		ogl_component_format,
		ogl_is_normalized,
		attribute_description.stride_,
		vertex_buffer_data
	);

	assert(!OglRendererUtils::was_errors());
}

void GenericOglVertexInput::assign_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	if (attribute_description.is_default_)
	{
		assign_default_attribute(attribute_description);
	}
	else
	{
		assign_regular_attribute(attribute_description);
	}
}

void GenericOglVertexInput::bind_internal()
{
	const auto ogl_context = vertex_input_manager_->ogl_context_get();
	const auto vertex_input_manager = ogl_context->vertex_input_get_manager();

	vertex_input_manager->vertex_input_location_assign_begin();

	for (const auto& attribute_description : attribute_descriptions_)
	{
		assign_attribute(attribute_description);
	}

	vertex_input_manager->vertex_input_location_assign_end();
}

//
// GenericOglVertexInput
// =========================================================================


// =========================================================================
// OglVertexInputFactory
//

OglVertexInputUPtr OglVertexInputFactory::create(
	const OglVertexInputManagerPtr vertex_input_manager,
	const RendererVertexInputCreateParam& param)
{
	return std::make_unique<GenericOglVertexInput>(vertex_input_manager, param);
}

//
// OglVertexInputFactory
// =========================================================================


} // detail
} // bstone
