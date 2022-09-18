/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL shader variable (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_detail_ren_3d_gl_shader_var.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_api.h"
#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"
#include "bstone_detail_ren_3d_gl_shader_stage.h"
#include "bstone_detail_ren_3d_gl_shader_stage_mgr.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlShaderVarException
//

class Ren3dGlShaderVarException :
	public Exception
{
public:
	explicit Ren3dGlShaderVarException(
		const char* message) noexcept
		:
		Exception{"REN_3D_SHDR_VAR", message}
	{
	}
}; // Ren3dGlShaderVarException

//
// Ren3dGlShaderVarException
// ==========================================================================


// ==========================================================================

namespace
{


[[noreturn]]
void fail(
	const char* message)
{
	throw Ren3dGlShaderVarException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlShaderVarException{message});
}


} // namespace

// ==========================================================================


// ==========================================================================
// Ren3dGlShaderVar
//

int Ren3dGlShaderVar::get_unit_size(
	const Ren3dShaderVarTypeId type_id)
try
{
	switch (type_id)
	{
		case Ren3dShaderVarTypeId::int32:
		case Ren3dShaderVarTypeId::float32:
		case Ren3dShaderVarTypeId::sampler2d:
			return 4;

		case Ren3dShaderVarTypeId::vec2:
			return 2 * 4;

		case Ren3dShaderVarTypeId::vec3:
			return 3 * 4;

		case Ren3dShaderVarTypeId::vec4:
			return 4 * 4;

		case Ren3dShaderVarTypeId::mat4:
			return 4 * 4 * 4;

		default:
			fail("Unsupported type.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlShaderVar
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderVarImpl
//

class Ren3dGlShaderVarImpl final :
	public Ren3dGlShaderVar
{
public:
	Ren3dGlShaderVarImpl(
		const Ren3dGlShaderStagePtr shader_stage,
		const Ren3dGlShaderVarCreateParam& param);

	~Ren3dGlShaderVarImpl() override;


	Ren3dShaderVarKind get_kind() const noexcept override;

	Ren3dShaderVarTypeId get_type_id() const noexcept override;

	int get_index() const noexcept override;

	const std::string& get_name() const noexcept override;

	int get_input_index() const noexcept override;


	void set_int32(
		const std::int32_t value) override;

	void set_float32(
		const float value) override;

	void set_vec2(
		const float* const value) override;

	void set_vec4(
		const float* const value) override;

	void set_mat4(
		const float* const value) override;

	void set_sampler_2d(
		const std::int32_t value) override;


private:
	const Ren3dGlShaderStagePtr shader_stage_;
	const Ren3dGlDeviceFeatures& gl_device_features_;

	Ren3dShaderVarKind kind;
	Ren3dShaderVarTypeId type_id_;
	int value_size_;
	int index_;
	std::string name_;
	int input_index_;
	int gl_location_;


	void set_value(
		const Ren3dShaderVarTypeId type_id,
		const void* const value_data);

	void set_value(
		const void* const value_data);
}; // Ren3dGlShaderVarImpl

//
// Ren3dGlShaderVarImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderVarImpl
//

Ren3dGlShaderVarImpl::Ren3dGlShaderVarImpl(
	const Ren3dGlShaderStagePtr shader_stage,
	const Ren3dGlShaderVarCreateParam& param)
try
	:
	shader_stage_{shader_stage},
	gl_device_features_{shader_stage->get_manager()->get_context()->get_gl_device_features()},
	kind{},
	type_id_{},
	value_size_{},
	index_{},
	name_{},
	input_index_{},
	gl_location_{}
{
	if (!shader_stage_)
	{
		fail("Null shader stage.");
	}

	kind = param.kind;
	type_id_ = param.type_id_;
	value_size_ = param.value_size_;
	index_ = param.index_;
	name_ = param.name_;
	input_index_ = param.input_index_;
	gl_location_ = param.gl_location_;
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlShaderVarImpl::~Ren3dGlShaderVarImpl() = default;

Ren3dShaderVarKind Ren3dGlShaderVarImpl::get_kind() const noexcept
{
	return kind;
}

Ren3dShaderVarTypeId Ren3dGlShaderVarImpl::get_type_id() const noexcept
{
	return type_id_;
}

int Ren3dGlShaderVarImpl::get_index() const noexcept
{
	return index_;
}

const std::string& Ren3dGlShaderVarImpl::get_name() const noexcept
{
	return name_;
}

int Ren3dGlShaderVarImpl::get_input_index() const noexcept
{
	return input_index_;
}

void Ren3dGlShaderVarImpl::set_int32(
	const std::int32_t value)
try
{
	set_value(Ren3dShaderVarTypeId::int32, &value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_float32(
	const float value)
try
{
	set_value(Ren3dShaderVarTypeId::float32, &value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_vec2(
	const float* const value)
try
{
	set_value(Ren3dShaderVarTypeId::vec2, value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_vec4(
	const float* const value)
try
{
	set_value(Ren3dShaderVarTypeId::vec4, value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_mat4(
	const float* const value)
try
{
	set_value(Ren3dShaderVarTypeId::mat4, value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_sampler_2d(
	const std::int32_t value)
try
{
	set_value(Ren3dShaderVarTypeId::sampler2d, &value);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_value(
	const Ren3dShaderVarTypeId type_id,
	const void* const value_data)
try
{
	if (type_id != type_id_)
	{
		fail("Mismatch type.");
	}

	if (!value_data)
	{
		fail("Null value data.");
	}

	const auto value_size = get_unit_size(type_id);

	if (value_size != value_size_)
	{
		fail("Value size mismatch.");
	}

	switch (kind)
	{
		case Ren3dShaderVarKind::sampler:
		case Ren3dShaderVarKind::uniform:
			break;

		default:
			fail("Changing vertex attribute not supported.");
	}


	if (!gl_device_features_.is_sso_available_)
	{
		shader_stage_->set();
	}

	set_value(value_data);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderVarImpl::set_value(
	const void* const value_data)
try
{
	auto shader_stage_gl_name = GLuint{};

	if (gl_device_features_.is_sso_available_)
	{
		shader_stage_gl_name = shader_stage_->get_gl_name();
	}

	switch (type_id_)
	{
		case Ren3dShaderVarTypeId::int32:
		case Ren3dShaderVarTypeId::sampler2d:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniform1iv(
					shader_stage_gl_name,
					gl_location_,
					1,
					static_cast<const GLint*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniform1iv(
					gl_location_,
					1,
					static_cast<const GLint*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		case Ren3dShaderVarTypeId::float32:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniform1fv(
					shader_stage_gl_name,
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniform1fv(
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		case Ren3dShaderVarTypeId::vec2:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniform2fv(
					shader_stage_gl_name,
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniform2fv(
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		case Ren3dShaderVarTypeId::vec3:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniform3fv(
					shader_stage_gl_name,
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniform3fv(
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		case Ren3dShaderVarTypeId::vec4:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniform4fv(
					shader_stage_gl_name,
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniform4fv(
					gl_location_,
					1,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		case Ren3dShaderVarTypeId::mat4:
			if (gl_device_features_.is_sso_available_)
			{
				glProgramUniformMatrix4fv(
					shader_stage_gl_name,
					gl_location_,
					1,
					GL_FALSE,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}
			else
			{
				glUniformMatrix4fv(
					gl_location_,
					1,
					GL_FALSE,
					static_cast<const GLfloat*>(value_data)
				);

				Ren3dGlError::ensure_debug();
			}

			break;

		default:
			fail("Unsupported type.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlShaderVarImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderVarFactory
//

Ren3dGlShaderVarUPtr Ren3dGlShaderVarFactory::create(
	const Ren3dGlShaderStagePtr shader_stage,
	const Ren3dGlShaderVarCreateParam& param)
{
	return std::make_unique<Ren3dGlShaderVarImpl>(shader_stage, param);
}

//
// Ren3dGlShaderVarFactory
// ==========================================================================


} // detail
} // bstone
