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
#include <cassert>
#include "bstone_ogl_renderer_utils.h"


namespace bstone
{


Ogl1XRenderer::Ogl1XRenderer()
	:
	is_initialized_{},
	error_message_{},
	probe_renderer_path_{},
	sdl_window_{},
	sdl_gl_context_{},
	palette_{},
	two_d_projection_matrix_{},
	vertex_buffers_{},
	texture_buffer_{},
	textures_2d_{}
{
}

Ogl1XRenderer::Ogl1XRenderer(
	Ogl1XRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	probe_renderer_path_{std::move(rhs.probe_renderer_path_)},
	sdl_window_{std::move(rhs.sdl_window_)},
	sdl_gl_context_{std::move(rhs.sdl_gl_context_)},
	palette_{std::move(rhs.palette_)},
	two_d_projection_matrix_{std::move(rhs.two_d_projection_matrix_)},
	vertex_buffers_{std::move(rhs.vertex_buffers_)},
	texture_buffer_{std::move(rhs.texture_buffer_)},
	textures_2d_{std::move(rhs.textures_2d_)}
{
	rhs.is_initialized_ = false;
	rhs.sdl_window_ = nullptr;
	rhs.sdl_gl_context_ = nullptr;
}

Ogl1XRenderer::~Ogl1XRenderer()
{
	uninitialize_internal(true);
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
	static const auto result = std::string{"ogl1"};

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
	return probe_or_initialize(true, renderer_path, {});
}

RendererPath Ogl1XRenderer::get_probe_path() const
{
	return probe_renderer_path_;
}

bool Ogl1XRenderer::is_initialized() const
{
	return is_initialized_;
}

bool Ogl1XRenderer::initialize(
	const RendererInitializeParam& param)
{
	return probe_or_initialize(false, {}, param);
}

void Ogl1XRenderer::uninitialize()
{
	uninitialize_internal();
}

RendererPath Ogl1XRenderer::get_path() const
{
	if (!is_initialized_)
	{
		return RendererPath::none;
	}

	return RendererPath::ogl_1_x;
}

void Ogl1XRenderer::set_2d_projection_matrix(
	const int width,
	const int height)
{
	assert(is_initialized_);
	assert(width > 0);
	assert(height > 0);

	const auto& new_matrix = OglRendererUtils::build_2d_projection_matrix(width, height);

	if (two_d_projection_matrix_ == new_matrix)
	{
		return;
	}

	two_d_projection_matrix_ = new_matrix;
}

Renderer::ObjectId Ogl1XRenderer::vertex_buffer_create(
	const int vertex_count)
{
	assert(is_initialized_);
	assert(vertex_count > 0 && (vertex_count % 3) == 0);

	const auto vertex_buffer_size = static_cast<int>(vertex_count * sizeof(RendererVertex));

	vertex_buffers_.emplace_back();
	auto& vertex_buffer = vertex_buffers_.back();

	vertex_buffer.resize(vertex_buffer_size);

	return &vertex_buffer;
}

void Ogl1XRenderer::vertex_buffer_destroy(
	ObjectId id)
{
	assert(is_initialized_);
	assert(id != NullObjectId);

	vertex_buffers_.remove_if(
		[=](const auto& item)
		{
			return id == &item;
		}
	);
}

void Ogl1XRenderer::vertex_buffer_update(
	ObjectId id,
	const int offset,
	const int count,
	const RendererVertex* const vertices)
{
	assert(is_initialized_);
	assert(id != NullObjectId);
	assert(offset >= 0);
	assert(count > 0);
	assert(vertices != nullptr);

	auto& vertex_buffer = *static_cast<VertexBuffer*>(id);
	const auto max_vertex_count = static_cast<int>(vertex_buffer.size());

	assert(offset >= max_vertex_count);
	assert(count > max_vertex_count);
	assert(count <= (max_vertex_count - offset));

	std::uninitialized_copy_n(vertices, count, vertex_buffer.begin() + offset);
}

bool Ogl1XRenderer::probe_or_initialize(
	const bool is_probe,
	const RendererPath probe_renderer_path,
	const RendererInitializeParam& param)
{
	const auto renderer_path = (is_probe ? probe_renderer_path : param.renderer_path_);

	if (renderer_path != RendererPath::ogl_1_x)
	{
		error_message_ = "Invalid renderer path value.";

		return false;
	}

	auto is_succeed = true;
	auto sdl_window = SdlWindowPtr{};
	auto sdl_gl_context = SdlGlContext{};

	if (is_succeed)
	{
		if (is_probe)
		{
			if (!OglRendererUtils::create_probe_window_and_context(sdl_window, sdl_gl_context, error_message_))
			{
				is_succeed = false;
			}
		}
		else
		{
			auto window_param = RendererUtilsCreateWindowParam{};
			window_param.is_opengl_ = true;
			window_param.window_ = param.window_;

			if (!OglRendererUtils::create_window_and_context(window_param, sdl_window, sdl_gl_context, error_message_))
			{
				is_succeed = false;
			}
		}
	}

	if (is_succeed)
	{
		if (!OglRendererUtils::resolve_symbols_1_1())
		{
			error_message_ = "Failed to load OpenGL 1.1 symbols.";

			is_succeed = false;
		}
	}

	if (!is_succeed)
	{
		OglRendererUtils::destroy_window_and_context(sdl_window, sdl_gl_context);

		return false;
	}

	is_initialized_ = true;
	sdl_window_ = sdl_window;
	sdl_gl_context_ = sdl_gl_context;

	return true;
}

Renderer::ObjectId Ogl1XRenderer::texture_2d_create(
	const RendererTextureCreateParam& param)
{
	assert(is_initialized_);
	assert(RendererUtils::validate_renderer_texture_create_param(param, error_message_));

	auto texture_2d = Texture2d{};
	texture_2d.width_ = param.width_;
	texture_2d.height_ = param.height_;

	texture_2d.actual_width_ = RendererUtils::find_nearest_pot_value(param.width_);
	texture_2d.actual_height_ = RendererUtils::find_nearest_pot_value(param.height_);

	texture_2d.actual_u_ =
		static_cast<float>(param.width_) / static_cast<float>(texture_2d.actual_width_);

	texture_2d.actual_v_ =
		static_cast<float>(param.height_) / static_cast<float>(texture_2d.actual_height_);

	texture_2d.indexed_data_ = param.indexed_data_;

	auto ogl_id = GLuint{};

	::glGenTextures(1, &ogl_id);
	assert(!OglRendererUtils::was_errors());
	assert(ogl_id != 0);

	::glBindTexture(GL_TEXTURE_2D, ogl_id);
	assert(!OglRendererUtils::was_errors());

	::glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB8,
		texture_2d.actual_width_,
		texture_2d.actual_height_,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		nullptr);

	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	assert(!OglRendererUtils::was_errors());

	update_indexed_texture(0, texture_2d);

	texture_2d.ogl_id_ = ogl_id;

	textures_2d_.push_back(texture_2d);

	return &textures_2d_.back();
}

void Ogl1XRenderer::texture_2d_destroy(
	ObjectId texture_id)
{
	assert(is_initialized_);
	assert(texture_id != NullObjectId);

	const auto texture_end_it = textures_2d_.end();

	auto texture_2d_it = std::find_if(
		textures_2d_.begin(),
		texture_end_it,
		[=](const auto& item)
		{
			return texture_id == &item;
		}
	);

	assert(texture_2d_it != texture_end_it);

	::glDeleteTextures(1, &texture_2d_it->ogl_id_);
	assert(!OglRendererUtils::was_errors());

	static_cast<void>(textures_2d_.erase(texture_2d_it));
}

void Ogl1XRenderer::texture_2d_update(
	ObjectId texture_id,
	const RendererTextureUpdateParam& param)
{
	assert(is_initialized_);
	assert(texture_id != NullObjectId);
	assert(RendererUtils::validate_renderer_texture_update_param(param, error_message_));

	const auto texture_end_it = textures_2d_.end();

	auto texture_2d_it = std::find_if(
		textures_2d_.begin(),
		texture_end_it,
		[=](const auto& item)
		{
			return texture_id == &item;
		}
	);

	assert(texture_2d_it != texture_end_it);

	auto& texture_2d = *texture_2d_it;
	texture_2d.indexed_data_ = param.indexed_data_;

	update_indexed_texture(0, texture_2d);
}

void Ogl1XRenderer::uninitialize_internal(
	const bool is_dtor)
{
	if (sdl_gl_context_)
	{
		static_cast<void>(OglRendererUtils::make_context_current(sdl_window_, nullptr, error_message_));
	}

	OglRendererUtils::destroy_window_and_context(sdl_window_, sdl_gl_context_);

	if (!is_dtor)
	{
		palette_ = {};
		two_d_projection_matrix_ = {};
		texture_buffer_.clear();
	}

	for (const auto& texture_2d : textures_2d_)
	{
		::glDeleteTextures(1, &texture_2d.ogl_id_);
		assert(!OglRendererUtils::was_errors());
	}

	textures_2d_.clear();
}

void Ogl1XRenderer::update_indexed_texture(
	const int mipmap_level,
	const Texture2d& texture_2d)
{
	assert(mipmap_level == 0);

	const auto area = texture_2d.actual_width_ * texture_2d.actual_height_;

	texture_buffer_.clear();
	texture_buffer_.resize(area);

	if (texture_2d.width_ == texture_2d.actual_width_ &&
		texture_2d.height_ == texture_2d.actual_height_)
	{
		for (int i = 0; i < area; ++i)
		{
			texture_buffer_[i] = palette_[texture_2d.indexed_data_[i]];
		}
	}
	else
	{
		auto src_index = 0;
		auto dst_base_index = 0;

		for (int h = 0; h < texture_2d.height_; ++h)
		{
			auto dst_index = dst_base_index;

			for (int w = 0; w < texture_2d.width_; ++w)
			{
				texture_buffer_[dst_index + w] = palette_[texture_2d.indexed_data_[src_index]];

				++src_index;
			}

			dst_base_index += texture_2d.actual_width_;
		}
	}

	::glTexSubImage2D(
		GL_TEXTURE_2D,
		0, // level
		0, // xoffset
		0, // yoffset
		texture_2d.actual_width_, // width
		texture_2d.actual_height_, // height
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		texture_buffer_.data() // pixels
	);

	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::update_indexed_textures()
{
	for (const auto& texture_2d : textures_2d_)
	{
		update_indexed_texture(0, texture_2d);
	}
}


} // bstone
