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
// OpenGL shader variable (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_shader_variable.h"
#include "glm/gtc/type_ptr.hpp"
#include "bstone_detail_ogl_shader_stage.h"


namespace bstone
{
namespace detail
{


OglShaderVariable::OglShaderVariable() = default;

OglShaderVariable::~OglShaderVariable() = default;

RendererShaderVariable::Kind OglShaderVariable::get_kind() const
{
	return kind_;
}

RendererShaderVariable::TypeId OglShaderVariable::get_type_id() const
{
	return type_id_;
}

int OglShaderVariable::get_index() const
{
	return index_;
}

const std::string& OglShaderVariable::get_name() const
{
	return name_;
}

int OglShaderVariable::get_input_index() const
{
	return input_index_;
}

void OglShaderVariable::set_value(
	const std::int32_t value)
{
	set_value(RendererShaderVariable::TypeId::int32, &value);
}

void OglShaderVariable::set_value(
	const float value)
{
	set_value(RendererShaderVariable::TypeId::float32, &value);
}

void OglShaderVariable::set_value(
	const glm::vec2& value)
{
	set_value(RendererShaderVariable::TypeId::vec2, &value);
}

void OglShaderVariable::set_value(
	const glm::vec4& value)
{
	set_value(RendererShaderVariable::TypeId::vec4, &value);
}

void OglShaderVariable::set_value(
	const glm::mat4& value)
{
	set_value(RendererShaderVariable::TypeId::mat4, &value);
}

int OglShaderVariable::get_unit_size(
	const TypeId type_id)
{
	switch (type_id)
	{
		case TypeId::int32:
		case TypeId::float32:
		case TypeId::sampler2d:
			return 4;

		case TypeId::vec2:
			return 2 * 4;

		case TypeId::vec3:
			return 3 * 4;

		case TypeId::vec4:
			return 4 * 4;

		case TypeId::mat4:
			return 4 * 4 * 4;

		default:
			assert(!"Unsupported type.");

			return 0;
	}
}

void OglShaderVariable::set_value(
	const TypeId type_id,
	const void* const value_data)
{
	if (type_id != type_id_)
	{
		assert(!"Mismatch type.");

		return;
	}

	if (value_data == nullptr)
	{
		assert(!"Null value data.");

		return;
	}

	const auto value_size = get_unit_size(type_id);

	if (value_size != value_size_)
	{
		assert(!"Value size mismatch.");

		return;
	}

	switch (kind_)
	{
		case Kind::sampler:
		case Kind::uniform:
			break;

		default:
			assert(!"Changing vertex attribute not supported.");

			return;
	}

	shader_stage_->set_current();

	set_value(value_data);
}

void OglShaderVariable::set_value(
	const void* const value_data)
{
	switch (type_id_)
	{
		case TypeId::int32:
		case TypeId::sampler2d:
			::glUniform1iv(ogl_location_, 1, static_cast<const GLint*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		case TypeId::float32:
			::glUniform1fv(ogl_location_, 1, static_cast<const GLfloat*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		case TypeId::vec2:
			::glUniform2fv(ogl_location_, 1, static_cast<const GLfloat*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		case TypeId::vec3:
			::glUniform3fv(ogl_location_, 1, static_cast<const GLfloat*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		case TypeId::vec4:
			::glUniform4fv(ogl_location_, 1, static_cast<const GLfloat*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		case TypeId::mat4:
			::glUniformMatrix4fv(ogl_location_, 1, GL_FALSE, static_cast<const GLfloat*>(value_data));
			assert(!OglRendererUtils::was_errors());
			break;

		default:
			assert(!"Unsupported type.");

			return;
	}
}


} // detail
} // bstone
