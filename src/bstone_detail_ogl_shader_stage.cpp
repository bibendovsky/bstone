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
// OpenGL shader stage (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_precompiled.h"
#include <unordered_set>
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


class OglShaderStage::Detail
{
public:
	static RendererShaderVariablePtr find_variable(
		const std::string& name,
		ShaderVariables& shader_variables)
	{
		const auto end_it = shader_variables.end();

		const auto it = std::find_if(
			shader_variables.begin(),
			end_it,
			[&](const auto& item)
			{
				return name == item.name_;
			}
		);

		if (it == end_it)
		{
			return nullptr;
		}

		return &(*it);
	}

	template<typename T>
	static T* find_variable(
		const RendererShaderVariable::TypeId type_id,
		const std::string& name,
		ShaderVariables& shader_variables)
	{
		const auto end_it = shader_variables.end();

		const auto it = std::find_if(
			shader_variables.begin(),
			end_it,
			[&](const auto& item)
			{
				return type_id == item.type_id_ && name == item.name_;
			}
		);

		if (it == end_it)
		{
			return nullptr;
		}

		return static_cast<T*>(&(*it));
	}

	static RendererShaderVariableCPtr find_variable(
		const std::string& name,
		const ShaderVariables& shader_variables)
	{
		const auto end_it = shader_variables.cend();

		const auto it = std::find_if(
			shader_variables.cbegin(),
			end_it,
			[&](const auto& item)
			{
				return name == item.name_;
			}
		);

		if (it == end_it)
		{
			return nullptr;
		}

		return &(*it);
	}
}; // Detail


OglShaderStage::OglShaderStage()
	:
	is_initialized_{},
	error_message_{},
	current_shader_stage_ptr_{},
	fragment_shader_{},
	vertex_shader_{},
	ogl_name_raii_{},
	shader_variables_{}
{
}

OglShaderStage::~OglShaderStage()
{
	if (fragment_shader_ != nullptr)
	{
		fragment_shader_->attach_to_shader_stage(nullptr);
	}

	if (vertex_shader_ != nullptr)
	{
		vertex_shader_->attach_to_shader_stage(nullptr);
	}
}

bool OglShaderStage::is_initialized() const
{
	return is_initialized_;
}

const std::string& OglShaderStage::get_error_message() const
{
	return error_message_;
}

void OglShaderStage::set_current()
{
	if (!is_initialized_)
	{
		assert(!"Not initialized.");

		return;
	}

	assert(current_shader_stage_ptr_ != nullptr);

	if (*current_shader_stage_ptr_ == this)
	{
		return;
	}

	*current_shader_stage_ptr_ = this;

	::glUseProgram(ogl_name_raii_.get());
	assert(!OglRendererUtils::was_errors());
}

RendererShaderVariablePtr OglShaderStage::find_variable(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable(name, shader_variables_);
}

RendererShaderVariableInt32Ptr OglShaderStage::find_variable_int32(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableInt32>(
		RendererShaderVariable::TypeId::int32,
		name,
		shader_variables_
	);
}

RendererShaderVariableFloat32Ptr OglShaderStage::find_variable_float32(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableFloat32>(
		RendererShaderVariable::TypeId::float32,
		name,
		shader_variables_
	);
}

RendererShaderVariableVec2Ptr OglShaderStage::find_variable_vec2(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableVec2>(
		RendererShaderVariable::TypeId::vec2,
		name,
		shader_variables_
	);
}

RendererShaderVariableVec4Ptr OglShaderStage::find_variable_vec4(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableVec4>(
		RendererShaderVariable::TypeId::vec4,
		name,
		shader_variables_
	);
}

RendererShaderVariableMat4Ptr OglShaderStage::find_variable_mat4(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableMat4>(
		RendererShaderVariable::TypeId::mat4,
		name,
		shader_variables_
	);
}

RendererShaderVariableSampler2dPtr OglShaderStage::find_variable_sampler_2d(
	const std::string& name)
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	return Detail::find_variable<RendererShaderVariableSampler2d>(
			RendererShaderVariable::TypeId::sampler2d,
			name,
			shader_variables_
	);
}

