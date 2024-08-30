/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_shader.h"
#include "bstone_gl_r3r_shader_stage.h"
#include "bstone_gl_r3r_utils.h"


namespace bstone {

GlR3rShader::GlR3rShader() = default;

GlR3rShader::~GlR3rShader() = default;

// ==========================================================================

class GlR3rShaderImpl final : public GlR3rShader
{
public:
	GlR3rShaderImpl(const R3rShaderInitParam& param);
	~GlR3rShaderImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderType do_get_type() const noexcept override;

public:
	GLuint get_gl_name() const noexcept override;

	void attach_to_shader_stage(GlR3rShaderStage* shader_stage) override;

private:
	R3rShaderType type_{};

	struct ShaderDeleter
	{
		void operator()(GLuint gl_name) noexcept;
	};

	using ShaderResource = UniqueResource<GLuint, ShaderDeleter>;

	ShaderResource shader_resource_{};
	GlR3rShaderStage* shader_stage_{};

private:
	GLenum get_gl_type(R3rShaderType type);

	void validate(const R3rShaderInitParam& param);
};

// ==========================================================================

using GlR3rShaderImplPool = FixedPoolResource<GlR3rShaderImpl, R3rLimits::max_shaders()>;
GlR3rShaderImplPool gl_r3r_shader_impl_pool{};

// ==========================================================================

GlR3rShaderImpl::GlR3rShaderImpl(const R3rShaderInitParam& param)
try {
	validate(param);

	const auto gl_type = get_gl_type(param.type);

	shader_resource_.reset(glCreateShader(gl_type));

	if (shader_resource_.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create an object.");
	}

	const char* const strings[] = {static_cast<const char*>(param.source.data)};
	const GLint lengths[] = {param.source.size};

	glShaderSource(shader_resource_.get(), 1, strings, lengths);
	GlR3rError::check_optionally();

	glCompileShader(shader_resource_.get());
	GlR3rError::check_optionally();

	auto compile_status = GLint{};

	glGetShaderiv(shader_resource_.get(), GL_COMPILE_STATUS, &compile_status);
	GlR3rError::check_optionally();

	if (compile_status != GL_TRUE)
	{
		auto error_message = std::string{"Failed to compile a shader."};

		const auto gl_log = GlR3rUtils::get_log(true, shader_resource_.get());

		if (!gl_log.empty())
		{
			error_message += '\n';
			error_message += gl_log;
		}

		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}

	type_ = param.type;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rShaderImpl::~GlR3rShaderImpl()
{
	if (shader_stage_)
	{
		switch (type_)
		{
			case R3rShaderType::fragment:
				shader_stage_->detach_fragment_shader();
				break;

			case R3rShaderType::vertex:
				shader_stage_->detach_vertex_shader();
				break;

			default:
				BSTONE_ASSERT(false && "Unsupported type.");
				break;
		}
	}
}

void* GlR3rShaderImpl::operator new(std::size_t size)
try {
	return gl_r3r_shader_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderImpl::operator delete(void* ptr)
{
	gl_r3r_shader_impl_pool.deallocate(ptr);
}

R3rShaderType GlR3rShaderImpl::do_get_type() const noexcept
{
	return type_;
}

void GlR3rShaderImpl::ShaderDeleter::operator()(GLuint gl_name) noexcept
{
	glDeleteShader(gl_name);
	GlR3rError::ensure_no_errors_assert();
}

GLuint GlR3rShaderImpl::get_gl_name() const noexcept
{
	return shader_resource_.get();
}

void GlR3rShaderImpl::attach_to_shader_stage(GlR3rShaderStage* shader_stage)
{
	shader_stage_ = shader_stage;
}

GLenum GlR3rShaderImpl::get_gl_type(R3rShaderType type)
try {
	switch (type)
	{
		case R3rShaderType::fragment: return GL_FRAGMENT_SHADER;
		case R3rShaderType::vertex: return GL_VERTEX_SHADER;
		default: BSTONE_THROW_STATIC_SOURCE("Invalid type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rShaderImpl::validate(const R3rShaderInitParam& param)
try {
	switch (param.type)
	{
		case R3rShaderType::fragment:
		case R3rShaderType::vertex:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid type.");
	}

	if (param.source.data == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null source data.");
	}

	if (param.source.size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Empty source data.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

GlR3rShaderUPtr make_gl_r3r_shader(const R3rShaderInitParam& param)
{
	return std::make_unique<GlR3rShaderImpl>(param);
}

} // namespace bstone
