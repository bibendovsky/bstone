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
#include "bstone_detail_ogl_1_x_renderer.h"
#include <cassert>
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ogl1XRenderer::Texture2d
//

Ogl1XRenderer::Texture2d::Texture2d(
	Ogl1XRendererPtr renderer)
	:
	renderer_{renderer},
	error_message_{},
	is_npot_{},
	is_rgba_{},
	is_indexed_{},
	is_indexed_sprite_{},
	has_rgba_alpha_{},
	is_generate_mipmaps_{},
	width_{},
	height_{},
	actual_width_{},
	actual_height_{},
	mipmap_count_{},
	indexed_pixels_{},
	indexed_palette_{},
	indexed_alphas_{},
	indexed_sprite_{},
	rgba_pixels_{},
	sampler_state_{},
	ogl_id_{}
{
	assert(renderer_);
}

Ogl1XRenderer::Texture2d::~Texture2d()
{
	uninitialize_internal();
}

void Ogl1XRenderer::Texture2d::update(
	const RendererTexture2dUpdateParam& param)
{
	auto renderer_utils = RendererUtils{};

	if (!renderer_utils.validate_texture_2d_update_param(param))
	{
		assert(!"Invalid update param.");

		return;
	}

	if (is_rgba_)
	{
		if (param.rgba_pixels_)
		{
			rgba_pixels_ = param.rgba_pixels_;
		}
	}
	else if (is_indexed_)
	{
		if (param.indexed_pixels_)
		{
			indexed_pixels_ = param.indexed_pixels_;
		}

		if (param.indexed_palette_)
		{
			indexed_palette_ = param.indexed_palette_;
		}

		if (param.indexed_alphas_)
		{
			indexed_alphas_ = param.indexed_alphas_;
		}
	}
	else if (is_indexed_sprite_)
	{
		indexed_sprite_ = param.indexed_sprite_;
	}

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	update_mipmaps();
}