void OglShaderStage::initialize(
	OglShaderStagePtr* current_shader_stage_ptr,
	const RendererShaderStage::CreateParam& param)
{
	if (current_shader_stage_ptr == nullptr)
	{
		error_message_ = "Null current shader stage pointer.";

		return;
	}

	current_shader_stage_ptr_ = current_shader_stage_ptr;

	if (!validate_param(param))
	{
		return;
	}

	auto ogl_name_raii = OglProgramHandle{::glCreateProgram()};

	if (ogl_name_raii == 0)
	{
		error_message_ = "Failed to create OpenGL program object.";

		return;
	}

	const auto fragment_shader = static_cast<OglShaderPtr>(param.fragment_shader_);
	::glAttachShader(ogl_name_raii.get(), fragment_shader->get_ogl_name());
	assert(!detail::OglRendererUtils::was_errors());

	const auto vertex_shader = static_cast<OglShaderPtr>(param.vertex_shader_);
	::glAttachShader(ogl_name_raii.get(), vertex_shader->get_ogl_name());
	assert(!detail::OglRendererUtils::was_errors());

	set_input_bindings(ogl_name_raii.get(), param.input_bindings_);

	::glLinkProgram(ogl_name_raii.get());
	assert(!detail::OglRendererUtils::was_errors());

	auto link_status = GLint{};

	::glGetProgramiv(ogl_name_raii.get(), GL_LINK_STATUS, &link_status);
	assert(!detail::OglRendererUtils::was_errors());

	if (link_status != GL_TRUE)
	{
		error_message_ = "Failed to link a program.";

		const auto ogl_log = OglRendererUtils::get_log(false, ogl_name_raii.get());

		if (!ogl_log.empty())
		{
			error_message_ += '\n';
			error_message_ += ogl_log;
		}

		return;
	}

	const auto variable_count = get_variable_count(ogl_name_raii.get());
	auto shader_variables = ShaderVariables{};
	shader_variables.reserve(variable_count);

	if (!get_variables(RendererShaderVariable::Kind::attribute, ogl_name_raii.get(), shader_variables))
	{
		return;
	}

	// Note that "samplers" are included in uniforms.
	if (!get_variables(RendererShaderVariable::Kind::uniform, ogl_name_raii.get(), shader_variables))
	{
		return;
	}

	if (!check_input_bindings(param.input_bindings_, shader_variables))
	{
		return;
	}

	is_initialized_ = true;
	fragment_shader_ = static_cast<OglShaderPtr>(param.fragment_shader_);
	vertex_shader_ = static_cast<OglShaderPtr>(param.vertex_shader_);
	ogl_name_raii_ = std::move(ogl_name_raii);
	shader_variables_ = std::move(shader_variables);
}

void OglShaderStage::detach_fragment_shader()
{
	is_initialized_ = false;
	fragment_shader_ = nullptr;
}

void OglShaderStage::detach_vertex_shader()
{
	is_initialized_ = false;
	vertex_shader_ = nullptr;
}

void OglShaderStage::unset_current()
{
	::glUseProgram(0);
	assert(!detail::OglRendererUtils::was_errors());
}

bool OglShaderStage::validate_shader(
	const RendererShader::Kind shader_kind,
	const RendererShaderPtr shader)
{
	if (shader == nullptr)
	{
		error_message_ = "Null shader.";

		return false;
	}

	if (!shader->is_initialized())
	{
		error_message_ = "Shader not initialized.";

		return false;
	}

	if (shader->get_kind() != shader_kind)
	{
		error_message_ = "Shader kind mismatch.";

		return false;
	}

	return true;
}

