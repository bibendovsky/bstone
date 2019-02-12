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
// OpenGL renderer.
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_ogl_renderer.h"
#include "bstone_ogl_1_x_renderer.h"


namespace bstone
{


OglRenderer::OglRenderer()
	:
	is_initialized_{},
	error_message_{},
	probe_renderer_path_{},
	renderer_path_{},
	renderer_{}
{
}

OglRenderer::OglRenderer(
	OglRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	probe_renderer_path_{std::move(rhs.probe_renderer_path_)},
	renderer_path_{std::move(rhs.renderer_path_)},
	renderer_{std::move(rhs.renderer_)}
{
	rhs.is_initialized_ = false;
}

OglRenderer::~OglRenderer()
{
}

const std::string& OglRenderer::get_error_message() const
{
	return error_message_;
}

RendererKind OglRenderer::get_kind() const
{
	return RendererKind::opengl;
}

const std::string& OglRenderer::get_name() const
{
	static const auto default_name = std::string{"OpenGL"};

	if (is_initialized_)
	{
		return renderer_->get_name();
	}

	return default_name;
}

const std::string& OglRenderer::get_description() const
{
	static const auto default_description = std::string{"OpenGL"};

	if (is_initialized_)
	{
		return renderer_->get_description();
	}

	return default_description;
}

bool OglRenderer::probe(
	const RendererPath renderer_path)
{
	if (renderer_path == RendererPath::none)
	{
		error_message_ = "No render path.";

		return false;
	}

	if (renderer_path == RendererPath::autodetect)
	{
		if (probe_ogl_x<Ogl1XRenderer>(RendererPath::ogl_1_x))
		{
			return true;
		}

		return false;
	}
	else
	{
		switch (renderer_path)
		{
		case RendererPath::ogl_1_x:
			return probe_ogl_x<Ogl1XRenderer>(RendererPath::ogl_1_x);

		default:
			return false;
		}
	}

	return false;
}

RendererPath OglRenderer::get_probe_path() const
{
	return probe_renderer_path_;
}

bool OglRenderer::is_initialized() const
{
	return is_initialized_;
}

bool OglRenderer::initialize(
	const RendererPath renderer_path)
{
	if (renderer_path == RendererPath::none)
	{
		error_message_ = "No render path.";

		return false;
	}

	switch (renderer_path)
	{
	case RendererPath::autodetect:
	{
		if (initialize_ogl_x<Ogl1XRenderer>(renderer_path))
		{
			return true;
		}

		error_message_ = "Failed to initialize any renderer path.";

		return false;
	}

	case RendererPath::ogl_1_x:
		return initialize_ogl_x<Ogl1XRenderer>(renderer_path);

	default:
		error_message_ = "Unsupported renderer path.";

		return false;
	}
}

void OglRenderer::uninitialize()
{
	is_initialized_ = false;

	if (renderer_)
	{
		renderer_->uninitialize();

		renderer_ = nullptr;
	}
}

RendererPath OglRenderer::get_path() const
{
	return renderer_path_;
}


} // bstone
