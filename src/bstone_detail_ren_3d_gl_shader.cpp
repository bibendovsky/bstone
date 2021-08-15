/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// OpenGL shader (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_detail_ren_3d_gl_shader.h"

#include <cassert>

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_shader_stage.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlShaderException
//

class Ren3dGlShaderException :
	public Exception
{
public:
	explicit Ren3dGlShaderException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_SHADER", message}
	{
	}
}; // Ren3dGlShaderException

//
// Ren3dGlShaderException
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderImpl
//

class Ren3dGlShaderImpl final :
	public Ren3dGlShader
{
public:
	Ren3dGlShaderImpl(
		const Ren3dCreateShaderParam& param);

	~Ren3dGlShaderImpl() override;


	Ren3dShaderKind get_kind() const noexcept override;

	GLuint get_gl_name() const noexcept override;

	void attach_to_shader_stage(
		const Ren3dGlShaderStagePtr shader_stage) override;


private:
	Ren3dShaderKind kind;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void shader_deleter(
		GLuint gl_name) noexcept;

	using ShaderResource = UniqueResource<GLuint, shader_deleter>;

	ShaderResource shader_resource_;

	Ren3dGlShaderStagePtr shader_stage_;


	GLenum get_gl_kind(
		const Ren3dShaderKind kind);

	void validate(
		const Ren3dCreateShaderParam& param);
}; // Ren3dGlShaderImpl

//
// Ren3dGlShaderImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderImpl
//

Ren3dGlShaderImpl::Ren3dGlShaderImpl(
	const Ren3dCreateShaderParam& param)
try
	:
	kind{},
	shader_resource_{},
	shader_stage_{}
{
	validate(param);

	const auto gl_kind = get_gl_kind(param.kind);

	shader_resource_.reset(glCreateShader(gl_kind));

	if (!shader_resource_)
	{
		fail("Failed to create an object.");
	}

	const char* const strings[] =
	{
		static_cast<const char*>(param.source_.data_),
	};

	const GLint lengths[] =
	{
		param.source_.size_,
	};

	glShaderSource(shader_resource_.get(), 1, strings, lengths);
	Ren3dGlError::ensure_debug();

	glCompileShader(shader_resource_.get());
	Ren3dGlError::ensure_debug();

	auto compile_status = GLint{};

	glGetShaderiv(shader_resource_.get(), GL_COMPILE_STATUS, &compile_status);
	Ren3dGlError::ensure_debug();

	if (compile_status != GL_TRUE)
	{
		auto error_message = std::string{"Failed to compile a shader."};

		const auto gl_log = Ren3dGlUtils::get_log(true, shader_resource_.get());

		if (!gl_log.empty())
		{
			error_message += '\n';
			error_message += gl_log;
		}

		fail(error_message.c_str());
	}

	kind = param.kind;
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlShaderImpl::~Ren3dGlShaderImpl()
{
	if (shader_stage_)
	{
		switch (kind)
		{
			case Ren3dShaderKind::fragment:
				shader_stage_->detach_fragment_shader();
				break;

			case Ren3dShaderKind::vertex:
				shader_stage_->detach_vertex_shader();
				break;

			default:
				assert(!"Unsupported kind.");
				break;
		}
	}
}

Ren3dShaderKind Ren3dGlShaderImpl::get_kind() const noexcept
{
	return kind;
}

[[noreturn]]
void Ren3dGlShaderImpl::fail(
	const char* message)
{
	throw Ren3dGlShaderException{message};
}

[[noreturn]]
void Ren3dGlShaderImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlShaderException{message});
}

void Ren3dGlShaderImpl::shader_deleter(
	GLuint gl_name) noexcept
{
	glDeleteShader(gl_name);
	Ren3dGlError::ensure_assert();
}

GLuint Ren3dGlShaderImpl::get_gl_name() const noexcept
{
	return shader_resource_.get();
}

void Ren3dGlShaderImpl::attach_to_shader_stage(
	const Ren3dGlShaderStagePtr shader_stage)
{
	shader_stage_ = shader_stage;
}

GLenum Ren3dGlShaderImpl::get_gl_kind(
	const Ren3dShaderKind kind)
try
{
	switch (kind)
	{
		case Ren3dShaderKind::fragment:
			return GL_FRAGMENT_SHADER;

		case Ren3dShaderKind::vertex:
			return GL_VERTEX_SHADER;

		default:
			fail("Invalid kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderImpl::validate(
	const Ren3dCreateShaderParam& param)
try
{
	switch (param.kind)
	{
		case Ren3dShaderKind::fragment:
		case Ren3dShaderKind::vertex:
			break;

		default:
			fail("Invalid kind.");
	}

	if (param.source_.data_ == nullptr)
	{
		fail("Null source data.");
	}

	if (param.source_.size_ <= 0)
	{
		fail("Empty source data.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlShaderImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderFactory
//

Ren3dGlShaderUPtr Ren3dGlShaderFactory::create(
	const Ren3dCreateShaderParam& param)
{
	return std::make_unique<Ren3dGlShaderImpl>(param);
}

//
// Ren3dGlShaderFactory
// ==========================================================================


} // detail
} // bstone
