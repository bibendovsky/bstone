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
#include "bstone_exception.h"
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


OglShader::OglShader(
	const RendererShader::CreateParam& param)
	:
	kind_{},
	ogl_resource_{},
	shader_stage_{}
{
	initialize(param);
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

RendererShader::Kind OglShader::get_kind() const
{
	return kind_;
}

void OglShader::initialize(
	const RendererShader::CreateParam& param)
{
	validate_param(param);

	const auto ogl_kind = get_ogl_kind(param.kind_);

	auto ogl_handle = OglShaderUniqueResource{::glCreateShader(ogl_kind)};

	if (!ogl_handle)
	{
		throw Exception{"Failed to create OpenGL shader object."};
	}

	const char* const strings[] =
	{
		static_cast<const char*>(param.source_.data_),
	};

	const GLint lengths[] =
	{
		param.source_.size_,
	};

	::glShaderSource(ogl_handle, 1, strings, lengths);
	assert(!detail::OglRendererUtils::was_errors());

	::glCompileShader(ogl_handle);
	assert(!detail::OglRendererUtils::was_errors());

	auto compile_status = GLint{};

	::glGetShaderiv(ogl_handle, GL_COMPILE_STATUS, &compile_status);
	assert(!detail::OglRendererUtils::was_errors());

	if (compile_status != GL_TRUE)
	{
		auto error_message = std::string{"Failed to compile a shader."};

		const auto ogl_log = OglRendererUtils::get_log(true, ogl_handle);

		if (!ogl_log.empty())
		{
			error_message += '\n';
			error_message += ogl_log;
		}

		throw Exception{std::move(error_message)};
	}

	kind_ = param.kind_;
	ogl_resource_ = std::move(ogl_handle);
}

GLuint OglShader::get_ogl_name() const
{
	return ogl_resource_;
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
			throw Exception{"Invalid kind."};
	}
}

void OglShader::validate_param(
	const RendererShader::CreateParam& param)
{
	switch (param.kind_)
	{
		case RendererShader::Kind::fragment:
		case RendererShader::Kind::vertex:
			break;

		default:
			throw Exception{"Invalid kind."};
	}

	if (param.source_.data_ == nullptr)
	{
		throw Exception{"Null source data."};
	}

	if (param.source_.size_ <= 0)
	{
		throw Exception{"Empty source data."};
	}
}


} // detail
} // bstone