bool Ogl1XRenderer::Texture2d::initialize(
	const RendererTexture2dCreateParam& param)
{
	auto renderer_utils = RendererUtils{};

	if (!renderer_utils.validate_texture_2d_create_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	is_rgba_ = (param.rgba_pixels_ != nullptr);
	is_indexed_ = (param.indexed_pixels_ != nullptr);
	is_indexed_sprite_ = (param.indexed_sprite_ != nullptr);

	has_rgba_alpha_ = param.has_rgba_alpha_;
	is_generate_mipmaps_ = param.is_generate_mipmaps_;

	width_ = param.width_;
	height_ = param.height_;

	actual_width_ = RendererUtils::find_nearest_pot_value(param.width_);
	actual_height_ = RendererUtils::find_nearest_pot_value(param.height_);

	if (is_generate_mipmaps_)
	{
		mipmap_count_ = RendererUtils::calculate_mipmap_count(actual_width_, actual_height_);
	}
	else
	{
		mipmap_count_ = 1;
	}

	indexed_is_column_major_ = param.indexed_is_column_major_;
	indexed_pixels_ = param.indexed_pixels_;
	indexed_palette_ = param.indexed_palette_;
	indexed_alphas_ = param.indexed_alphas_;

	indexed_sprite_ = param.indexed_sprite_;

	rgba_pixels_ = param.rgba_pixels_;

	is_npot_ = (width_ != actual_width_ || height_ != actual_height_);

	auto internal_format = GLenum{};

	if (is_rgba_)
	{
		internal_format = (has_rgba_alpha_ ? GL_RGBA8 : GL_RGB8);
	}
	else if (is_indexed_)
	{
		internal_format = (indexed_alphas_ ? GL_RGBA8 : GL_RGB8);
	}
	else if (is_indexed_sprite_)
	{
		internal_format = GL_RGBA8;
	}

	::glGenTextures(1, &ogl_id_);
	assert(!OglRendererUtils::was_errors());
	assert(ogl_id_ != 0);

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	set_sampler_state_defaults();

	auto mipmap_width = actual_width_;
	auto mipmap_height = actual_height_;

	for (int i_mipmap = 0; i_mipmap < mipmap_count_; ++i_mipmap)
	{
		::glTexImage2D(
			GL_TEXTURE_2D, // target
			i_mipmap, // level
			internal_format, // internal format
			mipmap_width, // width
			mipmap_height, // height
			0, // border
			GL_RGBA, // format
			GL_UNSIGNED_BYTE, // type
			nullptr // pixels
		);

		assert(!OglRendererUtils::was_errors());

		if (mipmap_width > 1)
		{
			mipmap_width /= 2;
		}

		if (mipmap_height > 1)
		{
			mipmap_height /= 2;
		}
	}

	update_mipmaps();

	return true;
}

void Ogl1XRenderer::Texture2d::uninitialize_internal()
{
	if (ogl_id_)
	{
		::glDeleteTextures(1, &ogl_id_);
		assert(!OglRendererUtils::was_errors());

		ogl_id_ = 0;
	}
}

void Ogl1XRenderer::Texture2d::upload_mipmap(
	const int mipmap_level,
	const int width,
	const int height,
	const RendererColor32CPtr src_pixels)
{
	::glTexSubImage2D(
		GL_TEXTURE_2D, // target
		mipmap_level, // level
		0, // xoffset
		0, // yoffset
		width, // width
		height, // height
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		src_pixels // pixels
	);

	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::Texture2d::update_mipmaps()
{
	const auto max_subbuffer_size = actual_width_ * actual_height_;

	auto max_buffer_size = max_subbuffer_size;

	if (is_generate_mipmaps_)
	{
		max_buffer_size *= 2;
	}

	if (static_cast<int>(renderer_->texture_buffer_.size()) < max_buffer_size)
	{
		renderer_->texture_buffer_.resize(max_buffer_size);
	}

	auto texture_subbuffer_0 = &renderer_->texture_buffer_[0];
	auto texture_subbuffer_1 = RendererColor32Ptr{};

	if (is_generate_mipmaps_)
	{
		texture_subbuffer_1 = &renderer_->texture_buffer_[max_subbuffer_size];
	}

	auto is_set_subbuffer_0 = false;

	if (is_rgba_)
	{
		if (is_npot_)
		{
			RendererUtils::rgba_npot_to_rgba_pot(
				width_,
				height_,
				actual_width_,
				actual_height_,
				rgba_pixels_,
				renderer_->texture_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = const_cast<RendererColor32Ptr>(rgba_pixels_);
		}
	}
	else if (is_indexed_)
	{
		const auto& indexed_palette = (indexed_palette_ ? *indexed_palette_ : renderer_->palette_);

		RendererUtils::indexed_to_rgba_pot(
			width_,
			height_,
			actual_width_,
			actual_height_,
			indexed_is_column_major_,
			indexed_pixels_,
			indexed_palette,
			indexed_alphas_,
			renderer_->texture_buffer_
		);
	}
	else if (is_indexed_sprite_)
	{
		RendererUtils::indexed_sprite_to_rgba_pot(
			*indexed_sprite_,
			renderer_->palette_,
			renderer_->texture_buffer_
		);
	}

	auto mipmap_width = actual_width_;
	auto mipmap_height = actual_height_;

	for (int i_mipmap = 0; i_mipmap < mipmap_count_; ++i_mipmap)
	{
		if (i_mipmap > 0)
		{
			RendererUtils::build_mipmap(
				mipmap_width,
				mipmap_height,
				texture_subbuffer_0,
				texture_subbuffer_1);

			if (mipmap_width > 1)
			{
				mipmap_width /= 2;
			}

			if (mipmap_height > 1)
			{
				mipmap_height /= 2;
			}

			if (is_set_subbuffer_0)
			{
				is_set_subbuffer_0 = false;

				texture_subbuffer_0 = &renderer_->texture_buffer_[0];
			}

			std::swap(texture_subbuffer_0, texture_subbuffer_1);
		}

		upload_mipmap(i_mipmap, mipmap_width, mipmap_height, texture_subbuffer_0);
	}
}

void Ogl1XRenderer::Texture2d::set_mag_filter()
{
	auto ogl_mag_filter = GLenum{};

	switch (sampler_state_.mag_filter_)
	{
	case RendererFilterKind::nearest:
		ogl_mag_filter = GL_NEAREST;
		break;

	case RendererFilterKind::linear:
		ogl_mag_filter = GL_LINEAR;
		break;

	default:
		assert(!"Invalid magnification filter.");
		break;
	}

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ogl_mag_filter);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::Texture2d::set_min_filter()
{
	auto ogl_min_filter = GLenum{};

	switch (sampler_state_.mipmap_mode_)
	{
	case RendererMipmapMode::none:
		switch (sampler_state_.min_filter_)
		{
		case RendererFilterKind::nearest:
			ogl_min_filter = GL_NEAREST;
			break;

		case RendererFilterKind::linear:
			ogl_min_filter = GL_LINEAR;
			break;

		default:
			assert(!"Invalid minification filter.");
			break;
		}

		break;

	case RendererMipmapMode::nearest:
		switch (sampler_state_.min_filter_)
		{
		case RendererFilterKind::nearest:
			ogl_min_filter = GL_NEAREST_MIPMAP_NEAREST;
			break;

		case RendererFilterKind::linear:
			ogl_min_filter = GL_LINEAR_MIPMAP_NEAREST;
			break;

		default:
			assert(!"Invalid minification mipmap filter.");
			break;
		}

		break;

	case RendererMipmapMode::linear:
		switch (sampler_state_.min_filter_)
		{
		case RendererFilterKind::nearest:
			ogl_min_filter = GL_NEAREST_MIPMAP_LINEAR;
			break;

		case RendererFilterKind::linear:
			ogl_min_filter = GL_LINEAR_MIPMAP_LINEAR;
			break;

		default:
			assert(!"Invalid minification mipmap filter.");
			break;
		}

		break;

	default:
		assert(!"Invalid mipmap mode.");
		break;
	}

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ogl_min_filter);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::Texture2d::set_address_mode(
	const RendererAddressMode address_mode)
{
	auto ogl_address_mode = GLenum{};

	switch (address_mode)
	{
	case RendererAddressMode::clamp:
		ogl_address_mode = GL_CLAMP;
		break;

	case RendererAddressMode::repeat:
		ogl_address_mode = GL_REPEAT;
		break;

	default:
		assert(!"Invalid address mode.");
		break;
	}

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ogl_address_mode);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::Texture2d::set_address_mode_u()
{
	set_address_mode(sampler_state_.address_mode_u_);
}

void Ogl1XRenderer::Texture2d::set_address_mode_v()
{
	set_address_mode(sampler_state_.address_mode_v_);
}

void Ogl1XRenderer::Texture2d::update_sampler_state(
	const RendererSamplerState& new_sampler_state)
{
	auto is_modified = false;

	// Magnification filter.
	//
	auto is_mag_filter_modified = false;

	if (sampler_state_.mag_filter_ != new_sampler_state.mag_filter_)
	{
		is_modified = true;
		is_mag_filter_modified = true;

		sampler_state_.mag_filter_ = new_sampler_state.mag_filter_;
	}

	// Minification filter.
	//
	auto is_min_filter_modified = false;

	if (sampler_state_.min_filter_ != new_sampler_state.min_filter_ ||
		sampler_state_.mipmap_mode_ != new_sampler_state.mipmap_mode_)
	{
		is_modified = true;
		is_min_filter_modified = true;

		sampler_state_.min_filter_ = new_sampler_state.min_filter_;
		sampler_state_.mipmap_mode_ = new_sampler_state.mipmap_mode_;
	}

	// U-axis address mode.
	//
	auto is_address_mode_u = false;

	if (sampler_state_.address_mode_u_ != new_sampler_state.address_mode_u_)
	{
		is_modified = true;
		is_address_mode_u = true;

		sampler_state_.address_mode_u_ = new_sampler_state.address_mode_u_;
	}

	// V-axis address mode.
	//
	auto is_address_mode_v = false;

	if (sampler_state_.address_mode_v_ != new_sampler_state.address_mode_v_)
	{
		is_modified = true;
		is_address_mode_v = true;

		sampler_state_.address_mode_v_ = new_sampler_state.address_mode_v_;
	}

	// Modify.
	//
	if (is_modified)
	{
		renderer_->texture_set(this);

		if (is_mag_filter_modified)
		{
			set_mag_filter();
		}

		if (is_min_filter_modified)
		{
			set_min_filter();
		}

		if (is_address_mode_u)
		{
			set_address_mode_u();
		}

		if (is_address_mode_v)
		{
			set_address_mode_v();
		}
	}
}

void Ogl1XRenderer::Texture2d::set_sampler_state_defaults()
{
	sampler_state_.mag_filter_ = RendererFilterKind::nearest;
	set_mag_filter();

	sampler_state_.min_filter_ = RendererFilterKind::nearest;
	sampler_state_.mipmap_mode_ = RendererMipmapMode::none;
	set_min_filter();

	sampler_state_.address_mode_u_ = RendererAddressMode::clamp;
	set_address_mode_u();

	sampler_state_.address_mode_v_ = RendererAddressMode::clamp;
	set_address_mode_v();
}

//
// Ogl1XRenderer::Texture2d
// ==========================================================================


// =========================================================================
// Ogl1XRenderer::Sampler
//

Ogl1XRenderer::Sampler::Sampler(
	Ogl1XRendererPtr renderer)
	:
	renderer_{renderer},
	state_{}
{
	assert(renderer_);
}

Ogl1XRenderer::Sampler::~Sampler()
{
}

void Ogl1XRenderer::Sampler::update(
	const RendererSamplerUpdateParam& param)
{
	if (state_.mag_filter_ != param.state_.mag_filter_ ||
		state_.min_filter_ != param.state_.min_filter_ ||
		state_.mipmap_mode_ != param.state_.mipmap_mode_ ||
		state_.address_mode_u_ != param.state_.address_mode_u_ ||
		state_.address_mode_v_ != param.state_.address_mode_v_)
	{
		state_ = param.state_;
	}
}

bool Ogl1XRenderer::Sampler::initialize(
	const RendererSamplerCreateParam& param)
{
	state_ = param.state_;

	return true;
}

//
// Ogl1XRenderer::Sampler
// =========================================================================


// ==========================================================================
// Ogl1XRenderer
//

Ogl1XRenderer::Ogl1XRenderer()
	:
	is_initialized_{},
	error_message_{},
	probe_renderer_path_{},
	sdl_window_{},
	sdl_gl_context_{},
	screen_width_{},
	screen_height_{},
	palette_{},
	viewport_x_{},
	viewport_y_{},
	viewport_width_{},
	viewport_height_{},
	viewport_min_depth_{},
	viewport_max_depth_{},
	scissor_is_enabled_{},
	scissor_x_{},
	scissor_y_{},
	scissor_width_{},
	scissor_height_{},
	culling_is_enabled_{},
	culling_face_{},
	culling_mode_{},
	depth_is_test_enabled_{},
	depth_is_write_enabled_{},
	blending_is_enabled_{},
	texture_2d_is_enabled_{},
	fog_is_enabled_{},
	fog_color_{},
	fog_start_distance_{},
	fog_end_distance_{},
	matrix_model_{},
	matrix_view_{},
	matrix_model_view_{},
	matrix_projection_{},
	matrix_texture_{},
	index_buffers_{},
	vertex_buffers_{},
	texture_buffer_{},
	textures_2d_{},
	texture_2d_current_{},
	samplers_{},
	sampler_current_{},
	sampler_default_{}
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
	screen_width_{std::move(rhs.screen_width_)},
	screen_height_{std::move(rhs.screen_height_)},
	palette_{std::move(rhs.palette_)},
	viewport_x_{std::move(rhs.viewport_x_)},
	viewport_y_{std::move(rhs.viewport_y_)},
	viewport_width_{std::move(rhs.viewport_width_)},
	viewport_height_{std::move(rhs.viewport_height_)},
	viewport_min_depth_{std::move(rhs.viewport_min_depth_)},
	viewport_max_depth_{std::move(rhs.viewport_max_depth_)},
	scissor_is_enabled_{std::move(rhs.scissor_is_enabled_)},
	scissor_x_{std::move(rhs.scissor_x_)},
	scissor_y_{std::move(rhs.scissor_y_)},
	scissor_width_{std::move(rhs.scissor_width_)},
	scissor_height_{std::move(rhs.scissor_height_)},
	culling_is_enabled_{std::move(rhs.culling_is_enabled_)},
	culling_face_{std::move(rhs.culling_face_)},
	culling_mode_{std::move(rhs.culling_mode_)},
	depth_is_test_enabled_{std::move(rhs.depth_is_test_enabled_)},
	depth_is_write_enabled_{std::move(rhs.depth_is_write_enabled_)},
	blending_is_enabled_{std::move(rhs.blending_is_enabled_)},
	texture_2d_is_enabled_{std::move(rhs.texture_2d_is_enabled_)},
	fog_is_enabled_{std::move(rhs.fog_is_enabled_)},
	fog_color_{std::move(rhs.fog_color_)},
	fog_start_distance_{std::move(rhs.fog_start_distance_)},
	fog_end_distance_{std::move(rhs.fog_end_distance_)},
	matrix_model_{std::move(rhs.matrix_model_)},
	matrix_view_{std::move(rhs.matrix_view_)},
	matrix_model_view_{std::move(rhs.matrix_model_view_)},
	matrix_projection_{std::move(rhs.matrix_projection_)},
	matrix_texture_{std::move(rhs.matrix_texture_)},
	index_buffers_{std::move(rhs.index_buffers_)},
	vertex_buffers_{std::move(rhs.vertex_buffers_)},
	texture_buffer_{std::move(rhs.texture_buffer_)},
	textures_2d_{std::move(rhs.textures_2d_)},
	texture_2d_current_{std::move(rhs.texture_2d_current_)},
	samplers_{std::move(rhs.samplers_)},
	sampler_current_{std::move(rhs.sampler_current_)},
	sampler_default_{std::move(rhs.sampler_default_)}
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
	return probe_or_initialize(true, renderer_path, RendererInitializeParam{});
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
	uninitialize_internal();

	return probe_or_initialize(false, RendererPath::none, param);
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

void Ogl1XRenderer::window_show(
	const bool is_visible)
{
	assert(is_initialized_);

	auto renderer_utils = RendererUtils{};

	static_cast<void>(renderer_utils.show_window(sdl_window_.get(), is_visible));
}

void Ogl1XRenderer::color_buffer_set_clear_color(
	const RendererColor32& color)
{
	assert(is_initialized_);

	OglRendererUtils::set_color_buffer_clear_color(color);
}

void Ogl1XRenderer::clear_buffers()
{
	assert(is_initialized_);

	OglRendererUtils::clear_buffers();
}

void Ogl1XRenderer::present()
{
	assert(is_initialized_);

	OglRendererUtils::swap_window(sdl_window_.get());
}

void Ogl1XRenderer::palette_update(
	const RendererPalette& palette)
{
	if (palette_ == palette)
	{
		return;
	}

	palette_ = palette;

	auto param = RendererTexture2dUpdateParam{};
	param.indexed_palette_ = &palette_;

	for (auto& texture_2d : textures_2d_)
	{
		if (!texture_2d->indexed_palette_)
		{
			texture_2d->update(param);
		}
	}
}

RendererIndexBufferPtr Ogl1XRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	assert(is_initialized_);

	auto index_buffer = RendererSwIndexBufferUPtr{new RendererSwIndexBuffer{}};

	if (!index_buffer->initialize(param))
	{
		error_message_ = index_buffer->get_error_message();

		return nullptr;
	}

	index_buffers_.push_back(std::move(index_buffer));

	return index_buffers_.back().get();
}

void Ogl1XRenderer::index_buffer_destroy(
	RendererIndexBufferPtr index_buffer)
{
	assert(index_buffer);

	index_buffers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == index_buffer;
		}
	);
}

