/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader Stage

#include <cassert>

#include <algorithm>
#include <unordered_set>

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"
#include "bstone_zstring_view.h"
#include "bstone_zstring_view_hasher.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_shader.h"
#include "bstone_gl_r3r_shader_stage.h"
#include "bstone_gl_r3r_shader_var.h"
#include "bstone_gl_r3r_utils.h"

namespace bstone {

GlR3rShaderStage::GlR3rShaderStage() = default;

GlR3rShaderStage::~GlR3rShaderStage() = default;

// ==========================================================================

class GlR3rShaderStageImpl final : public GlR3rShaderStage
{
public:
	GlR3rShaderStageImpl(GlR3rContext& context, const R3rShaderStageInitParam& param);
	~GlR3rShaderStageImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	GlR3rContext& get_context() const noexcept override;
	void set() override;

private:
	R3rShaderVar* do_find_var(const char* name) noexcept override;
	R3rShaderInt32Var* do_find_int32_var(const char* name) noexcept override;
	R3rShaderFloat32Var* do_find_float32_var(const char* name) noexcept override;
	R3rShaderVec2Var* do_find_vec2_var(const char* name) noexcept override;
	R3rShaderVec4Var* do_find_vec4_var(const char* name) noexcept override;
	R3rShaderMat4Var* do_find_mat4_var(const char* name) noexcept override;
	R3rShaderR2SamplerVar* do_find_r2_sampler_var(const char* name) noexcept override;

	void detach_fragment_shader() override;
	void detach_vertex_shader() override;

	GLuint get_gl_name() const noexcept override;

private:
	using NameBuffer = std::vector<char>;

	struct ShaderStageDeleter
	{
		void operator()(GLuint gl_name) noexcept;
	};

	using ShaderStageResource = UniqueResource<GLuint, ShaderStageDeleter>;
	using ShaderVars = std::vector<GlR3rShaderVarUPtr>;

private:
	GlR3rContext& context_;

	GlR3rShader* fragment_shader_{};
	GlR3rShader* vertex_shader_{};

	ShaderStageResource shader_stage_resource_{};
	ShaderVars shader_vars_{};

private:
	void validate(R3rShaderType shader_type, R3rShader* shader);
	void validate(R3rShaderStageInputBindings input_bindings);
	void validate(const R3rShaderStageInitParam& param);

	void set_input_bindings(GLuint gl_name, R3rShaderStageInputBindings input_bindings);

	int get_var_count(GLuint gl_name);
	void get_vars(R3rShaderVarType type, GLuint gl_name, ShaderVars& shader_vars);
	void check_input_bindings(R3rShaderStageInputBindings input_bindings);
	R3rShaderVar* find_var_internal(const std::string& name) noexcept;

