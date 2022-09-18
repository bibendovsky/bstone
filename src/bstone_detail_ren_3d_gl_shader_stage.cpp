/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL shader stage (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_detail_ren_3d_gl_shader_stage.h"

#include <cassert>

#include <unordered_set>

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_shader_stage_mgr.h"
#include "bstone_detail_ren_3d_gl_shader.h"
#include "bstone_detail_ren_3d_gl_shader_var.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlShaderStageException
//

class Ren3dGlShaderStageException :
	public Exception
{
public:
	explicit Ren3dGlShaderStageException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_SHADER_STAGE", message}
	{
	}
}; // Ren3dGlShaderStageException

//
// Ren3dGlShaderStageException
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageImpl
//

class Ren3dGlShaderStageImpl final :
	public Ren3dGlShaderStage
{
public:
	Ren3dGlShaderStageImpl(
		const Ren3dGlShaderStageMgrPtr shader_stage_manager,
		const Ren3dCreateShaderStageParam& param);

	~Ren3dGlShaderStageImpl() override;


	Ren3dGlShaderStageMgrPtr get_manager() const noexcept override;


	void set() override;


	Ren3dShaderVarPtr find_var(
		const std::string& name) noexcept override;

	Ren3dShaderInt32VarPtr find_int32_var(
		const std::string& name) noexcept override;

	Ren3dShaderFloat32VarPtr find_float32_var(
		const std::string& name) noexcept override;

	Ren3dShaderVec2VarPtr find_vec2_var(
		const std::string& name) noexcept override;

	Ren3dShaderVec4VarPtr find_vec4_var(
		const std::string& name) noexcept override;

	Ren3dShaderMat4VarPtr find_mat4_var(
		const std::string& name) noexcept override;

	Ren3dShaderSampler2dVarPtr find_sampler_2d_var(
		const std::string& name) noexcept override;


	void detach_fragment_shader() override;

	void detach_vertex_shader() override;

	GLuint get_gl_name() const noexcept override;


private:
	using NameBuffer = std::vector<char>;

	const Ren3dGlShaderStageMgrPtr shader_stage_manager_;

	Ren3dGlShaderPtr fragment_shader_;
	Ren3dGlShaderPtr vertex_shader_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void shader_stage_deleter(
		GLuint gl_name) noexcept;

	using ShaderStageResource = UniqueResource<GLuint, shader_stage_deleter>;

	ShaderStageResource shader_stage_resource_;

	using ShaderVars = std::vector<Ren3dGlShaderVarUPtr>;
	ShaderVars shader_vars_;


	void validate(
		const Ren3dShaderKind shader_kind,
		const Ren3dShaderPtr shader);

	void validate(
		const Ren3dShaderStageInputBindings& input_bindings);

	void validate(
		const Ren3dCreateShaderStageParam& param);

	void set_input_bindings(
		const GLuint gl_name,
		const Ren3dShaderStageInputBindings& input_bindings);

	int get_var_count(
		const GLuint gl_name);

	void get_vars(
		const Ren3dShaderVarKind kind,
		const GLuint gl_name,
		ShaderVars& shader_vars);

	void check_input_bindings(
		const Ren3dShaderStageInputBindings& input_bindings);

	Ren3dShaderVarPtr find_var_internal(
		const std::string& name) noexcept
	{
		const auto end_it = shader_vars_.end();

		const auto it = std::find_if(
			shader_vars_.begin(),
			end_it,
			[&name](const auto& item)
			{
				return name == item->get_name();
			}
		);

		if (it == end_it)
		{
			return nullptr;
		}

		return it->get();
	}

	template<typename T>
	T* find_var_internal(
		const Ren3dShaderVarTypeId type_id,
		const std::string& name) noexcept
	{
		const auto end_it = shader_vars_.end();

		const auto it = std::find_if(
			shader_vars_.begin(),
			end_it,
			[type_id, &name](const auto& item)
			{
				return type_id == item->get_type_id() && name == item->get_name();
			}
		);

		if (it == end_it)
		{
			return nullptr;
		}

		return static_cast<T*>(it->get());
	}
}; // Ren3dGlShaderStageImpl

//
// Ren3dGlShaderStageImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageImpl
//

Ren3dGlShaderStageImpl::Ren3dGlShaderStageImpl(
	const Ren3dGlShaderStageMgrPtr shader_stage_manager,
	const Ren3dCreateShaderStageParam& param)
try
	:
	shader_stage_manager_{shader_stage_manager},
	fragment_shader_{},
	vertex_shader_{},
	shader_stage_resource_{},
	shader_vars_{}
{
	if (!shader_stage_manager_)
	{
		fail("Null shader stage manager.");
	}

	validate(param);

	shader_stage_resource_.reset(glCreateProgram());

	if (!shader_stage_resource_)
	{
		fail("Failed to create an object.");
	}

	const auto fragment_shader = static_cast<Ren3dGlShaderPtr>(param.fragment_shader_);
	glAttachShader(shader_stage_resource_.get(), fragment_shader->get_gl_name());
	Ren3dGlError::ensure_debug();

	const auto vertex_shader = static_cast<Ren3dGlShaderPtr>(param.vertex_shader_);
	glAttachShader(shader_stage_resource_.get(), vertex_shader->get_gl_name());
	Ren3dGlError::ensure_debug();

	set_input_bindings(shader_stage_resource_.get(), param.input_bindings_);

	glLinkProgram(shader_stage_resource_.get());
	Ren3dGlError::ensure_debug();

	auto link_status = GLint{};

	glGetProgramiv(shader_stage_resource_.get(), GL_LINK_STATUS, &link_status);
	Ren3dGlError::ensure_debug();

	if (link_status != GL_TRUE)
	{
		auto error_message = std::string{"Failed to link a program."};

		const auto gl_log = Ren3dGlUtils::get_log(false, shader_stage_resource_.get());

		if (!gl_log.empty())
		{
			error_message += '\n';
			error_message += gl_log;
		}

		fail(error_message.c_str());
	}

	const auto var_count = get_var_count(shader_stage_resource_.get());
	shader_vars_.reserve(var_count);

	get_vars(Ren3dShaderVarKind::attribute, shader_stage_resource_.get(), shader_vars_);

	// Note that "samplers" are included in uniforms.
	get_vars(Ren3dShaderVarKind::uniform, shader_stage_resource_.get(), shader_vars_);

	check_input_bindings(param.input_bindings_);

	fragment_shader_ = static_cast<Ren3dGlShaderPtr>(param.fragment_shader_);
	vertex_shader_ = static_cast<Ren3dGlShaderPtr>(param.vertex_shader_);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlShaderStageImpl::~Ren3dGlShaderStageImpl()
{
	if (fragment_shader_)
	{
		fragment_shader_->attach_to_shader_stage(nullptr);
	}

	if (vertex_shader_)
	{
		vertex_shader_->attach_to_shader_stage(nullptr);
	}
}

Ren3dGlShaderStageMgrPtr Ren3dGlShaderStageImpl::get_manager() const noexcept
{
	return shader_stage_manager_;
}

void Ren3dGlShaderStageImpl::set()
try
{
	glUseProgram(shader_stage_resource_.get());
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dShaderVarPtr Ren3dGlShaderStageImpl::find_var(
	const std::string& name) noexcept
{
	return find_var_internal(name);
}

Ren3dShaderInt32VarPtr Ren3dGlShaderStageImpl::find_int32_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarInt32>(
		Ren3dShaderVarTypeId::int32,
		name
	);
}

Ren3dShaderFloat32VarPtr Ren3dGlShaderStageImpl::find_float32_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarFloat32>(
		Ren3dShaderVarTypeId::float32,
		name
	);
}

Ren3dShaderVec2VarPtr Ren3dGlShaderStageImpl::find_vec2_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarVec2>(
		Ren3dShaderVarTypeId::vec2,
		name
	);
}