RendererVertexBufferPtr Ogl1XRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	auto vertex_buffer = RendererSwVertexBufferUPtr{new RendererSwVertexBuffer{}};

	if (!vertex_buffer->initialize(param))
	{
		error_message_ = vertex_buffer->get_error_message();

		return nullptr;
	}

	vertex_buffers_.push_back(std::move(vertex_buffer));

	return vertex_buffers_.back().get();
}

void Ogl1XRenderer::vertex_buffer_destroy(
	RendererVertexBufferPtr vertex_buffer)
{
	assert(vertex_buffer);

	vertex_buffers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == vertex_buffer;
		}
	);
}

void Ogl1XRenderer::execute_command_sets(
	const RendererCommandSets& command_sets)
{
	for (auto& command_set : command_sets)
	{
		const auto& commands = command_set.commands_;

		for (int i = 0; i < command_set.count_; ++i)
		{
			const auto& command = commands[i];

			switch (command.id_)
			{
			case RendererCommandId::culling_enable:
				command_execute_culling_enable(command.culling_enabled);
				break;

			case RendererCommandId::depth_set_test:
				command_execute_depth_set_test(command.depth_set_test_);
				break;

			case RendererCommandId::depth_set_write:
				command_execute_depth_set_write(command.depth_set_write_);
				break;

			case RendererCommandId::viewport_set:
				command_execute_viewport_set(command.viewport_set_);
				break;

			case RendererCommandId::scissor_enable:
				command_execute_scissor_enable(command.scissor_enable_);
				break;

			case RendererCommandId::scissor_set_box:
				command_execute_scissor_set_box(command.scissor_set_box_);
				break;

			case RendererCommandId::fog_enable:
				command_execute_fog_enable(command.fog_enable_);
				break;

			case RendererCommandId::fog_set_color:
				command_execute_fog_set_color(command.fog_set_color_);
				break;

			case RendererCommandId::fog_set_distances:
				command_execute_fog_set_distances(command.fog_set_distances_);
				break;

			case RendererCommandId::matrix_set_model:
				command_execute_matrix_set_model(command.matrix_set_model_);
				break;

			case RendererCommandId::matrix_set_view:
				command_execute_matrix_set_view(command.matrix_set_view_);
				break;

			case RendererCommandId::matrix_set_model_view:
				command_execute_matrix_set_model_view(command.matrix_set_model_view_);
				break;

			case RendererCommandId::matrix_set_projection:
				command_execute_matrix_set_projection(command.matrix_set_projection_);
				break;

			case RendererCommandId::blending_enable:
				command_execute_enable_blending(command.blending_enable_);
				break;

			case RendererCommandId::texture_set:
				command_execute_texture_set(command.texture_set_);
				break;

			case RendererCommandId::sampler_set:
				command_execute_sampler_set(command.sampler_set_);
				break;

			case RendererCommandId::draw_quads:
				command_execute_draw_quads(command.draw_quads_);
				break;

			default:
				assert(!"Unsupported command id.");
				break;
			}
		}
	}
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
	auto sdl_window = SdlWindowUPtr{};
	auto sdl_gl_context = SdlGlContextUPtr{};
	int screen_width = 0;
	int screen_height = 0;

	auto ogl_renderer_utils = OglRendererUtils{};

	if (is_succeed)
	{
		if (is_probe)
		{
			if (!ogl_renderer_utils.create_probe_window_and_context(sdl_window, sdl_gl_context))
			{
				is_succeed = false;

				error_message_ = ogl_renderer_utils.get_error_message();
			}
		}
		else
		{
			auto window_param = RendererUtilsCreateWindowParam{};
			window_param.is_opengl_ = true;
			window_param.window_ = param.window_;

			if (!ogl_renderer_utils.create_window_and_context(window_param, sdl_window, sdl_gl_context))
			{
				is_succeed = false;

				error_message_ = ogl_renderer_utils.get_error_message();
			}
		}
	}

	if (is_succeed)
	{
		if (!ogl_renderer_utils.window_get_drawable_size(
			sdl_window.get(),
			screen_width,
			screen_height))
		{
			is_succeed = false;

			error_message_ = "Failed to get screen size. ";
			error_message_ += ogl_renderer_utils.get_error_message();
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

	if (is_succeed)
	{
		if (!create_default_sampler())
		{
			is_succeed = false;
		}
	}

	if (!is_succeed)
	{
		return false;
	}

	is_initialized_ = true;
	sdl_window_ = std::move(sdl_window);
	sdl_gl_context_ = std::move(sdl_gl_context);
	screen_width_ = screen_width;
	screen_height_ = screen_height;

	if (!is_probe)
	{
		// Default state.
		//
		viewport_set_defaults();
		scissor_set_defaults();
		culling_set_defaults();
		depth_set_defaults();
		blending_set_defaults();
		texture_2d_set_defaults();
		fog_set_defaults();
		matrix_set_defaults();


		// Present.
		//
		OglRendererUtils::clear_buffers();
		OglRendererUtils::swap_window(sdl_window_.get());
	}

	return true;
}

RendererTexture2dPtr Ogl1XRenderer::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	auto texture_2d = Texture2dUPtr{new Texture2d{this}};

	if (!texture_2d->initialize(param))
	{
		error_message_ = texture_2d->error_message_;

		return nullptr;
	}

	textures_2d_.push_back(std::move(texture_2d));

	return textures_2d_.back().get();
}

void Ogl1XRenderer::texture_2d_destroy(
	RendererTexture2dPtr texture_2d)
{
	assert(texture_2d);

	textures_2d_.remove_if(
		[=](const auto& item)
		{
			return item.get() == texture_2d;
		}
	);
}

RendererSamplerPtr Ogl1XRenderer::sampler_create(
	const RendererSamplerCreateParam& param)
{
	auto sampler = SamplerUPtr{new Sampler{this}};

	if (!sampler->initialize(param))
	{
		error_message_ = "Failed to initialize a sampler.";

		return nullptr;
	}

	samplers_.push_back(std::move(sampler));

	return samplers_.back().get();
}

void Ogl1XRenderer::sampler_destroy(
	RendererSamplerPtr sampler)
{
	assert(sampler);

	auto is_update = false;

	if (sampler == sampler_current_)
	{
		is_update = true;

		sampler_current_ = sampler_default_.get();

		sampler_set();
	}

	samplers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == sampler;
		}
	);
}