	template<typename T>
	T* find_var_internal(R3rShaderVarTypeId type_id, const char* name) noexcept;
};

// ==========================================================================

using GlR3rShaderStageImplPool = FixedPoolResource<GlR3rShaderStageImpl, R3rLimits::max_shader_stages()>;
GlR3rShaderStageImplPool gl_r3r_shader_stage_impl_pool{};

// ==========================================================================

GlR3rShaderStageImpl::GlR3rShaderStageImpl(
	GlR3rContext& context,
	const R3rShaderStageInitParam& param)
try
	:
	context_{context}
{
	validate(param);

	shader_stage_resource_.reset(glCreateProgram());

	if (shader_stage_resource_.get() == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create an object.");
	}

	const auto fragment_shader = static_cast<GlR3rShader*>(param.fragment_shader);
	glAttachShader(shader_stage_resource_.get(), fragment_shader->get_gl_name());
	GlR3rError::check_optionally();

	const auto vertex_shader = static_cast<GlR3rShader*>(param.vertex_shader);
	glAttachShader(shader_stage_resource_.get(), vertex_shader->get_gl_name());
	GlR3rError::check_optionally();

	set_input_bindings(shader_stage_resource_.get(), param.input_bindings);

	glLinkProgram(shader_stage_resource_.get());
	GlR3rError::check_optionally();

	auto link_status = GLint{};

	glGetProgramiv(shader_stage_resource_.get(), GL_LINK_STATUS, &link_status);
	GlR3rError::check_optionally();

	if (link_status != GL_TRUE)
	{
		auto error_message = std::string{};
		error_message.reserve(256);
		error_message += "Failed to link a program.";

		const auto gl_log = GlR3rUtils::get_log(false, shader_stage_resource_.get());

		if (!gl_log.empty())
		{
			error_message += '\n';
			error_message += gl_log;
		}

		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}

	const auto var_count = get_var_count(shader_stage_resource_.get());
	shader_vars_.reserve(var_count);

	get_vars(R3rShaderVarType::attribute, shader_stage_resource_.get(), shader_vars_);

	// Note that "samplers" are included in uniforms.
	get_vars(R3rShaderVarType::uniform, shader_stage_resource_.get(), shader_vars_);

	check_input_bindings(param.input_bindings);

	fragment_shader_ = static_cast<GlR3rShader*>(param.fragment_shader);
	vertex_shader_ = static_cast<GlR3rShader*>(param.vertex_shader);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rShaderStageImpl::~GlR3rShaderStageImpl()
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

void* GlR3rShaderStageImpl::operator new(std::size_t size)
try {
	return gl_r3r_shader_stage_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::operator delete(void* ptr)
{
	gl_r3r_shader_stage_impl_pool.deallocate(ptr);
}

GlR3rContext& GlR3rShaderStageImpl::get_context() const noexcept
{
	return context_;
}

void GlR3rShaderStageImpl::set()
try {
	glUseProgram(shader_stage_resource_.get());
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderVar* GlR3rShaderStageImpl::do_find_var(const char* name) noexcept
{
	return find_var_internal(name);
}

R3rShaderInt32Var* GlR3rShaderStageImpl::do_find_int32_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderInt32Var>(R3rShaderVarTypeId::int32, name);
}

R3rShaderFloat32Var* GlR3rShaderStageImpl::do_find_float32_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderFloat32Var>(R3rShaderVarTypeId::float32, name);
}

R3rShaderVec2Var* GlR3rShaderStageImpl::do_find_vec2_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderVec2Var>(R3rShaderVarTypeId::vec2, name);
}

R3rShaderVec4Var* GlR3rShaderStageImpl::do_find_vec4_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderVec4Var>(R3rShaderVarTypeId::vec4, name);
}

R3rShaderMat4Var* GlR3rShaderStageImpl::do_find_mat4_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderMat4Var>(R3rShaderVarTypeId::mat4, name);
}

R3rShaderR2SamplerVar* GlR3rShaderStageImpl::do_find_r2_sampler_var(const char* name) noexcept
{
	return find_var_internal<R3rShaderR2SamplerVar>(R3rShaderVarTypeId::sampler2d, name);
}