Ren3dShaderVec4VarPtr Ren3dGlShaderStageImpl::find_vec4_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarVec4>(
		Ren3dShaderVarTypeId::vec4,
		name
	);
}

Ren3dShaderMat4VarPtr Ren3dGlShaderStageImpl::find_mat4_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarMat4>(
		Ren3dShaderVarTypeId::mat4,
		name
	);
}

Ren3dShaderSampler2dVarPtr Ren3dGlShaderStageImpl::find_sampler_2d_var(
	const std::string& name) noexcept
{
	return find_var_internal<Ren3dShaderVarSampler2d>(
		Ren3dShaderVarTypeId::sampler2d,
		name
	);
}

void Ren3dGlShaderStageImpl::detach_fragment_shader()
try
{
	fragment_shader_ = nullptr;
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::detach_vertex_shader()
try
{
	vertex_shader_ = nullptr;
}
catch (...)
{
	fail_nested(__func__);
}

GLuint Ren3dGlShaderStageImpl::get_gl_name() const noexcept
{
	return shader_stage_resource_.get();
}

[[noreturn]]
void Ren3dGlShaderStageImpl::fail(
	const char* message)
{
	throw Ren3dGlShaderStageException{message};
}

[[noreturn]]
void Ren3dGlShaderStageImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlShaderStageException{message});
}

