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
#include "bstone_detail_ogl_renderer.h"
#include <cassert>
#include "bstone_detail_ogl_1_x_renderer.h"


namespace bstone
{
namespace detail
{


OglRenderer::OglRenderer()
	:
	is_initialized_{},
	error_message_{},
	renderer_probe_{},
	renderer_path_{},
	renderer_{}
{
}

OglRenderer::OglRenderer(
	OglRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	renderer_probe_{std::move(rhs.renderer_probe_)},
	renderer_path_{std::move(rhs.renderer_path_)},
	renderer_{std::move(rhs.renderer_)}
{
	rhs.is_initialized_ = false;
}

OglRenderer::~OglRenderer()
{
	uninitialize_internal(true);
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
			if (probe_ogl_x<Ogl1XRenderer>(RendererPath::ogl_1_x))
			{
				return true;
			}

			return false;

		default:
			return false;
		}
	}

	return false;
}

const RendererProbe& OglRenderer::probe_get() const
{
	return renderer_probe_;
}

bool OglRenderer::is_initialized() const
{
	return is_initialized_;
}

bool OglRenderer::initialize(
	const RendererInitializeParam& param)
{
	auto renderer_utils = RendererUtils{};

	if (!renderer_utils.validate_initialize_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	switch (param.renderer_path_)
	{
	case RendererPath::ogl_1_x:
		return initialize_ogl_x<Ogl1XRenderer>(param);

	default:
		error_message_ = "Unsupported renderer path.";

		return false;
	}
}

void OglRenderer::uninitialize()
{
	uninitialize_internal();
}

RendererPath OglRenderer::get_path() const
{
	return renderer_path_;
}

const RendererDeviceFeatures& OglRenderer::device_get_features() const
{
	assert(is_initialized_);
	assert(renderer_);

	return renderer_->device_get_features();
}

bool OglRenderer::device_is_lost() const
{
	assert(is_initialized_);
	assert(renderer_);

	return renderer_->device_is_lost();
}

bool OglRenderer::device_is_ready_to_reset() const
{
	assert(is_initialized_);
	assert(renderer_);

	return renderer_->device_is_ready_to_reset();
}

void OglRenderer::device_reset()
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->device_reset();
}

void OglRenderer::window_show(
	const bool is_visible)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->window_show(is_visible);
}

void OglRenderer::color_buffer_set_clear_color(
	const RendererColor32& color)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->color_buffer_set_clear_color(color);
}

void OglRenderer::clear_buffers()
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->clear_buffers();
}

void OglRenderer::present()
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->present();
}

void OglRenderer::palette_update(
	const RendererPalette& palette)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->palette_update(palette);
}

RendererIndexBufferPtr OglRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	assert(is_initialized_);
	assert(renderer_);

	auto ib = renderer_->index_buffer_create(param);

	if (!ib)
	{
		error_message_ = renderer_->get_error_message();
	}

	return ib;
}

void OglRenderer::index_buffer_destroy(
	RendererIndexBufferPtr index_buffer)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->index_buffer_destroy(index_buffer);
}

RendererVertexBufferPtr OglRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	assert(is_initialized_);
	assert(renderer_);

	auto vb = renderer_->vertex_buffer_create(param);

	if (!vb)
	{
		error_message_ = renderer_->get_error_message();
	}

	return vb;
}

void OglRenderer::vertex_buffer_destroy(
	RendererVertexBufferPtr vertex_buffer)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->vertex_buffer_destroy(vertex_buffer);
}

RendererTexture2dPtr OglRenderer::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	assert(is_initialized_);
	assert(renderer_);

	auto t2d = renderer_->texture_2d_create(param);

	if (!t2d)
	{
		error_message_ = renderer_->get_error_message();
	}

	return t2d;
}

void OglRenderer::texture_2d_destroy(
	RendererTexture2dPtr texture_2d)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->texture_2d_destroy(texture_2d);
}

RendererSamplerPtr OglRenderer::sampler_create(
	const RendererSamplerCreateParam& param)
{
	assert(is_initialized_);
	assert(renderer_);

	auto sampler = renderer_->sampler_create(param);

	if (!sampler)
	{
		error_message_ = renderer_->get_error_message();
	}

	return sampler;
}

void OglRenderer::sampler_destroy(
	RendererSamplerPtr sampler)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->sampler_destroy(sampler);
}

RendererVertexInputPtr OglRenderer::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	assert(is_initialized_);
	assert(renderer_);

	auto vertex_input = renderer_->vertex_input_create(param);

	if (!vertex_input)
	{
		error_message_ = renderer_->get_error_message();
	}

	return vertex_input;
}

void OglRenderer::vertex_input_destroy(
	RendererVertexInputPtr vertex_input)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->vertex_input_destroy(vertex_input);
}

void OglRenderer::execute_commands(
	const RendererCommandManagerPtr command_manager)
{
	assert(is_initialized_);
	assert(renderer_);

	renderer_->execute_commands(command_manager);
}

void OglRenderer::uninitialize_internal(
	const bool is_dtor)
{
	is_initialized_ = false;
	renderer_ = nullptr;
}


} // detail
} // bstone
