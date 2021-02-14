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
// Ren3dGlShaderImplCreateException
//

class Ren3dGlShaderImplCreateException :
	public Exception
{
public:
	explicit Ren3dGlShaderImplCreateException(
		const char* const message)
		:
		Exception{get_prefix() + message}
	{
	}

	explicit Ren3dGlShaderImplCreateException(
		const std::string& message)
		:
		Exception{get_prefix() + message}
	{
	}


private:
	static const std::string& get_prefix()
	{
		static const auto prefix = std::string{"[REN_3D_SHDR_INIT] "};

		return prefix;
	}
}; // Ren3dGlShaderImplCreateException

//
// Ren3dGlShaderImplCreateException
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
	Ren3dShaderKind kind_;

	static void shader_deleter(
		const GLuint& gl_name) noexcept;

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
	:
	kind_{},
	shader_resource_{},
	shader_stage_{}
{
	validate(param);

	const auto gl_kind = get_gl_kind(param.kind_);

	shader_resource_.reset(glCreateShader(gl_kind));

	if (!shader_resource_)
	{
		throw Ren3dGlShaderImplCreateException{"Failed to create an object."};
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

		throw Ren3dGlShaderImplCreateException{error_message};
	}

	kind_ = param.kind_;
}

Ren3dGlShaderImpl::~Ren3dGlShaderImpl()
{
	if (shader_stage_)
	{
		switch (kind_)
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
	return kind_;
}

void Ren3dGlShaderImpl::shader_deleter(
	const GLuint& gl_name) noexcept
{
	glDeleteShader(gl_name);
	Ren3dGlError::ensure_debug();
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
{
	switch (kind)
	{
		case Ren3dShaderKind::fragment:
			return GL_FRAGMENT_SHADER;

		case Ren3dShaderKind::vertex:
			return GL_VERTEX_SHADER;

		default:
			throw Ren3dGlShaderImplCreateException{"Invalid kind."};
	}
}

void Ren3dGlShaderImpl::validate(
	const Ren3dCreateShaderParam& param)
{
	switch (param.kind_)
	{
		case Ren3dShaderKind::fragment:
		case Ren3dShaderKind::vertex:
			break;

		default:
			throw Ren3dGlShaderImplCreateException{"Invalid kind."};
	}

	if (param.source_.data_ == nullptr)
	{
		throw Ren3dGlShaderImplCreateException{"Null source data."};
	}

	if (param.source_.size_ <= 0)
	{
		throw Ren3dGlShaderImplCreateException{"Empty source data."};
	}
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
