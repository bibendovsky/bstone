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
// OpenGL 1.x renderer.
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_ogl_1_x_renderer.h"
#include "bstone_ogl_renderer_utils.h"


namespace bstone
{


Ogl1XRenderer::Ogl1XRenderer()
	:
	is_initialized_{},
	error_message_{}
{
}

Ogl1XRenderer::Ogl1XRenderer(
	Ogl1XRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)}
{
	rhs.is_initialized_ = false;
}

Ogl1XRenderer::~Ogl1XRenderer()
{
}

const std::string& Ogl1XRenderer::get_error_message() const
{
	return error_message_;
}

RendererKind Ogl1XRenderer::get_kind() const
{
	return RendererKind::opengl;
}

const std::string& Ogl1XRenderer::get_name() const
{
	static const auto result = std::string{"OGL1"};

	return result;
}

const std::string& Ogl1XRenderer::get_description() const
{
	static const auto result = std::string{"OpenGL 1.x"};

	return result;
}

bool Ogl1XRenderer::probe(
	const RendererPath renderer_path)
{
	if (renderer_path != RendererPath::ogl_1_x)
	{
		error_message_ = "Invalid renderer path value.";

		return false;
	}

	auto is_succeed = true;
	auto sdl_window = SdlWindowPtr{};
	auto sdl_ogl_context = SdlGlContext{};
	auto missing_symbols = OglRendererUtils::Strings{};

	if (is_succeed)
	{
		if (!OglRendererUtils::create_probe_window_and_context(sdl_window, sdl_ogl_context, error_message_))
		{
			is_succeed = false;
		}
	}

	if (is_succeed)
	{
		if (!OglRendererUtils::resolve_symbols_1_1(missing_symbols))
		{
			error_message_ = "Failed to load OpenGL 1.1 symbols.";

			is_succeed = false;
		}
	}

	OglRendererUtils::destroy_window_and_context(sdl_window, sdl_ogl_context);

	return is_succeed;
}

RendererPath Ogl1XRenderer::get_probe_path() const
{
	throw "Not implemented.";
}

bool Ogl1XRenderer::is_initialized() const
{
	throw "Not implemented.";
}

bool Ogl1XRenderer::initialize(
	const RendererPath renderer_path)
{
	throw "Not implemented.";
}

void Ogl1XRenderer::uninitialize()
{
	throw "Not implemented.";
}

RendererPath Ogl1XRenderer::get_path() const
{
	throw "Not implemented.";
}


} // bstone