bool OglShaderStage::validate_input_bindings(
	const InputBindings& input_bindings)
{
	if (input_bindings.empty())
	{
		error_message_ = "No input bindings.";

		return false;
	}

	// Check for duplicate names.
	//
	{
		using NameSetItem = const std::string*;

		struct NameSetItemComparer
		{
			bool operator()(
				const NameSetItem lhs,
				const NameSetItem rhs) const
			{
				return *lhs == *rhs;
			}
		}; // NameSetItemComparer

		using NameSet = std::unordered_set<NameSetItem, std::hash<NameSetItem>, NameSetItemComparer>;
		auto name_set = NameSet{};
		name_set.reserve(input_bindings.size());

		for (const auto& input_binding : input_bindings)
		{
			name_set.emplace(&input_binding.name_);
		}

		if (name_set.size() != input_bindings.size())
		{
			error_message_ = "Duplicate name.";

			return false;
		}
	}

	// Check for duplicate indices.
	//
	{
		using NameSetItem = int;
		using NameSet = std::unordered_set<NameSetItem>;
		auto name_set = NameSet{};
		name_set.reserve(input_bindings.size());

		for (const auto& input_binding : input_bindings)
		{
			name_set.emplace(input_binding.index_);
		}

		if (name_set.size() != input_bindings.size())
		{
			error_message_ = "Duplicate index.";

			return false;
		}
	}

	// Check for index value.
	//
	{
		for (const auto& input_binding : input_bindings)
		{
			if (input_binding.index_ < 0)
			{
				error_message_ = "Negative index.";

				return false;
			}
		}
	}

	//
	return true;
}

bool OglShaderStage::validate_param(
	const RendererShaderStage::CreateParam& param)
{
	if (!validate_shader(RendererShader::Kind::fragment, param.fragment_shader_))
	{
		return false;
	}

	if (!validate_shader(RendererShader::Kind::vertex, param.vertex_shader_))
	{
		return false;
	}

	if (!validate_input_bindings(param.input_bindings_))
	{
		return false;
	}

	return true;
}

void OglShaderStage::set_input_bindings(
	const GLuint ogl_name,
	const InputBindings& input_bindings)
{
	for (const auto& input_binding : input_bindings)
	{
		::glBindAttribLocation(ogl_name, input_binding.index_, input_binding.name_.c_str());
		assert(!detail::OglRendererUtils::was_errors());
	}
}

int OglShaderStage::get_variable_count(
	const GLuint ogl_name)
{
	auto ogl_vertex_attribute_count = GLint{};
	::glGetProgramiv(ogl_name, GL_ACTIVE_ATTRIBUTES, &ogl_vertex_attribute_count);
	assert(!detail::OglRendererUtils::was_errors());

	auto ogl_uniform_count = GLint{};
	::glGetProgramiv(ogl_name, GL_ACTIVE_UNIFORMS, &ogl_uniform_count);
	assert(!detail::OglRendererUtils::was_errors());

	const auto result = ogl_vertex_attribute_count + ogl_uniform_count;

	return result;
}

