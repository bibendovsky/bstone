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

#include "bstone_exception.h"
#include "bstone_ogl_api.h"

#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_shader_stage_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglShaderVariable
//

OglShaderVariable::OglShaderVariable() = default;

OglShaderVariable::~OglShaderVariable() = default;

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
			throw Exception{"Unsupported type."};
	}
}

//
// OglShaderVariable
// ==========================================================================


// ==========================================================================
// GenericOglShaderVariable
//

class GenericOglShaderVariable :
	public OglShaderVariable
{
public:
	GenericOglShaderVariable(
		const OglShaderStagePtr shader_stage,
		const OglShaderVariableFactory::CreateParam& param);

	~GenericOglShaderVariable() override;


	Kind get_kind() const override;

	TypeId get_type_id() const override;

	int get_index() const override;

	const std::string& get_name() const override;

	int get_input_index() const override;


	void set_value(
		const std::int32_t value) override;

	void set_value(
		const float value) override;

	void set_value(
		const glm::vec2& value) override;

	void set_value(
		const glm::vec4& value) override;

	void set_value(
		const glm::mat4& value) override;


private:
	const OglShaderStagePtr shader_stage_;

	Kind kind_;
	TypeId type_id_;
	int value_size_;
	int index_;
	std::string name_;
	int input_index_;
	int ogl_location_;


	void initialize(
		const OglShaderVariableFactory::CreateParam& param);

	void set_value(
		const TypeId type_id,
		const void* const value_data);

	void set_value(
		const void* const value_data);
}; // GenericOglShaderVariable

using GenericOglShaderVariablePtr = GenericOglShaderVariable*;
using GenericOglShaderVariableUPtr = std::unique_ptr<GenericOglShaderVariable>;

//
// GenericOglShaderVariable
// ==========================================================================


// ==========================================================================
// GenericOglShaderVariable
//

GenericOglShaderVariable::GenericOglShaderVariable(
	const OglShaderStagePtr shader_stage,
	const OglShaderVariableFactory::CreateParam& param)
	:
	shader_stage_{shader_stage},
	kind_{},
	type_id_{},
	value_size_{},
	index_{},
	name_{},
	input_index_{},
	ogl_location_{}
{
	initialize(param);
}

GenericOglShaderVariable::~GenericOglShaderVariable() = default;

RendererShaderVariable::Kind GenericOglShaderVariable::get_kind() const
{
	return kind_;
}

RendererShaderVariable::TypeId GenericOglShaderVariable::get_type_id() const
{
	return type_id_;
}

int GenericOglShaderVariable::get_index() const
{
	return index_;
}

const std::string& GenericOglShaderVariable::get_name() const
{
	return name_;
}

int GenericOglShaderVariable::get_input_index() const
{
	return input_index_;
}

void GenericOglShaderVariable::set_value(
	const std::int32_t value)
{
	set_value(RendererShaderVariable::TypeId::int32, &value);
}

void GenericOglShaderVariable::set_value(
	const float value)
{
	set_value(RendererShaderVariable::TypeId::float32, &value);
}

void GenericOglShaderVariable::set_value(
	const glm::vec2& value)
{
	set_value(RendererShaderVariable::TypeId::vec2, &value);
}

void GenericOglShaderVariable::set_value(
	const glm::vec4& value)
{
	set_value(RendererShaderVariable::TypeId::vec4, &value);
}

void GenericOglShaderVariable::set_value(
	const glm::mat4& value)
{
	set_value(RendererShaderVariable::TypeId::mat4, &value);
}

void GenericOglShaderVariable::initialize(
	const OglShaderVariableFactory::CreateParam& param)
{
	if (!shader_stage_)
	{
		throw Exception{"Null shader stage."};
	}

	kind_ = param.kind_;
	type_id_ = param.type_id_;
	value_size_ = param.value_size_;
	index_ = param.index_;
	name_ = param.name_;
	input_index_ = param.input_index_;
	ogl_location_ = param.ogl_location_;
}

void GenericOglShaderVariable::set_value(
	const TypeId type_id,
	const void* const value_data)
{
	if (type_id != type_id_)
	{
		throw Exception{"Mismatch type."};
	}

	if (!value_data)
	{
		throw Exception{"Null value data."};
	}

	const auto value_size = get_unit_size(type_id);

	if (value_size != value_size_)
	{
		throw Exception{"Value size mismatch."};
	}

	switch (kind_)
	{
		case Kind::sampler:
		case Kind::uniform:
			break;

		default:
			throw Exception{"Changing vertex attribute not supported."};
	}


	const auto& ogl_device_features = shader_stage_->manager_get()->ogl_context_get()->get_ogl_device_features();

	if (!ogl_device_features.sso_is_available_)
	{
		shader_stage_->set_current();
	}

	set_value(value_data);
}

void GenericOglShaderVariable::set_value(
	const void* const value_data)
{
	const auto& ogl_device_features = shader_stage_->manager_get()->ogl_context_get()->get_ogl_device_features();

	auto shader_stage_ogl_name = GLuint{};

	if (ogl_device_features.sso_is_available_)
	{
		shader_stage_ogl_name = shader_stage_->ogl_name_get();
	}

	switch (type_id_)
	{
		case TypeId::int32:
		case TypeId::sampler2d:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniform1iv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					static_cast<const GLint*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniform1iv(
					ogl_location_,
					1,
					static_cast<const GLint*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		case TypeId::float32:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniform1fv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniform1fv(
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		case TypeId::vec2:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniform2fv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniform2fv(
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		case TypeId::vec3:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniform3fv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniform3fv(
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		case TypeId::vec4:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniform4fv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniform4fv(
					ogl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		case TypeId::mat4:
			if (ogl_device_features.sso_is_available_)
			{
				::glProgramUniformMatrix4fv(
					shader_stage_ogl_name,
					ogl_location_,
					1,
					GL_FALSE,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}
			else
			{
				::glUniformMatrix4fv(
					ogl_location_,
					1,
					GL_FALSE,
					static_cast<const GLfloat*>(value_data)
				);

				assert(!OglRendererUtils::was_errors());
			}

			break;

		default:
			throw Exception{"Unsupported type."};
	}
}

//
// GenericOglShaderVariable
// ==========================================================================


// ==========================================================================
// OglShaderVariableFactory
//

OglShaderVariableUPtr OglShaderVariableFactory::create(
	const OglShaderStagePtr shader_stage,
	const OglShaderVariableFactory::CreateParam& param)
{
	return std::make_unique<GenericOglShaderVariable>(shader_stage, param);
}

//
// OglShaderVariableFactory
// ==========================================================================


} // detail
} // bstone
