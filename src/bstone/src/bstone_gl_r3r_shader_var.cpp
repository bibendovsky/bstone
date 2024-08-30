/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader Variable

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_utils.h"
#include "bstone_gl_r3r_shader_stage.h"
#include "bstone_gl_r3r_shader_var.h"

namespace bstone {

GlR3rShaderVar::GlR3rShaderVar() = default;

GlR3rShaderVar::~GlR3rShaderVar() = default;

// ==========================================================================

int GlR3rShaderVar::get_unit_size(R3rShaderVarTypeId type_id)
try {
	switch (type_id)
	{
		case R3rShaderVarTypeId::int32:
		case R3rShaderVarTypeId::float32:
		case R3rShaderVarTypeId::sampler2d:
			return 4;

		case R3rShaderVarTypeId::vec2: return 2 * 4;
		case R3rShaderVarTypeId::vec3: return 3 * 4;
		case R3rShaderVarTypeId::vec4: return 4 * 4;
		case R3rShaderVarTypeId::mat4: return 4 * 4 * 4;

		default: BSTONE_THROW_STATIC_SOURCE("Unsupported type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

class GlR3rShaderVarImpl final : public GlR3rShaderVar
{
public:
	GlR3rShaderVarImpl(
		GlR3rShaderStage& shader_stage,
		const GlR3rShaderVarInitParam& param);

	~GlR3rShaderVarImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderVarType do_get_type() const noexcept override;
	R3rShaderVarTypeId do_get_type_id() const noexcept override;
	int do_get_index() const noexcept override;
	const std::string& do_get_name() const noexcept override;
	int do_get_input_index() const noexcept override;

	void do_set_int32(std::int32_t value) override;
	void do_set_float32(float value) override;
	void do_set_vec2(const float* value) override;
	void do_set_vec4(const float* value) override;
	void do_set_mat4(const float* value) override;
	void do_set_r2_sampler(std::int32_t value) override;

private:
	GlR3rShaderStage& shader_stage_;
	const GlR3rDeviceFeatures& gl_device_features_;

	R3rShaderVarType type_{};
	R3rShaderVarTypeId type_id_{};
	int value_size_{};
	int index_{};
	std::string name_{};
	int input_index_{};
	int gl_location_{};

private:
	void set_value(R3rShaderVarTypeId type_id, const void* value_data);
	void set_value(const void* value_data);
};

// ==========================================================================

using GlR3rShaderVarImplPool = FixedPoolResource<GlR3rShaderVarImpl, R3rLimits::max_shader_vars()>;
GlR3rShaderVarImplPool gl_r3r_shader_var_impl_pool{};

// ==========================================================================

GlR3rShaderVarImpl::GlR3rShaderVarImpl(
	GlR3rShaderStage& shader_stage,
	const GlR3rShaderVarInitParam& param)
try
	:
	shader_stage_{shader_stage},
	gl_device_features_{shader_stage.get_context().get_gl_device_features()}
{
	type_ = param.type;
	type_id_ = param.type_id;
	value_size_ = param.value_size;
	index_ = param.index;
	name_.assign(param.name.get_data(), static_cast<std::size_t>(param.name.get_size()));
	input_index_ = param.input_index;
	gl_location_ = param.gl_location;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rShaderVarImpl::~GlR3rShaderVarImpl() = default;

void* GlR3rShaderVarImpl::operator new(std::size_t size)
try {
	return gl_r3r_shader_var_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::operator delete(void* ptr)
{
	gl_r3r_shader_var_impl_pool.deallocate(ptr);
}

R3rShaderVarType GlR3rShaderVarImpl::do_get_type() const noexcept
{
	return type_;
}

R3rShaderVarTypeId GlR3rShaderVarImpl::do_get_type_id() const noexcept
{
	return type_id_;
}

int GlR3rShaderVarImpl::do_get_index() const noexcept
{
	return index_;
}

const std::string& GlR3rShaderVarImpl::do_get_name() const noexcept
{
	return name_;
}

int GlR3rShaderVarImpl::do_get_input_index() const noexcept
{
	return input_index_;
}

void GlR3rShaderVarImpl::do_set_int32(std::int32_t value)
try {
	set_value(R3rShaderVarTypeId::int32, &value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::do_set_float32(float value)
try {
	set_value(R3rShaderVarTypeId::float32, &value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::do_set_vec2(const float* value)
try {
	set_value(R3rShaderVarTypeId::vec2, value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::do_set_vec4(const float* value)
try {
	set_value(R3rShaderVarTypeId::vec4, value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::do_set_mat4(const float* value)
try {
	set_value(R3rShaderVarTypeId::mat4, value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::do_set_r2_sampler(std::int32_t value)
try {
	set_value(R3rShaderVarTypeId::sampler2d, &value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::set_value(R3rShaderVarTypeId type_id, const void* value_data)
try {
	if (type_id != type_id_)
	{
		BSTONE_THROW_STATIC_SOURCE("Mismatch type.");
	}

	if (!value_data)
	{
		BSTONE_THROW_STATIC_SOURCE("Null value data.");
	}

	const auto value_size = get_unit_size(type_id);

	if (value_size != value_size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Value size mismatch.");
	}

	switch (type_)
	{
		case R3rShaderVarType::sampler:
		case R3rShaderVarType::uniform:
			break;

		default: BSTONE_THROW_STATIC_SOURCE("Changing vertex attribute not supported.");
	}

	if (!gl_device_features_.is_sso_available)
	{
		shader_stage_.set();
	}

	set_value(value_data);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderVarImpl::set_value(const void* value_data)
try {
	auto shader_stage_gl_name = GLuint{};

	if (gl_device_features_.is_sso_available)
	{
		shader_stage_gl_name = shader_stage_.get_gl_name();
	}

	switch (type_id_)
	{
		case R3rShaderVarTypeId::int32:
		case R3rShaderVarTypeId::sampler2d:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniform1iv(
					shader_stage_gl_name, gl_location_, 1, static_cast<const GLint*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniform1iv(gl_location_, 1, static_cast<const GLint*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		case R3rShaderVarTypeId::float32:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniform1fv(
					shader_stage_gl_name, gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniform1fv(gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		case R3rShaderVarTypeId::vec2:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniform2fv(
					shader_stage_gl_name, gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniform2fv(gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		case R3rShaderVarTypeId::vec3:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniform3fv(
					shader_stage_gl_name, gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniform3fv(gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		case R3rShaderVarTypeId::vec4:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniform4fv(
					shader_stage_gl_name, gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniform4fv(gl_location_, 1, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		case R3rShaderVarTypeId::mat4:
			if (gl_device_features_.is_sso_available)
			{
				glProgramUniformMatrix4fv(
					shader_stage_gl_name, gl_location_, 1, GL_FALSE, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}
			else
			{
				glUniformMatrix4fv(gl_location_, 1, GL_FALSE, static_cast<const GLfloat*>(value_data));
				GlR3rError::check_optionally();
			}

			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

GlR3rShaderVarUPtr make_gl_r3r_shader_var(
	GlR3rShaderStage& shader_stage,
	const GlR3rShaderVarInitParam& param)
{
	return std::make_unique<GlR3rShaderVarImpl>(shader_stage, param);
}

} // namespace bstone