bool OglShaderStage::get_variables(
	const RendererShaderVariable::Kind kind,
	const GLuint ogl_name,
	ShaderVariables& shader_variables)
{
	using OglInfoFunction = void (APIENTRYP)(
		const GLuint program,
		const GLuint index,
		const GLsizei bufSize,
		GLsizei* const length,
		GLint* const size,
		GLenum* const type,
		GLchar* const name);

	bool is_attribute = false;
	bool is_uniform = false;
	auto ogl_count_enum = GLenum{};
	auto ogl_max_length_enum = GLenum{};
	auto ogl_info_function = OglInfoFunction{};

	switch (kind)
	{
		case RendererShaderVariable::Kind::attribute:
			is_attribute = true;
			ogl_count_enum = GL_ACTIVE_ATTRIBUTES;
			ogl_max_length_enum = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
			ogl_info_function = ::glGetActiveAttrib;
			break;

		case RendererShaderVariable::Kind::uniform:
			is_uniform = true;
			ogl_count_enum = GL_ACTIVE_UNIFORMS;
			ogl_max_length_enum = GL_ACTIVE_UNIFORM_MAX_LENGTH;
			ogl_info_function = ::glGetActiveUniform;
			break;

		default:
			error_message_ = "Unsupported variable kind.";

			return false;
	}

	auto ogl_count = GLint{};
	::glGetProgramiv(ogl_name, ogl_count_enum, &ogl_count);
	assert(!detail::OglRendererUtils::was_errors());

	if (ogl_count <= 0)
	{
		return true;
	}

	auto ogl_max_length = GLint{};
	::glGetProgramiv(ogl_name, ogl_max_length_enum, &ogl_max_length);
	assert(!detail::OglRendererUtils::was_errors());

	if (ogl_max_length <= 0)
	{
		return true;
	}

	auto name_buffer = NameBuffer{};
	name_buffer.resize(ogl_max_length);

	for (int i = 0; i < ogl_count; ++i)
	{
		auto ogl_length = GLsizei{};
		auto ogl_size = GLint{};
		auto ogl_type = GLenum{};
		auto variable = OglShaderVariable{};

		ogl_info_function(
			ogl_name,
			i,
			ogl_max_length,
			&ogl_length,
			&ogl_size,
			&ogl_type,
			name_buffer.data()
		);

		assert(!detail::OglRendererUtils::was_errors());

		if (ogl_length <= 0)
		{
			error_message_ = "Empty name.";

			return false;
		}

		auto unit_count = 0;

		switch (ogl_size)
		{
			case 1:
				unit_count = 1;
				break;

			default:
				error_message_ = "Unsupported unit count.";

				return false;
		}

		bool is_sampler = false;
		auto unit_type_id = RendererShaderVariable::TypeId{};

		switch (ogl_type)
		{
			case GL_INT:
				unit_type_id = RendererShaderVariable::TypeId::int32;
				break;

			case GL_FLOAT:
				unit_type_id = RendererShaderVariable::TypeId::float32;
				break;

			case GL_FLOAT_VEC2:
				unit_type_id = RendererShaderVariable::TypeId::vec2;
				break;

			case GL_FLOAT_VEC3:
				unit_type_id = RendererShaderVariable::TypeId::vec3;
				break;

			case GL_FLOAT_VEC4:
				unit_type_id = RendererShaderVariable::TypeId::vec4;
				break;

			case GL_FLOAT_MAT4:
				unit_type_id = RendererShaderVariable::TypeId::mat4;
				break;

			case GL_SAMPLER_2D:
				is_sampler = true;
				unit_type_id = RendererShaderVariable::TypeId::sampler2d;
				break;

			default:
				error_message_ = "Unsupported unit type.";

				return false;
		}

		auto input_index = GLint{};

		if (is_attribute)
		{
			input_index = ::glGetAttribLocation(ogl_name, name_buffer.data());
			assert(!detail::OglRendererUtils::was_errors());

			if (input_index < 0)
			{
				error_message_ = "Vertex attribute not found.";

				return false;
			}
		}
		else
		{
			input_index = -1;
		}

		const auto new_kind = (is_sampler ? RendererShaderVariable::Kind::sampler : kind);
		const auto index = static_cast<int>(shader_variables.size());
		const auto unit_size = OglShaderVariable::get_unit_size(unit_type_id);
		const auto value_size = unit_count * unit_size;

		auto name = std::string{};
		name.assign(name_buffer.data(), static_cast<std::size_t>(ogl_length));

		variable.kind_ = new_kind;
		variable.type_id_ = unit_type_id;
		variable.value_size_ = value_size;
		variable.index_ = index;
		variable.name_ = std::move(name);
		variable.input_index_ = input_index;
		variable.ogl_location_ = i;
		variable.shader_stage_ = this;

		shader_variables.emplace_back(variable);
	}

	return true;
}

bool OglShaderStage::check_input_bindings(
	const InputBindings& input_bindings,
	const ShaderVariables& shader_variables)
{
	for (const auto& input_binding : input_bindings)
	{
		const auto vertex_attribute = Detail::find_variable(input_binding.name_, shader_variables);

		if (vertex_attribute == nullptr)
		{
			error_message_ = "Vertex attribute not found.";

			return false;
		}

		if (vertex_attribute->get_kind() != RendererShaderVariable::Kind::attribute)
		{
			error_message_ = "Not a vertex attribute.";

			return false;
		}
	}

	return true;
}


} // detail
} // bstone
