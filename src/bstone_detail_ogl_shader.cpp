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
// OpenGL shader (implementation).
//
// !!! Internal usage only. !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


OglShader::OglShader()
	:
	error_message_{},
	kind_{},
	ogl_name_raii_{},
	shader_stage_{}
{
}

OglShader::~OglShader()
{
	if (shader_stage_ == nullptr)
	{
		return;
	}

	switch (kind_)
	{
		case Kind::fragment:
			shader_stage_->detach_fragment_shader();
			break;

		case Kind::vertex:
			shader_stage_->detach_vertex_shader();
			break;

		default:
			assert(!"Unsupported kind.");

			break;
	}
}

bool OglShader::is_initialized() const
{
	return ogl_name_raii_ != nullptr;
}

const std::string& OglShader::get_error_message() const
{
	return error_message_;
}

RendererShader::Kind OglShader::get_kind() const
{
	return kind_;
}

void OglShader::initialize(
	const RendererShader::CreateParam& param)
{
	if (!validate_param(param))
	{
		return;
	}

	const auto ogl_kind = get_ogl_kind(param.kind_);

	auto ogl_name_raii = OglShaderHandle{::glCreateShader(ogl_kind)};

	if (ogl_name_raii == 0)
	{
		error_message_ = "Failed to create OpenGL shader object.";

		return;
	}

	const char* const strings[] =
	{
		static_cast<const char*>(param.source_.data_),
	};

	const GLint lengths[] =
	{
		param.source_.size_,
	};

	::glShaderSource(ogl_name_raii.get(), 1, strings, lengths);
	assert(!detail::OglRendererUtils::was_errors());

	::glCompileShader(ogl_name_raii.get());
	assert(!detail::OglRendererUtils::was_errors());

	auto compile_status = GLint{};

	::glGetShaderiv(ogl_name_raii.get(), GL_COMPILE_STATUS, &compile_status);
	assert(!detail::OglRendererUtils::was_errors());

	if (compile_status != GL_TRUE)
	{
		error_message_ = "Failed to compile a shader.";

		const auto ogl_log = OglRendererUtils::get_log(true, ogl_name_raii.get());

		if (!ogl_log.empty())
		{
			error_message_ += '\n';
			error_message_ += ogl_log;
		}

		return;
	}

	kind_ = param.kind_;
	ogl_name_raii_ = std::move(ogl_name_raii);
}

GLuint OglShader::get_ogl_name() const
{
	return ogl_name_raii_.get();
}

void OglShader::attach_to_shader_stage(
	const OglShaderStagePtr shader_stage)
{
	shader_stage_ = shader_stage;
}

GLenum OglShader::get_ogl_kind(
	const Kind kind)
{
	switch (kind)
	{
		case RendererShader::Kind::fragment:
			return GL_FRAGMENT_SHADER;

		case RendererShader::Kind::vertex:
			return GL_VERTEX_SHADER;

		default:
			error_message_ = "Invalid kind.";

			return GL_NONE;
	}
}

bool OglShader::validate_param(
	const RendererShader::CreateParam& param)
{
	switch (param.kind_)
	{
		case RendererShader::Kind::fragment:
		case RendererShader::Kind::vertex:
			break;

		default:
			error_message_ = "Invalid kind.";

			return false;
	}

	if (param.source_.data_ == nullptr)
	{
		error_message_ = "Null source data.";

		return false;
	}

	if (param.source_.size_ <= 0)
	{
		error_message_ = "Empty source data.";

		return false;
	}

	return true;
}


} // detail
} // bstone