bool Ogl1XRenderer::create_default_sampler()
{
	sampler_default_ = SamplerUPtr{new Sampler{this}};

	auto param = RendererSamplerCreateParam{};

	if (!sampler_default_->initialize(param))
	{
		error_message_ = "Failed to initialize default sampler.";

		return false;
	}

	sampler_current_ = sampler_default_.get();

	return true;
}

void Ogl1XRenderer::uninitialize_internal(
	const bool is_dtor)
{
	auto ogl_renderer_utils = OglRendererUtils{};

	if (sdl_gl_context_)
	{
		static_cast<void>(ogl_renderer_utils.make_context_current(sdl_window_.get(), nullptr));
	}

	sdl_window_ = {};
	sdl_gl_context_ = {};

	if (!is_dtor)
	{
		screen_width_ = {};
		screen_height_ = {};
		palette_ = {};
		viewport_x_ = {};
		viewport_y_ = {};
		viewport_width_ = {};
		viewport_height_ = {};
		viewport_min_depth_ = {};
		viewport_max_depth_ = {};
		scissor_is_enabled_ = {};
		scissor_x_ = {};
		scissor_y_ = {};
		scissor_width_ = {};
		scissor_height_ = {};
		culling_is_enabled_ = {};
		culling_face_ = {};
		culling_mode_ = {};
		depth_is_test_enabled_ = {};
		depth_is_write_enabled_ = {};
		blending_is_enabled_ = {};
		texture_2d_is_enabled_ = {};
		fog_is_enabled_ = {};
		fog_color_ = {};
		fog_start_distance_ = {};
		fog_end_distance_ = {};
		matrix_model_ = {};
		matrix_view_ = {};
		matrix_model_view_ = {};
		matrix_projection_ = {};
		matrix_texture_ = {};
		index_buffers_.clear();
		vertex_buffers_.clear();
		texture_buffer_.clear();
		texture_2d_current_ = {};
		sampler_current_ = {};
	}

	index_buffers_.clear();
	vertex_buffers_.clear();
	textures_2d_.clear();
	samplers_.clear();
	sampler_default_ = {};
}

