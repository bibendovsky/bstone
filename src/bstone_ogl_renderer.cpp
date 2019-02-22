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
#include <cassert>
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
	const RendererInitializeParam& param)
{
	if (!RendererUtils::validate_renderer_initialize_param(param, error_message_))
	{
		error_message_ = "Failed to validate an initialize param. " + error_message_;

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

void OglRenderer::window_show(
	const bool is_visible)
{
	assert(is_initialized_);

	renderer_->window_show(is_visible);
}

void OglRenderer::color_buffer_set_clear_color(
	const RendererColor32& color)
{
	assert(is_initialized_);

	renderer_->color_buffer_set_clear_color(color);
}

void OglRenderer::clear_buffers()
{
	assert(is_initialized_);

	renderer_->clear_buffers();
}

void OglRenderer::present()
{
	assert(is_initialized_);

	renderer_->present();
}

void OglRenderer::set_2d_projection_matrix(
	const int width,
	const int height)
{
	assert(is_initialized_);
	assert(width > 0);
	assert(height > 0);

	renderer_->set_2d_projection_matrix(width, height);
}

RendererIndexBufferHandle OglRenderer::index_buffer_create(
	const int index_count)
{
	assert(is_initialized_);
	assert(index_count > 0);

	return renderer_->index_buffer_create(index_count);
}

void OglRenderer::index_buffer_destroy(
	RendererIndexBufferHandle id)
{
	assert(is_initialized_);
	assert(id);

	renderer_->index_buffer_destroy(id);
}

void OglRenderer::index_buffer_update(
	RendererIndexBufferHandle id,
	const int offset,
	const int count,
	const void* const indices)
{
	assert(is_initialized_);
	assert(id);
	assert(offset >= 0);
	assert(count > 0);
	assert(indices != nullptr);

	renderer_->index_buffer_update(id, offset, count, indices);
}

RendererVertexBufferHandle OglRenderer::vertex_buffer_create(
	const int vertex_count)
{
	assert(is_initialized_);
	assert(vertex_count > 0 && (vertex_count % 4) == 0);

	return renderer_->vertex_buffer_create(vertex_count);
}

void OglRenderer::vertex_buffer_destroy(
	RendererVertexBufferHandle id)
{
	assert(is_initialized_);
	assert(id);

	renderer_->vertex_buffer_destroy(id);
}

void OglRenderer::vertex_buffer_update(
	RendererVertexBufferHandle id,
	const int offset,
	const int count,
	const RendererVertex* const vertices)
{
	assert(is_initialized_);
	assert(id);
	assert(offset >= 0);
	assert(count > 0);
	assert(vertices != nullptr);

	renderer_->vertex_buffer_update(id, offset, count, vertices);
}

RendererTexture2dHandle OglRenderer::texture_2d_create(
	const RendererTextureCreateParam& param)
{
	assert(is_initialized_);
	assert(RendererUtils::validate_renderer_texture_create_param(param, error_message_));

	return renderer_->texture_2d_create(param);
}

void OglRenderer::texture_2d_destroy(
	RendererTexture2dHandle texture_handle)
{
	assert(is_initialized_);
	assert(texture_handle);

	renderer_->texture_2d_destroy(texture_handle);
}

void OglRenderer::texture_2d_update(
	RendererTexture2dHandle texture_handle,
	const RendererTextureUpdateParam& param)
{
	assert(is_initialized_);
	assert(texture_handle);
	assert(RendererUtils::validate_renderer_texture_update_param(param, error_message_));

	renderer_->texture_2d_update(texture_handle, param);
}

void OglRenderer::execute_command_sets(
	const RendererCommandSets& command_sets)
{
	assert(is_initialized_);
	assert(!command_sets.empty());

	renderer_->execute_command_sets(command_sets);
}

void OglRenderer::uninitialize_internal(
	const bool is_dtor)
{
	is_initialized_ = false;
	renderer_ = nullptr;
}


} // bstone