void GlR3rShaderStageImpl::detach_fragment_shader()
try {
	fragment_shader_ = nullptr;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::detach_vertex_shader()
try {
	vertex_shader_ = nullptr;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLuint GlR3rShaderStageImpl::get_gl_name() const noexcept
{
	return shader_stage_resource_.get();
}

void GlR3rShaderStageImpl::ShaderStageDeleter::operator()(GLuint gl_name) noexcept
{
	glDeleteProgram(gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rShaderStageImpl::validate(R3rShaderType shader_type, R3rShader* shader)
try {
	if (shader == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null shader.");
	}

	if (shader->get_type() != shader_type)
	{
		BSTONE_THROW_STATIC_SOURCE("Shader type mismatch.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::validate(R3rShaderStageInputBindings input_bindings)
try {
	if (input_bindings.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("No input bindings.");
	}

	// Check for duplicate names.
	//
	{
		using NameSet = std::unordered_set<ZStringView, ZStringViewHasher>;
		auto name_set = NameSet{};
		name_set.reserve(static_cast<std::size_t>(input_bindings.get_size()));

		for (const auto& input_binding : input_bindings)
		{
			name_set.emplace(ZStringView{input_binding.name});
		}

		if (name_set.size() != static_cast<std::size_t>(input_bindings.get_size()))
		{
			BSTONE_THROW_STATIC_SOURCE("Duplicate name.");
		}
	}

	// Check for duplicate indices.
	//
	{
		using NameSetItem = int;
		using NameSet = std::unordered_set<NameSetItem>;
		auto name_set = NameSet{};
		name_set.reserve(static_cast<std::size_t>(input_bindings.get_size()));

		for (const auto& input_binding : input_bindings)
		{
			name_set.emplace(input_binding.index);
		}

		if (name_set.size() != static_cast<std::size_t>(input_bindings.get_size()))
		{
			BSTONE_THROW_STATIC_SOURCE("Duplicate index.");
		}
	}

	// Check for index value.
	//
	{
		for (const auto& input_binding : input_bindings)
		{
			if (input_binding.index < 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Negative index.");
			}
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::validate(const R3rShaderStageInitParam& param)
try {
	validate(R3rShaderType::fragment, param.fragment_shader);
	validate(R3rShaderType::vertex, param.vertex_shader);
	validate(param.input_bindings);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::set_input_bindings(
	GLuint gl_name,
	R3rShaderStageInputBindings input_bindings)
try {
	for (const auto& input_binding : input_bindings)
	{
		glBindAttribLocation(gl_name, input_binding.index, input_binding.name);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rShaderStageImpl::get_var_count(GLuint gl_name)
try {
	auto gl_vertex_attribute_count = GLint{};
	glGetProgramiv(gl_name, GL_ACTIVE_ATTRIBUTES, &gl_vertex_attribute_count);
	GlR3rError::check_optionally();

	auto gl_uniform_count = GLint{};
	glGetProgramiv(gl_name, GL_ACTIVE_UNIFORMS, &gl_uniform_count);
	GlR3rError::check_optionally();

	return static_cast<int>(gl_vertex_attribute_count + gl_uniform_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::get_vars(R3rShaderVarType type, GLuint gl_name, ShaderVars& shader_vars)
try {
	using GlInfoFunction = void (GL_APIENTRY *)(
		const GLuint program,
		const GLuint index,
		const GLsizei bufSize,
		GLsizei* const length,
		GLint* const size,
		GLenum* const type,
		GLchar* const name);

	auto is_attribute = false;
	auto gl_count_enum = GLenum{};
	auto gl_max_length_enum = GLenum{};
	auto gl_info_function = GlInfoFunction{};

	switch (type)
	{
		case R3rShaderVarType::attribute:
			is_attribute = true;
			gl_count_enum = GL_ACTIVE_ATTRIBUTES;
			gl_max_length_enum = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
			gl_info_function = glGetActiveAttrib;
			break;

		case R3rShaderVarType::uniform:
			gl_count_enum = GL_ACTIVE_UNIFORMS;
			gl_max_length_enum = GL_ACTIVE_UNIFORM_MAX_LENGTH;
			gl_info_function = glGetActiveUniform;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported variable type.");
	}

	auto gl_count = GLint{};
	glGetProgramiv(gl_name, gl_count_enum, &gl_count);
	GlR3rError::check_optionally();

	if (gl_count <= 0)
	{
		return;
	}

	auto gl_max_length = GLint{};
	glGetProgramiv(gl_name, gl_max_length_enum, &gl_max_length);
	GlR3rError::check_optionally();

	if (gl_max_length <= 0)
	{
		return;
	}

	auto name_buffer = NameBuffer{};
	name_buffer.resize(gl_max_length);

	for (auto i = 0; i < gl_count; ++i)
	{
		auto gl_length = GLsizei{};
		auto gl_size = GLint{};
		auto gl_type = GLenum{};
		auto var_param = GlR3rShaderVarInitParam{};

		gl_info_function(
			gl_name,
			i,
			gl_max_length,
			&gl_length,
			&gl_size,
			&gl_type,
			name_buffer.data());

		GlR3rError::check_optionally();

		if (gl_length <= 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Empty name.");
		}

		auto unit_count = 0;

		switch (gl_size)
		{
			case 1:
				unit_count = 1;
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unsupported unit count.");
		}

		auto is_sampler = false;
		auto unit_type_id = R3rShaderVarTypeId{};

		switch (gl_type)
		{
			case GL_INT: unit_type_id = R3rShaderVarTypeId::int32; break;
			case GL_FLOAT: unit_type_id = R3rShaderVarTypeId::float32; break;
			case GL_FLOAT_VEC2: unit_type_id = R3rShaderVarTypeId::vec2; break;
			case GL_FLOAT_VEC3: unit_type_id = R3rShaderVarTypeId::vec3; break;
			case GL_FLOAT_VEC4: unit_type_id = R3rShaderVarTypeId::vec4; break;
			case GL_FLOAT_MAT4: unit_type_id = R3rShaderVarTypeId::mat4; break;

			case GL_SAMPLER_2D:
				is_sampler = true;
				unit_type_id = R3rShaderVarTypeId::sampler2d;
				break;

			default: BSTONE_THROW_STATIC_SOURCE("Unsupported unit type.");
		}

		auto input_index = GLint{};
		auto gl_location = GLint{};

		if (is_attribute)
		{
			input_index = glGetAttribLocation(gl_name, name_buffer.data());
			GlR3rError::check_optionally();

			if (input_index < 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Vertex attribute not found.");
			}

			gl_location = -1;
		}
		else
		{
			input_index = -1;

			gl_location = glGetUniformLocation(gl_name, name_buffer.data());
			GlR3rError::check_optionally();

			if (gl_location < 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Uniform not found.");
			}
		}

		const auto new_type = is_sampler ? R3rShaderVarType::sampler : type;
		const auto index = static_cast<int>(shader_vars.size());
		const auto unit_size = GlR3rShaderVar::get_unit_size(unit_type_id);
		const auto value_size = unit_count * unit_size;

		var_param.type = new_type;
		var_param.type_id = unit_type_id;
		var_param.value_size = value_size;
		var_param.index = index;
		var_param.name = StringView{name_buffer.data(), gl_length};
		var_param.input_index = input_index;
		var_param.gl_location = gl_location;

		auto var = make_gl_r3r_shader_var(*this, var_param);
		shader_vars.emplace_back(std::move(var));
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderStageImpl::check_input_bindings(R3rShaderStageInputBindings input_bindings)
try {
	for (const auto& input_binding : input_bindings)
	{
		const auto vertex_attribute = find_var_internal(input_binding.name);

		if (vertex_attribute == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("Vertex attribute not found.");
		}

		if (vertex_attribute->get_type() != R3rShaderVarType::attribute)
		{
			BSTONE_THROW_STATIC_SOURCE("Not a vertex attribute.");
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderVar* GlR3rShaderStageImpl::find_var_internal(const std::string& name) noexcept
{
	const auto end_it = shader_vars_.end();

	const auto it = std::find_if(
		shader_vars_.begin(),
		end_it,
		[&name](const GlR3rShaderVarUPtr& item)
		{
			return name == item->get_name();
		});

	if (it == end_it)
	{
		return nullptr;
	}

	return it->get();
}

template<typename T>
T* GlR3rShaderStageImpl::find_var_internal(R3rShaderVarTypeId type_id, const char* name) noexcept
{
	const auto end_it = shader_vars_.end();

	const auto it = std::find_if(
		shader_vars_.begin(),
		end_it,
		[type_id, name](const GlR3rShaderVarUPtr& item)
		{
			return type_id == item->get_type_id() && name == item->get_name();
		});

	if (it == end_it)
	{
		return nullptr;
	}

	return static_cast<T*>(it->get());
}

// ==========================================================================

GlR3rShaderStageUPtr make_gl_r3r_shader_stage(GlR3rContext& context, const R3rShaderStageInitParam& param)
{
	return std::make_unique<GlR3rShaderStageImpl>(context, param);
}

} // namespace bstone