void Ogl1XRenderer::scissor_enable()
{
	detail::OglRendererUtils::scissor_enable(scissor_is_enabled_);
}

void Ogl1XRenderer::scissor_set_box()
{
	assert(scissor_x_ >= 0 && scissor_x_ < screen_width_);
	assert(scissor_y_ >= 0 && scissor_y_ < screen_height_);
	assert((scissor_x_ + screen_width_) <= screen_width_);
	assert((scissor_y_ + screen_height_) <= screen_height_);

	detail::OglRendererUtils::scissor_set_box(
		scissor_x_,
		scissor_y_,
		scissor_width_,
		scissor_height_
	);
}

void Ogl1XRenderer::scissor_set_defaults()
{
	scissor_is_enabled_ = false;

	scissor_x_ = 0;
	scissor_y_ = 0;
	scissor_width_ = screen_width_;
	scissor_height_ = screen_height_;

	scissor_enable();
	scissor_set_box();
}

void Ogl1XRenderer::viewport_set_rectangle()
{
	detail::OglRendererUtils::viewport_set_rectangle(
		viewport_x_,
		viewport_y_,
		viewport_width_,
		viewport_height_
	);
}

void Ogl1XRenderer::viewport_set_depth_range()
{
	detail::OglRendererUtils::viewport_set_depth_range(
		viewport_min_depth_,
		viewport_max_depth_
	);
}