void Ren3dGlShaderStageImpl::shader_stage_deleter(
	GLuint gl_name) noexcept
{
	glDeleteProgram(gl_name);
	Ren3dGlError::ensure_assert();
}

void Ren3dGlShaderStageImpl::validate(
	const Ren3dShaderKind shader_kind,
	const Ren3dShaderPtr shader)
try
{
	if (!shader)
	{
		fail("Null shader.");
	}

	if (shader->get_kind() != shader_kind)
	{
		fail("Shader kind mismatch.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::validate(
	const Ren3dShaderStageInputBindings& input_bindings)
try
{
	if (input_bindings.empty())
	{
		fail("No input bindings.");
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
			fail("Duplicate name.");
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
			fail("Duplicate index.");
		}
	}

	// Check for index value.
	//
	{
		for (const auto& input_binding : input_bindings)
		{
			if (input_binding.index_ < 0)
			{
				fail("Negative index.");
			}
		}
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::validate(
	const Ren3dCreateShaderStageParam& param)
try
{
	validate(Ren3dShaderKind::fragment, param.fragment_shader_);
	validate(Ren3dShaderKind::vertex, param.vertex_shader_);
	validate(param.input_bindings_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::set_input_bindings(
	const GLuint gl_name,
	const Ren3dShaderStageInputBindings& input_bindings)
try
{
	for (const auto& input_binding : input_bindings)
	{
		glBindAttribLocation(gl_name, input_binding.index_, input_binding.name_.c_str());
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

int Ren3dGlShaderStageImpl::get_var_count(
	const GLuint gl_name)
try
{
	auto gl_vertex_attribute_count = GLint{};
	glGetProgramiv(gl_name, GL_ACTIVE_ATTRIBUTES, &gl_vertex_attribute_count);
	Ren3dGlError::ensure_debug();

	auto gl_uniform_count = GLint{};
	glGetProgramiv(gl_name, GL_ACTIVE_UNIFORMS, &gl_uniform_count);
	Ren3dGlError::ensure_debug();

	const auto result = static_cast<int>(gl_vertex_attribute_count + gl_uniform_count);

	return result;
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::get_vars(
	const Ren3dShaderVarKind kind,
	const GLuint gl_name,
	ShaderVars& shader_vars)
try
{
	using GlInfoFunction = void (APIENTRYP)(
		const GLuint program,
		const GLuint index,
		const GLsizei bufSize,
		GLsizei* const length,
		GLint* const size,
		GLenum* const type,
		GLchar* const name);

	bool is_attribute = false;
	auto gl_count_enum = GLenum{};
	auto gl_max_length_enum = GLenum{};
	auto gl_info_function = GlInfoFunction{};

	switch (kind)
	{
		case Ren3dShaderVarKind::attribute:
			is_attribute = true;
			gl_count_enum = GL_ACTIVE_ATTRIBUTES;
			gl_max_length_enum = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
			gl_info_function = glGetActiveAttrib;
			break;

		case Ren3dShaderVarKind::uniform:
			gl_count_enum = GL_ACTIVE_UNIFORMS;
			gl_max_length_enum = GL_ACTIVE_UNIFORM_MAX_LENGTH;
			gl_info_function = glGetActiveUniform;
			break;

		default:
			fail("Unsupported variable kind.");
	}

	auto gl_count = GLint{};
	glGetProgramiv(gl_name, gl_count_enum, &gl_count);
	Ren3dGlError::ensure_debug();

	if (gl_count <= 0)
	{
		return;
	}

	auto gl_max_length = GLint{};
	glGetProgramiv(gl_name, gl_max_length_enum, &gl_max_length);
	Ren3dGlError::ensure_debug();

	if (gl_max_length <= 0)
	{
		return;
	}

	auto name_buffer = NameBuffer{};
	name_buffer.resize(gl_max_length);

	for (int i = 0; i < gl_count; ++i)
	{
		auto gl_length = GLsizei{};
		auto gl_size = GLint{};
		auto gl_type = GLenum{};
		auto var_param = Ren3dGlShaderVarCreateParam{};

		gl_info_function(
			gl_name,
			i,
			gl_max_length,
			&gl_length,
			&gl_size,
			&gl_type,
			name_buffer.data()
		);

		Ren3dGlError::ensure_debug();

		if (gl_length <= 0)
		{
			fail("Empty name.");
		}

		auto unit_count = 0;

		switch (gl_size)
		{
			case 1:
				unit_count = 1;
				break;

			default:
				fail("Unsupported unit count.");
		}

		bool is_sampler = false;
		auto unit_type_id = Ren3dShaderVarTypeId{};

		switch (gl_type)
		{
			case GL_INT:
				unit_type_id = Ren3dShaderVarTypeId::int32;
				break;

			case GL_FLOAT:
				unit_type_id = Ren3dShaderVarTypeId::float32;
				break;

			case GL_FLOAT_VEC2:
				unit_type_id = Ren3dShaderVarTypeId::vec2;
				break;

			case GL_FLOAT_VEC3:
				unit_type_id = Ren3dShaderVarTypeId::vec3;
				break;

			case GL_FLOAT_VEC4:
				unit_type_id = Ren3dShaderVarTypeId::vec4;
				break;

			case GL_FLOAT_MAT4:
				unit_type_id = Ren3dShaderVarTypeId::mat4;
				break;

			case GL_SAMPLER_2D:
				is_sampler = true;
				unit_type_id = Ren3dShaderVarTypeId::sampler2d;
				break;

			default:
				fail("Unsupported unit type.");
		}

		auto input_index = GLint{};
		auto gl_location = GLint{};

		if (is_attribute)
		{
			input_index = glGetAttribLocation(gl_name, name_buffer.data());
			Ren3dGlError::ensure_debug();

			if (input_index < 0)
			{
				fail("Vertex attribute not found.");
			}

			gl_location = -1;
		}
		else
		{
			input_index = -1;

			gl_location = glGetUniformLocation(gl_name, name_buffer.data());
			Ren3dGlError::ensure_debug();

			if (gl_location < 0)
			{
				fail("Uniform not found.");
			}
		}

		const auto new_kind = (is_sampler ? Ren3dShaderVarKind::sampler : kind);
		const auto index = static_cast<int>(shader_vars.size());
		const auto unit_size = Ren3dGlShaderVar::get_unit_size(unit_type_id);
		const auto value_size = unit_count * unit_size;

		auto name = std::string{};
		name.assign(name_buffer.data(), static_cast<std::size_t>(gl_length));

		var_param.kind = new_kind;
		var_param.type_id_ = unit_type_id;
		var_param.value_size_ = value_size;
		var_param.index_ = index;
		var_param.name_ = std::move(name);
		var_param.input_index_ = input_index;
		var_param.gl_location_ = gl_location;

		auto var = Ren3dGlShaderVarFactory::create(this, var_param);

		shader_vars.emplace_back(std::move(var));
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageImpl::check_input_bindings(
	const Ren3dShaderStageInputBindings& input_bindings)
try
{
	for (const auto& input_binding : input_bindings)
	{
		const auto vertex_attribute = find_var_internal(input_binding.name_);

		if (!vertex_attribute)
		{
			fail("Vertex attribute not found.");
		}

		if (vertex_attribute->get_kind() != Ren3dShaderVarKind::attribute)
		{
			fail("Not a vertex attribute.");
		}
	}
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlShaderStageImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageFactory
//

Ren3dGlShaderStageUPtr Ren3dGlShaderStageFactory::create(
	const Ren3dGlShaderStageMgrPtr gl_shader_stage_manager,
	const Ren3dCreateShaderStageParam& param)
{
	return std::make_unique<Ren3dGlShaderStageImpl>(gl_shader_stage_manager, param);
}

//
// Ren3dGlShaderStageFactory
// ==========================================================================


} // detail
} // bstone