void Ogl1XRenderer::viewport_set_defaults()
{
	viewport_x_ = 0;
	viewport_y_ = 0;
	viewport_width_ = screen_width_;
	viewport_height_ = screen_height_;

	viewport_set_rectangle();

	viewport_min_depth_ = 0.0F;
	viewport_max_depth_ = 1.0F;

	viewport_set_depth_range();
}

void Ogl1XRenderer::culling_set_is_enabled()
{
	if (culling_is_enabled_)
	{
		::glEnable(GL_CULL_FACE);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_CULL_FACE);
		assert(!OglRendererUtils::was_errors());
	}
}

void Ogl1XRenderer::culling_set_face()
{
	auto ogl_face = GLenum{};

	switch (culling_face_)
	{
	case RendererCullingFace::clockwise:
		ogl_face = GL_CW;
		break;

	case RendererCullingFace::counter_clockwise:
		ogl_face = GL_CCW;
		break;

	default:
		assert(!"Invalid front face.");
		break;
	}

	::glFrontFace(ogl_face);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::culling_set_mode()
{
	auto ogl_mode = GLenum{};

	switch (culling_mode_)
	{
	case RendererCullingMode::back:
		ogl_mode = GL_BACK;
		break;

	case RendererCullingMode::front:
		ogl_mode = GL_FRONT;
		break;

	case RendererCullingMode::both:
		ogl_mode = GL_FRONT_AND_BACK;
		break;

	default:
		assert(!"Invalid culling mode.");
		break;
	}

	::glCullFace(ogl_mode);
}

void Ogl1XRenderer::culling_set_defaults()
{
	culling_is_enabled_ = false;
	culling_set_is_enabled();

	culling_face_ = RendererCullingFace::counter_clockwise;
	culling_set_face();

	culling_mode_ = RendererCullingMode::back;
	culling_set_mode();
}

void Ogl1XRenderer::depth_set_test()
{
	if (depth_is_test_enabled_)
	{
		::glEnable(GL_DEPTH_TEST);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_DEPTH_TEST);
		assert(!OglRendererUtils::was_errors());
	}
}

void Ogl1XRenderer::depth_set_write()
{
	::glDepthMask(depth_is_write_enabled_);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::depth_set_defaults()
{
	depth_is_test_enabled_ = false;
	depth_set_test();

	depth_is_write_enabled_ = false;
	depth_set_write();
}

void Ogl1XRenderer::blending_set_is_enable()
{
	if (blending_is_enabled_)
	{
		::glEnable(GL_BLEND);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_BLEND);
		assert(!OglRendererUtils::was_errors());
	}
}

void Ogl1XRenderer::blending_set_function()
{
	::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::blending_set_defaults()
{
	blending_is_enabled_ = false;
	blending_set_is_enable();

	blending_set_function();
}

void Ogl1XRenderer::texture_2d_enable()
{
	::glEnable(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::texture_set()
{
	auto ogl_texture_name = GLuint{};

	if (texture_2d_current_)
	{
		ogl_texture_name = texture_2d_current_->ogl_id_;
	}

	OglRendererUtils::texture_2d_set(ogl_texture_name);
}

void Ogl1XRenderer::texture_set(
	Texture2dPtr new_texture_2d)
{
	if (texture_2d_current_ != new_texture_2d)
	{
		texture_2d_current_ = new_texture_2d;

		texture_set();
	}
}

void Ogl1XRenderer::texture_2d_set_defaults()
{
	texture_2d_is_enabled_ = true;
	texture_2d_enable();

	texture_2d_current_ = nullptr;
	texture_set();
}

void Ogl1XRenderer::fog_set_is_enabled()
{
	if (fog_is_enabled_)
	{
		::glEnable(GL_FOG);
		assert(!OglRendererUtils::was_errors());
	}
	else
	{
		::glDisable(GL_FOG);
		assert(!OglRendererUtils::was_errors());
	}
}

void Ogl1XRenderer::fog_set_mode()
{
	::glFogi(GL_FOG_MODE, GL_LINEAR);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::fog_set_color()
{
	::glFogfv(GL_FOG_COLOR, glm::value_ptr(fog_color_));
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::fog_set_distances()
{
	::glFogf(GL_FOG_START, fog_start_distance_);
	assert(!OglRendererUtils::was_errors());

	::glFogf(GL_FOG_END, fog_end_distance_);
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::fog_set_defaults()
{
	fog_is_enabled_ = false;
	fog_set_is_enabled();

	fog_set_mode();

	fog_color_ = glm::vec4{};
	fog_set_color();

	fog_start_distance_ = 0.0F;
	fog_end_distance_ = 1.0F;
	fog_set_distances();
}

void Ogl1XRenderer::matrix_set_model()
{
	matrix_set_model_view();
}

void Ogl1XRenderer::matrix_set_view()
{
	matrix_set_model_view();
}

void Ogl1XRenderer::matrix_set_model_view()
{
	::glMatrixMode(GL_MODELVIEW);
	assert(!OglRendererUtils::was_errors());

	::glLoadMatrixf(glm::value_ptr(matrix_model_view_));
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::matrix_set_projection()
{
	::glMatrixMode(GL_PROJECTION);
	assert(!OglRendererUtils::was_errors());

	::glLoadMatrixf(glm::value_ptr(matrix_projection_));
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::matrix_set_texture()
{
	::glMatrixMode(GL_TEXTURE);
	assert(!OglRendererUtils::was_errors());

	::glLoadMatrixf(glm::value_ptr(matrix_texture_));
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::matrix_set_defaults()
{
	const auto& identity = glm::identity<glm::mat4>();

	// Convert from bottom-top to top-bottom.
	const auto& matrix_texture = glm::mat4
	{
		1.0F, 0.0F, 0.0F, 0.0F,
		0.0F, -1.0F, 0.0F, 0.0F,
		0.0F, 0.0F, 1.0F, 0.0F,
		0.0F, 0.0F, 0.0F, 1.0F,
	};

	matrix_model_ = identity;
	matrix_view_ = identity;
	matrix_model_view_ = matrix_view_ * matrix_model_;
	matrix_projection_ = identity;
	matrix_texture_ = matrix_texture;

	matrix_set_model_view();
	matrix_set_projection();
	matrix_set_texture();
}

void Ogl1XRenderer::sampler_set()
{
	assert(sampler_current_);

	for (auto& texture_2d : textures_2d_)
	{
		texture_2d->update_sampler_state(sampler_default_->state_);
	}
}

void Ogl1XRenderer::command_execute_culling_enable(
	const RendererCommand::CullingEnabled& command)
{
	if (culling_is_enabled_ != command.is_enabled_)
	{
		culling_is_enabled_ = command.is_enabled_;

		culling_set_is_enabled();
	}
}

void Ogl1XRenderer::command_execute_depth_set_test(
	const RendererCommand::DepthSetTest& command)
{
	if (depth_is_test_enabled_ != command.is_enabled_)
	{
		depth_is_test_enabled_ = command.is_enabled_;

		depth_set_test();
	}
}

void Ogl1XRenderer::command_execute_depth_set_write(
	const RendererCommand::DepthSetWrite& command)
{
	if (depth_is_write_enabled_ != command.is_enabled_)
	{
		depth_is_write_enabled_ = command.is_enabled_;

		depth_set_write();
	}
}

void Ogl1XRenderer::command_execute_viewport_set(
	const RendererCommand::ViewportSet& command)
{
	assert(command.x_ < screen_width_);
	assert(command.y_ < screen_height_);
	assert(command.width_ <= screen_width_);
	assert(command.height_ <= screen_height_);
	assert((command.x_ + command.width_) <= screen_width_);
	assert((command.y_ + command.height_) <= screen_height_);

	if (viewport_x_ != command.x_ ||
		viewport_y_ != command.y_ ||
		viewport_width_ != command.width_ ||
		viewport_height_ != command.height_)
	{
		viewport_x_ = command.x_;
		viewport_y_ = command.y_;
		viewport_width_ = command.width_;
		viewport_height_ = command.height_;

		viewport_set_rectangle();
	}

	if (viewport_min_depth_ != command.min_depth_ ||
		viewport_max_depth_ != command.max_depth_)
	{
		viewport_min_depth_ = command.min_depth_;
		viewport_max_depth_ = command.max_depth_;

		viewport_set_depth_range();
	}
}

void Ogl1XRenderer::command_execute_scissor_enable(
	const RendererCommand::ScissorEnable& command)
{
	if (scissor_is_enabled_ != command.is_enabled_)
	{
		scissor_is_enabled_ = command.is_enabled_;

		scissor_enable();
	}
}

void Ogl1XRenderer::command_execute_scissor_set_box(
	const RendererCommand::ScissorSetBox& command)
{
	assert(command.x_ < screen_width_);
	assert(command.y_ < screen_height_);
	assert(command.width_ <= screen_width_);
	assert(command.height_ <= screen_height_);
	assert((command.x_ + command.width_) <= screen_width_);
	assert((command.y_ + command.height_) <= screen_height_);

	if (scissor_x_ != command.x_ ||
		scissor_y_ != command.y_ ||
		scissor_width_ != command.width_ ||
		scissor_height_ != command.height_)
	{
		scissor_x_ = command.x_;
		scissor_y_ = command.y_;
		scissor_width_ = command.width_;
		scissor_height_ = command.height_;

		scissor_set_box();
	}
}

void Ogl1XRenderer::command_execute_fog_enable(
	const RendererCommand::FogEnable& command)
{
	if (fog_is_enabled_ != command.is_enabled_)
	{
		fog_is_enabled_ = command.is_enabled_;

		fog_set_is_enabled();
	}
}

void Ogl1XRenderer::command_execute_fog_set_color(
	const RendererCommand::FogSetColor& command)
{
	if (fog_color_ != command.color_)
	{
		fog_color_ = command.color_;

		fog_set_color();
	}
}

void Ogl1XRenderer::command_execute_fog_set_distances(
	const RendererCommand::FogSetDistances& command)
{
	auto is_modified = false;

	if (fog_start_distance_ != command.start_)
	{
		is_modified = true;

		fog_start_distance_ = command.start_;
	}

	if (fog_end_distance_ != command.end_)
	{
		is_modified = true;

		fog_end_distance_ = command.end_;
	}

	if (is_modified)
	{
		fog_set_distances();
	}
}

void Ogl1XRenderer::command_execute_matrix_set_model(
	const RendererCommand::MatrixSetModel& command)
{
	if (matrix_model_ != command.model_)
	{
		matrix_model_ = command.model_;

		matrix_set_model();
	}
}

void Ogl1XRenderer::command_execute_matrix_set_view(
	const RendererCommand::MatrixSetView& command)
{
	if (matrix_view_ != command.view_)
	{
		matrix_view_ = command.view_;

		matrix_set_model();
	}
}

void Ogl1XRenderer::command_execute_matrix_set_model_view(
	const RendererCommand::MatrixSetModelView& command)
{
	auto is_modified = false;

	if (matrix_model_ != command.model_)
	{
		is_modified = true;

		matrix_model_ = command.model_;
	}

	if (matrix_view_ != command.view_)
	{
		is_modified = true;

		matrix_view_ = command.view_;
	}

	if (is_modified)
	{
		matrix_model_view_ = matrix_view_ * matrix_model_;

		matrix_set_model_view();
	}
}

void Ogl1XRenderer::command_execute_matrix_set_projection(
	const RendererCommand::MatrixSetProjection& command)
{
	if (matrix_projection_ != command.projection_)
	{
		matrix_projection_ = command.projection_;

		matrix_set_projection();
	}
}

void Ogl1XRenderer::command_execute_enable_blending(
	const RendererCommand::BlendingEnable& command)
{
	if (blending_is_enabled_ != command.is_enabled_)
	{
		blending_is_enabled_ = command.is_enabled_;

		blending_set_is_enable();
	}
}

void Ogl1XRenderer::command_execute_texture_set(
	const RendererCommand::TextureSet& command)
{
	texture_set(static_cast<Texture2dPtr>(command.texture_2d_));
}

void Ogl1XRenderer::command_execute_sampler_set(
	const RendererCommand::SamplerSet& command)
{
	assert(command.sampler_);

	auto sampler = static_cast<SamplerPtr>(command.sampler_);

	if (!sampler)
	{
		return;
	}

	sampler_current_ = sampler;
}

void Ogl1XRenderer::command_execute_draw_quads(
	const RendererCommand::DrawQuads& command)
{
	assert(command.count_ > 0);
	assert(command.index_offset_ >= 0);
	assert(command.index_buffer_);
	assert(command.vertex_buffer_);

	const auto triangles_per_quad = 2;
	const auto triangle_count = command.count_ * triangles_per_quad;

	const auto indices_per_triangle = 3;
	const auto indices_per_quad = triangles_per_quad * indices_per_triangle;
	const auto index_count = indices_per_quad * command.count_;

	auto& index_buffer = *reinterpret_cast<RendererSwIndexBuffer*>(command.index_buffer_);
	auto& vertex_buffer = *reinterpret_cast<RendererSwVertexBuffer*>(command.vertex_buffer_);

	const auto index_byte_depth = index_buffer.get_byte_depth();
	const auto max_index_count = index_buffer.get_size() / index_byte_depth;
	const auto index_byte_offset = command.index_offset_ * index_byte_depth;
	assert(command.index_offset_ < max_index_count);
	assert(command.count_ <= max_index_count);
	assert((command.index_offset_ + command.count_) <= max_index_count);

	const auto stride = static_cast<GLsizei>(sizeof(RendererVertex));
	const auto vertex_buffer_data = reinterpret_cast<const std::uint8_t*>(vertex_buffer.get_data());

	// Sampler state.
	//
	if (texture_2d_current_)
	{
		texture_2d_current_->update_sampler_state(sampler_current_->state_);
	}

	// Diffuse.
	//
	::glEnableClientState(GL_COLOR_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glColorPointer(
		4, // size
		GL_UNSIGNED_BYTE, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, rgba_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Texture coordinates.
	//
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glTexCoordPointer(
		2, // size
		GL_FLOAT, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, uv_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Position.
	//
	::glEnableClientState(GL_VERTEX_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glVertexPointer(
		3, // size
		GL_FLOAT, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, xyz_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Draw the quads.
	//

	const auto index_buffer_data = static_cast<const std::uint8_t*>(index_buffer.get_data()) + index_byte_offset;

	const auto ogl_element_type = OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
		index_buffer.get_byte_depth());

	::glDrawElements(
		GL_TRIANGLES, // mode
		index_count, // count
		ogl_element_type, // type
		index_buffer_data // indices
	);

	assert(!OglRendererUtils::was_errors());

	// Disable the state.
	//
	::glBindTexture(GL_TEXTURE_2D, 0);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_VERTEX_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_COLOR_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	assert(!OglRendererUtils::was_errors());
}

//
// Ogl1XRenderer
// ==========================================================================


} // detail
} // bstone
