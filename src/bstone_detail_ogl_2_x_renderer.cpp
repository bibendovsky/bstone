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
// OpenGL 2.x renderer (implementation).
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_detail_ogl_2_x_renderer.h"
#include <cassert>
#include "glm/gtc/type_ptr.hpp"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_extension_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ogl2XRenderer::Texture2d
//

Ogl2XRenderer::Texture2d::Texture2d(
	Ogl2XRendererPtr renderer)
	:
	renderer_{renderer},
	error_message_{},
	storage_pixel_format_{},
	width_{},
	height_{},
	mipmap_count_{},
	sampler_state_{},
	ogl_id_{}
{
	assert(renderer_);
}

Ogl2XRenderer::Texture2d::~Texture2d()
{
	uninitialize_internal();
}

void Ogl2XRenderer::Texture2d::update(
	const RendererTexture2dUpdateParam& param)
{
	auto renderer_utils = RendererUtils{};

	if (!renderer_utils.validate_texture_2d_update_param(param))
	{
		assert(!"Invalid update param.");

		return;
	}

	if (param.mipmap_level_ >= mipmap_count_)
	{
		assert(!"Mipmap level out of range.");

		return;
	}

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	auto mipmap_width = width_;
	auto mipmap_height = height_;

	for (auto i = 0; i < param.mipmap_level_; ++i)
	{
		if (mipmap_width > 1)
		{
			mipmap_width /= 2;
		}

		if (mipmap_height > 1)
		{
			mipmap_height /= 2;
		}
	}

	upload_mipmap(param.mipmap_level_, mipmap_width, mipmap_height, param.rgba_pixels_);
}

void Ogl2XRenderer::Texture2d::generate_mipmaps()
{
	if (mipmap_count_ <= 1)
	{
		assert(!"Base mipmap.");

		return;
	}

	const auto& device_features = renderer_->device_features_;

	if (!device_features.mipmap_is_available_)
	{
		assert(!"Mipmap generation not available.");

		return;
	}

	const auto& ogl_device_features = renderer_->ogl_device_features_;

	if (ogl_device_features.mipmap_function_ == nullptr)
	{
		assert(!"Null mipmap generation function.");

		return;
	}

	ogl_device_features.mipmap_function_(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());
}

bool Ogl2XRenderer::Texture2d::initialize(
	const RendererTexture2dCreateParam& param)
{
	auto renderer_utils = RendererUtils{};

	if (!renderer_utils.validate_texture_2d_create_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	storage_pixel_format_ = param.storage_pixel_format_;

	width_ = param.width_;
	height_ = param.height_;
	mipmap_count_ = param.mipmap_count_;

	const auto max_mipmap_count = RendererUtils::calculate_mipmap_count(width_, height_);

	if (mipmap_count_ > max_mipmap_count)
	{
		error_message_ = "Mipmap count out of range.";

		return false;
	}

	const auto internal_format = (storage_pixel_format_ == RendererPixelFormat::r8g8b8a8_unorm ? GL_RGBA8 : GL_RGB8);

	::glGenTextures(1, &ogl_id_);
	assert(!OglRendererUtils::was_errors());
	assert(ogl_id_ != 0);

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
	assert(!OglRendererUtils::was_errors());

	set_sampler_state_defaults();

	auto mipmap_width = width_;
	auto mipmap_height = height_;

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

	return true;
}

void Ogl2XRenderer::Texture2d::uninitialize_internal()
{
	if (ogl_id_)
	{
		::glDeleteTextures(1, &ogl_id_);
		assert(!OglRendererUtils::was_errors());

		ogl_id_ = 0;
	}
}

void Ogl2XRenderer::Texture2d::upload_mipmap(
	const int mipmap_level,
	const int width,
	const int height,
	const R8g8b8a8CPtr src_pixels)
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

void Ogl2XRenderer::Texture2d::set_mag_filter()
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

void Ogl2XRenderer::Texture2d::set_min_filter()
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

void Ogl2XRenderer::Texture2d::set_address_mode(
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

void Ogl2XRenderer::Texture2d::set_address_mode_u()
{
	set_address_mode(sampler_state_.address_mode_u_);
}

void Ogl2XRenderer::Texture2d::set_address_mode_v()
{
	set_address_mode(sampler_state_.address_mode_v_);
}

void Ogl2XRenderer::Texture2d::set_anisotropy()
{
	OglRendererUtils::anisotropy_set_value(
		GL_TEXTURE_2D,
		renderer_->device_features_,
		sampler_state_.anisotropy_
	);
}

void Ogl2XRenderer::Texture2d::update_sampler_state(
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

	// Anisotropy.
	//
	auto is_anisotropy = false;

	if (sampler_state_.anisotropy_ != new_sampler_state.anisotropy_)
	{
		is_modified = true;
		is_anisotropy = true;

		sampler_state_.anisotropy_ = new_sampler_state.anisotropy_;
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

		if (is_anisotropy)
		{
			set_anisotropy();
		}
	}
}

void Ogl2XRenderer::Texture2d::set_sampler_state_defaults()
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

	sampler_state_.anisotropy_ = RendererSampler::anisotropy_min;
}

//
// Ogl2XRenderer::Texture2d
// ==========================================================================


// =========================================================================
// Ogl2XRenderer::Sampler
//

Ogl2XRenderer::Sampler::Sampler(
	Ogl2XRendererPtr renderer)
	:
	renderer_{renderer},
	state_{}
{
	assert(renderer_);
}

Ogl2XRenderer::Sampler::~Sampler()
{
}

void Ogl2XRenderer::Sampler::update(
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

bool Ogl2XRenderer::Sampler::initialize(
	const RendererSamplerCreateParam& param)
{
	state_ = param.state_;

	return true;
}

//
// Ogl2XRenderer::Sampler
// =========================================================================


// =========================================================================
// Ogl2XRenderer::VertexInput
//

Ogl2XRenderer::VertexInput::VertexInput(
	Ogl2XRendererPtr renderer)
	:
	renderer_{renderer},
	error_message_{},
	index_buffer_{},
	attribute_descriptions_{}
{
	assert(renderer_ != nullptr);
}

Ogl2XRenderer::VertexInput::~VertexInput()
{
}

bool Ogl2XRenderer::VertexInput::initialize(
	const RendererVertexInputCreateParam& param)
{
	auto renderer_utils = RendererUtils{};

	const auto max_locations = renderer_->device_features_.max_vertex_input_locations_;

	if (!renderer_utils.vertex_input_validate_param(max_locations, param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	const auto is_location_out_of_range = std::any_of(
		param.attribute_descriptions_.cbegin(),
		param.attribute_descriptions_.cend(),
		[=](const auto& item)
		{
			return item.location_ < 0 || item.location_ >= max_locations;
		}
	);

	if (is_location_out_of_range)
	{
		error_message_ = "Location out of range.";

		return false;
	}

	index_buffer_ = param.index_buffer_;
	attribute_descriptions_ = param.attribute_descriptions_;

	return true;
}

//
// Ogl2XRenderer::VertexInput
// =========================================================================


// ==========================================================================
// Ogl2XRenderer
//

Ogl2XRenderer::Ogl2XRenderer()
	:
	is_initialized_{},
	error_message_{},
	probe_{},
	sdl_window_{},
	sdl_gl_context_{},
	extension_manager_{},
	ogl_state_{},
	device_info_{},
	device_features_{},
	ogl_device_features_{},
	screen_width_{},
	screen_height_{},
	downscale_width_{},
	downscale_height_{},
	downscale_blit_filter_{},
	aa_kind_{},
	aa_value_{},
	ogl_msaa_fbo_{},
	ogl_msaa_color_rb_{},
	ogl_msaa_depth_rb_{},
	ogl_downscale_fbo_{},
	ogl_downscale_color_rb_{},
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
	blending_src_factor_{},
	blending_dst_factor_{},
	texture_2d_is_enabled_{},
	index_buffers_{},
	vertex_buffers_{},
	texture_buffer_{},
	textures_2d_{},
	texture_2d_current_{},
	samplers_{},
	sampler_current_{},
	sampler_default_{},
	vertex_inputs_{},
	vertex_input_current_{},
	vertex_input_enabled_locations_{},
	shaders_{},
	shader_stages_{},
	current_shader_stage_{}
{
}

Ogl2XRenderer::Ogl2XRenderer(
	Ogl2XRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	probe_{std::move(rhs.probe_)},
	sdl_window_{std::move(rhs.sdl_window_)},
	sdl_gl_context_{std::move(rhs.sdl_gl_context_)},
	extension_manager_{std::move(rhs.extension_manager_)},
	ogl_state_{std::move(rhs.ogl_state_)},
	device_info_{std::move(rhs.device_info_)},
	device_features_{std::move(rhs.device_features_)},
	ogl_device_features_{std::move(rhs.ogl_device_features_)},
	screen_width_{std::move(rhs.screen_width_)},
	screen_height_{std::move(rhs.screen_height_)},
	downscale_width_{std::move(rhs.downscale_width_)},
	downscale_height_{std::move(rhs.downscale_height_)},
	downscale_blit_filter_{std::move(rhs.downscale_blit_filter_)},
	aa_kind_{std::move(rhs.aa_kind_)},
	aa_value_{std::move(rhs.aa_value_)},
	ogl_msaa_fbo_{std::move(rhs.ogl_msaa_fbo_)},
	ogl_msaa_color_rb_{std::move(rhs.ogl_msaa_color_rb_)},
	ogl_msaa_depth_rb_{std::move(rhs.ogl_msaa_depth_rb_)},
	ogl_downscale_fbo_{std::move(rhs.ogl_downscale_fbo_)},
	ogl_downscale_color_rb_{std::move(rhs.ogl_downscale_color_rb_)},
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
	blending_src_factor_{std::move(rhs.blending_src_factor_)},
	blending_dst_factor_{std::move(rhs.blending_dst_factor_)},
	texture_2d_is_enabled_{std::move(rhs.texture_2d_is_enabled_)},
	index_buffers_{std::move(rhs.index_buffers_)},
	vertex_buffers_{std::move(rhs.vertex_buffers_)},
	texture_buffer_{std::move(rhs.texture_buffer_)},
	textures_2d_{std::move(rhs.textures_2d_)},
	texture_2d_current_{std::move(rhs.texture_2d_current_)},
	samplers_{std::move(rhs.samplers_)},
	sampler_current_{std::move(rhs.sampler_current_)},
	sampler_default_{std::move(rhs.sampler_default_)},
	vertex_inputs_{std::move(rhs.vertex_inputs_)},
	vertex_input_current_{std::move(rhs.vertex_input_current_)},
	vertex_input_enabled_locations_{std::move(rhs.vertex_input_enabled_locations_)},
	shaders_{std::move(rhs.shaders_)},
	shader_stages_{std::move(rhs.shader_stages_)},
	current_shader_stage_{std::move(rhs.current_shader_stage_)}
{
	rhs.is_initialized_ = false;
	rhs.sdl_window_ = nullptr;
	rhs.sdl_gl_context_ = nullptr;

	rhs.ogl_msaa_fbo_ = GL_NONE;
	rhs.ogl_msaa_color_rb_ = GL_NONE;
	rhs.ogl_msaa_depth_rb_ = GL_NONE;

	rhs.ogl_downscale_fbo_ = GL_NONE;
	rhs.ogl_downscale_color_rb_ = GL_NONE;

	rhs.current_shader_stage_ = nullptr;
}

Ogl2XRenderer::~Ogl2XRenderer()
{
	uninitialize_internal(true);
}

const std::string& Ogl2XRenderer::get_error_message() const
{
	return error_message_;
}

RendererKind Ogl2XRenderer::get_kind() const
{
	return RendererKind::ogl_2_x;
}

const std::string& Ogl2XRenderer::get_name() const
{
	static const auto result = std::string{"ogl2"};

	return result;
}

const std::string& Ogl2XRenderer::get_description() const
{
	static const auto result = std::string{"OpenGL 2.x"};

	return result;
}

bool Ogl2XRenderer::probe()
{
	uninitialize_internal();

	const auto result = probe_or_initialize(true, RendererInitializeParam{});

	uninitialize_internal();

	return result;
}

const RendererProbe& Ogl2XRenderer::probe_get() const
{
	return probe_;
}

bool Ogl2XRenderer::is_initialized() const
{
	return is_initialized_;
}

bool Ogl2XRenderer::initialize(
	const RendererInitializeParam& param)
{
	if (probe_.kind_ == RendererKind::none)
	{
		uninitialize_internal();

		const auto probe_result = probe_or_initialize(true, RendererInitializeParam{});

		uninitialize_internal();

		if (!probe_result)
		{
			return false;
		}
	}

	uninitialize_internal();

	if (!probe_or_initialize(false, param))
	{
		uninitialize_internal();

		return false;
	}

	return true;
}

void Ogl2XRenderer::uninitialize()
{
	uninitialize_internal();
}

const RendererDeviceFeatures& Ogl2XRenderer::device_get_features() const
{
	return device_features_;
}

const RendererDeviceInfo& Ogl2XRenderer::device_get_info() const
{
	return device_info_;
}

bool Ogl2XRenderer::device_is_lost() const
{
	return false;
}

bool Ogl2XRenderer::device_is_ready_to_reset() const
{
	return true;
}

void Ogl2XRenderer::device_reset()
{
}

void Ogl2XRenderer::window_show(
	const bool is_visible)
{
	assert(is_initialized_);

	auto renderer_utils = RendererUtils{};

	static_cast<void>(renderer_utils.show_window(sdl_window_.get(), is_visible));
}

const glm::mat4& Ogl2XRenderer::csc_get_texture() const
{
	return detail::OglRendererUtils::csc_get_texture();
}

const glm::mat4& Ogl2XRenderer::csc_get_projection() const
{
	return detail::OglRendererUtils::csc_get_projection();
}

bool Ogl2XRenderer::vsync_get() const
{
	if (!device_features_.vsync_is_available_)
	{
		return false;
	}

	return OglRendererUtils::vsync_get();
}

bool Ogl2XRenderer::vsync_set(
	const bool is_enabled)
{
	if (!device_features_.vsync_is_available_)
	{
		error_message_ = "Not available.";

		return false;
	}

	if (device_features_.vsync_is_requires_restart_)
	{
		error_message_ = "Requires restart.";

		return false;
	}

	if (!OglRendererUtils::vsync_set(is_enabled))
	{
		error_message_ = "Not supported.";

		return false;
	}

	return true;
}

bool Ogl2XRenderer::downscale_set(
	const int width,
	const int height,
	const RendererFilterKind blit_filter)
{
	if (width <= 0 ||
		width > screen_width_ ||
		height <= 0 ||
		height > screen_height_)
	{
		error_message_ = "Dimensions out of range.";

		return false;
	}

	switch (blit_filter)
	{
		case RendererFilterKind::nearest:
		case RendererFilterKind::linear:
			break;

		default:
			error_message_ = "Invalid blit filter.";

			return false;
	}

	if (!device_features_.framebuffer_is_available_)
	{
		error_message_ = "Off-screen framebuffer not supported.";

		return false;
	}

	if (ogl_msaa_fbo_ == GL_NONE)
	{
		error_message_ = "No off-screen framebuffer.";

		return false;
	}

	downscale_width_ = width;
	downscale_height_ = height;
	downscale_blit_filter_ = blit_filter;

	framebuffers_destroy();

	return framebuffers_create();
}

bool Ogl2XRenderer::aa_set(
	const RendererAaKind aa_kind,
	const int aa_value)
{
	switch (aa_kind)
	{
		case RendererAaKind::none:
			aa_disable();
			return true;

		case RendererAaKind::ms:
			return msaa_set(aa_value);

		default:
			error_message_ = "Invalid AA kind.";

			return false;
	}
}

void Ogl2XRenderer::color_buffer_set_clear_color(
	const R8g8b8a8& color)
{
	assert(is_initialized_);

	OglRendererUtils::set_color_buffer_clear_color(color);
}

void Ogl2XRenderer::clear_buffers()
{
	assert(is_initialized_);

	framebuffers_bind();

	OglRendererUtils::clear_buffers();
}

void Ogl2XRenderer::present()
{
	assert(is_initialized_);

	framebuffers_blit();

	OglRendererUtils::swap_window(sdl_window_.get());
}

RendererIndexBufferPtr Ogl2XRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	assert(is_initialized_);

	auto index_buffer = IndexBufferImplUPtr{new OglIndexBuffer{ogl_state_.get()}};

	if (!index_buffer->initialize(param))
	{
		error_message_ = index_buffer->get_error_message();

		return nullptr;
	}

	index_buffers_.push_back(std::move(index_buffer));

	return index_buffers_.back().get();
}

void Ogl2XRenderer::index_buffer_destroy(
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

RendererVertexBufferPtr Ogl2XRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	auto vertex_buffer = VertexBufferImplUPtr{new OglVertexBuffer{ogl_state_.get()}};

	if (!vertex_buffer->initialize(param))
	{
		error_message_ = vertex_buffer->get_error_message();

		return nullptr;
	}

	vertex_buffers_.push_back(std::move(vertex_buffer));

	return vertex_buffers_.back().get();
}

void Ogl2XRenderer::vertex_buffer_destroy(
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

RendererVertexInputPtr Ogl2XRenderer::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	auto vertex_input = VertexInputUPtr{new VertexInput{this}};

	if (!vertex_input->initialize(param))
	{
		error_message_ = vertex_input->error_message_;

		return nullptr;
	}

	vertex_inputs_.push_back(std::move(vertex_input));

	return vertex_inputs_.back().get();
}

void Ogl2XRenderer::vertex_input_destroy(
	RendererVertexInputPtr vertex_input)
{
	assert(vertex_input);

	vertex_inputs_.remove_if(
		[=](const auto& item)
		{
			return item.get() == vertex_input;
		}
	);
}

RendererShaderPtr Ogl2XRenderer::shader_create(
	const RendererShader::CreateParam& param)
{
	auto shader = detail::OglShaderUPtr{new detail::OglShader{}};

	shader->initialize(param);

	if (!shader->is_initialized())
	{
		error_message_ = "Failed to create a shader. ";
		error_message_ += shader->get_error_message();

		return nullptr;
	}

	shaders_.emplace_back(std::move(shader));

	return shaders_.back().get();
}

void Ogl2XRenderer::shader_destroy(
	const RendererShaderPtr shader)
{
	if (shader == nullptr)
	{
		return;
	}

	shaders_.remove_if(
		[=](const auto& item)
		{
			return item.get() == shader;
		}
	);
}

RendererShaderStagePtr Ogl2XRenderer::shader_stage_create(
	const RendererShaderStage::CreateParam& param)
{
	auto shader_stage = detail::OglShaderStageUPtr{new detail::OglShaderStage{}};

	shader_stage->initialize(&current_shader_stage_, param);

	if (!shader_stage->is_initialized())
	{
		error_message_ = "Failed to create a shader stage. ";
		error_message_ += shader_stage->get_error_message();

		return nullptr;
	}

	shader_stages_.emplace_back(std::move(shader_stage));

	return shader_stages_.back().get();
}

void Ogl2XRenderer::shader_stage_destroy(
	const RendererShaderStagePtr shader_stage)
{
	if (shader_stage == nullptr)
	{
		return;
	}

	const auto size_before = shader_stages_.size();

	shader_stages_.remove_if(
		[=](const auto& item)
		{
			return item.get() == shader_stage;
		}
	);

	const auto size_after = shader_stages_.size();

	if (size_before != size_after)
	{
		current_shader_stage_ = nullptr;
	}
}

void Ogl2XRenderer::execute_commands(
	const RendererCommandManagerPtr command_manager)
{
	const auto buffer_count = command_manager->buffer_get_count();

	for (int i = 0; i < buffer_count; ++i)
	{
		auto command_buffer = command_manager->buffer_get(i);

		if (!command_buffer->is_enabled())
		{
			continue;
		}

		const auto command_count = command_buffer->get_command_count();

		command_buffer->read_begin();

		for (int j = 0; j < command_count; ++j)
		{
			const auto command_id = command_buffer->read_command_id();

			switch (command_id)
			{
			case RendererCommandId::culling_enable:
				command_execute_culling(*command_buffer->read_culling());
				break;

			case RendererCommandId::depth_set_test:
				command_execute_depth_test(*command_buffer->read_depth_test());
				break;

			case RendererCommandId::depth_set_write:
				command_execute_depth_write(*command_buffer->read_depth_write());
				break;

			case RendererCommandId::viewport_set:
				command_execute_viewport(*command_buffer->read_viewport());
				break;

			case RendererCommandId::scissor_enable:
				command_execute_scissor(*command_buffer->read_scissor());
				break;

			case RendererCommandId::scissor_set_box:
				command_execute_scissor_box(*command_buffer->read_scissor_box());
				break;

			case RendererCommandId::blending_enable:
				command_execute_blending(*command_buffer->read_blending());
				break;

			case RendererCommandId::blending_function:
				command_execute_blending_function(*command_buffer->read_blending_function());
				break;

			case RendererCommandId::texture_set:
				command_execute_texture(*command_buffer->read_texture());
				break;

			case RendererCommandId::sampler_set:
				command_execute_sampler(*command_buffer->read_sampler());
				break;

			case RendererCommandId::vertex_input_set:
				command_execute_vertex_input(*command_buffer->read_vertex_input());
				break;

			case RendererCommandId::shader_stage:
				command_execute_shader_stage(*command_buffer->read_shader_stage());
				break;

			case RendererCommandId::shader_variable_int32:
				command_execute_shader_variable_int32(*command_buffer->read_shader_variable_int32());
				break;

			case RendererCommandId::shader_variable_float32:
				command_execute_shader_variable_float32(*command_buffer->read_shader_variable_float32());
				break;

			case RendererCommandId::shader_variable_vec2:
				command_execute_shader_variable_vec2(*command_buffer->read_shader_variable_vec2());
				break;

			case RendererCommandId::shader_variable_vec4:
				command_execute_shader_variable_vec4(*command_buffer->read_shader_variable_vec4());
				break;

			case RendererCommandId::shader_variable_mat4:
				command_execute_shader_variable_mat4(*command_buffer->read_shader_variable_mat4());
				break;

			case RendererCommandId::shader_variable_sampler2d:
				command_execute_shader_variable_sampler_2d(*command_buffer->read_shader_variable_sampler_2d());
				break;

			case RendererCommandId::draw_quads:
				command_execute_draw_quads(*command_buffer->read_draw_quads());
				break;

			default:
				assert(!"Unsupported command id.");
				break;
			}
		}

		command_buffer->read_end();
	}
}

bool Ogl2XRenderer::probe_or_initialize(
	const bool is_probe,
	const RendererInitializeParam& param)
{
	auto ogl_renderer_utils = OglRendererUtils{};

	if (is_probe)
	{
		if (!ogl_renderer_utils.create_probe_window_and_context(sdl_window_, sdl_gl_context_))
		{
			error_message_ = ogl_renderer_utils.get_error_message();

			return false;
		}
	}
	else
	{
		auto window_param = RendererUtilsCreateWindowParam{};
		window_param.is_opengl_ = true;
		window_param.window_ = param.window_;
		window_param.aa_kind_ = RendererAaKind::none;
		window_param.aa_value_ = 0;

		const auto& probe_device_features = probe_.device_features_;

		switch (param.aa_kind_)
		{
			case RendererAaKind::ms:
				if (!probe_device_features.framebuffer_is_available_)
				{
					auto aa_kind = param.aa_kind_;
					auto aa_value = param.aa_value_;

					if (aa_value < probe_device_features.msaa_min_value_)
					{
						aa_value = probe_device_features.msaa_min_value_;
					}
					else if (aa_value > probe_device_features.msaa_max_value_)
					{
						aa_value = probe_device_features.msaa_max_value_;
					}

					window_param.aa_kind_ = aa_kind;
					window_param.aa_value_ = aa_value;
				}

				break;

			default:
				break;
		}

		if (probe_device_features.framebuffer_is_available_)
		{
			window_param.is_default_depth_buffer_disabled_ = true;
		}

		if (!ogl_renderer_utils.create_window_and_context(window_param, sdl_window_, sdl_gl_context_))
		{
			error_message_ = ogl_renderer_utils.get_error_message();

			return false;
		}

		aa_kind_ = param.aa_kind_;
		aa_value_ = param.aa_value_;
	}

	if (!ogl_renderer_utils.window_get_drawable_size(
		sdl_window_.get(),
		screen_width_,
		screen_height_))
	{
		error_message_ = "Failed to get screen size. ";
		error_message_ += ogl_renderer_utils.get_error_message();

		return false;
	}

	if (!is_probe)
	{
		downscale_width_ = param.downscale_width_;
		downscale_height_ = param.downscale_height_;

		if (downscale_width_ <= 0 || downscale_width_ > param.downscale_width_ ||
			downscale_height_ <= 0 || downscale_height_ > param.downscale_height_)
		{
			error_message_ = "Downscale dimensions out of range.";

			return false;
		}
	}

	extension_manager_ = detail::OglExtensionManagerFactory::create();

	if (extension_manager_ == nullptr)
	{
		error_message_ = "Failed to create an extension manager.";

		return false;
	}

	extension_manager_->probe_extension(OglExtensionId::v2_0);

	if (!extension_manager_->has_extension(OglExtensionId::v2_0))
	{
		error_message_ = "Failed to load OpenGL 2.0 symbols.";

		return false;
	}

	if (!ogl_renderer_utils.renderer_features_set(device_features_))
	{
		error_message_ = ogl_renderer_utils.get_error_message();

		return false;
	}

	ogl_device_features_.context_type_ = OglRendererUtils::context_get_type();

	OglRendererUtils::anisotropy_probe(
		extension_manager_.get(),
		device_features_
	);

	OglRendererUtils::npot_probe(
		extension_manager_.get(),
		device_features_
	);

	OglRendererUtils::mipmap_probe(
		extension_manager_.get(),
		device_features_,
		ogl_device_features_
	);

	OglRendererUtils::framebuffer_probe(
		extension_manager_.get(),
		device_features_,
		ogl_device_features_
	);

	OglRendererUtils::vertex_input_probe_max_locations(device_features_);

	if (device_features_.max_vertex_input_locations_ <= 0)
	{
		error_message_ = "No vertex input locations.";

		return false;
	}

	OglRendererUtils::vsync_probe(device_features_);

	if (!is_probe)
	{
		ogl_state_ = std::move(OglStateImplFactory::create(RendererKind::ogl_2_x));

		auto error_message = std::string{"Failed to initialize the state."};

		if (ogl_state_ == nullptr)
		{
			error_message_ = error_message;

			return false;
		}

		if (!ogl_state_->is_initialized())
		{
			error_message += ' ';
			error_message += ogl_state_->get_error_message();

			return false;
		}
	}

	if (!is_probe)
	{
		if (device_features_.vsync_is_available_)
		{
			static_cast<void>(ogl_renderer_utils.vsync_set(param.is_vsync_));
		}

		if (!framebuffers_create())
		{
			return false;
		}
	}

	if (!create_default_sampler())
	{
		return false;
	}

	if (is_probe)
	{
		probe_.kind_ = RendererKind::ogl_2_x;
		probe_.device_features_ = device_features_;

		uninitialize_internal();
	}
	else
	{
		is_initialized_ = true;

		device_info_ = ogl_renderer_utils.device_info_get();

		// Default state.
		//
		viewport_set_defaults();
		scissor_set_defaults();
		culling_set_defaults();
		depth_set_defaults();
		blending_set_defaults();
		texture_2d_set_defaults();
		vertex_input_defaults();


		// Present.
		//
		clear_buffers();
		present();
	}

	return true;
}

RendererTexture2dPtr Ogl2XRenderer::texture_2d_create(
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

void Ogl2XRenderer::texture_2d_destroy(
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

RendererSamplerPtr Ogl2XRenderer::sampler_create(
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

void Ogl2XRenderer::sampler_destroy(
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

bool Ogl2XRenderer::create_default_sampler()
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

void Ogl2XRenderer::uninitialize_internal(
	const bool is_dtor)
{
	framebuffers_destroy();

	auto ogl_renderer_utils = OglRendererUtils{};

	if (sdl_gl_context_)
	{
		static_cast<void>(ogl_renderer_utils.make_context_current(sdl_window_.get(), nullptr));
	}

	sdl_window_ = {};
	sdl_gl_context_ = {};

	if (!is_dtor)
	{
		device_info_ = {};
		device_features_ = {};
		ogl_device_features_ = {};
		screen_width_ = {};
		screen_height_ = {};
		downscale_width_ = {};
		downscale_height_ = {};
		downscale_blit_filter_ = {};
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
		index_buffers_.clear();
		vertex_buffers_.clear();
		texture_buffer_.clear();
		texture_2d_current_ = {};
		sampler_current_ = {};
		vertex_input_current_ = {};
		vertex_input_enabled_locations_ = {};
		current_shader_stage_ = {};
		ogl_state_ = {};
		extension_manager_ = {};
	}

	index_buffers_.clear();
	vertex_buffers_.clear();
	textures_2d_.clear();
	samplers_.clear();
	sampler_default_ = {};
	vertex_inputs_.clear();
	shaders_.clear();
	shader_stages_.clear();
}

void Ogl2XRenderer::scissor_enable()
{
	detail::OglRendererUtils::scissor_enable(scissor_is_enabled_);
}

void Ogl2XRenderer::scissor_set_box()
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

void Ogl2XRenderer::scissor_set_defaults()
{
	scissor_is_enabled_ = false;

	scissor_x_ = 0;
	scissor_y_ = 0;
	scissor_width_ = screen_width_;
	scissor_height_ = screen_height_;

	scissor_enable();
	scissor_set_box();
}

void Ogl2XRenderer::renderbuffer_destroy(
	GLuint& ogl_renderbuffer_name)
{
	if (ogl_renderbuffer_name == GL_NONE)
	{
		return;
	}

	assert(device_features_.framebuffer_is_available_);

	renderbuffer_bind(GL_NONE);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto delete_renderbuffers = (is_arb ? ::glDeleteRenderbuffers : ::glDeleteRenderbuffersEXT);
	assert(delete_renderbuffers != nullptr);

	delete_renderbuffers(1, &ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());
	ogl_renderbuffer_name = GL_NONE;
}

bool Ogl2XRenderer::renderbuffer_create(
	GLuint& ogl_renderbuffer_name)
{
	assert(ogl_renderbuffer_name == GL_NONE);
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto gen_renderbuffers = (is_arb ? ::glGenRenderbuffers : ::glGenRenderbuffersEXT);
	assert(gen_renderbuffers != nullptr);

	gen_renderbuffers(1, &ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());

	return ogl_renderbuffer_name != GL_NONE;
}

void Ogl2XRenderer::renderbuffer_bind(
	const GLuint ogl_renderbuffer_name)
{
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto renderbuffer_bind = (is_arb ? ::glBindRenderbuffer : ::glBindRenderbufferEXT);

	renderbuffer_bind(GL_RENDERBUFFER, ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::framebuffer_destroy(
	GLuint& ogl_framebuffer_name)
{
	if (ogl_msaa_fbo_ == GL_NONE)
	{
		return;
	}

	assert(device_features_.framebuffer_is_available_);

	framebuffer_bind(GL_FRAMEBUFFER, GL_NONE);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto delete_framebuffers = (is_arb ? ::glDeleteFramebuffers : ::glDeleteFramebuffersEXT);
	assert(delete_framebuffers != nullptr);

	delete_framebuffers(1, &ogl_framebuffer_name);
	assert(!OglRendererUtils::was_errors());
	ogl_framebuffer_name = GL_NONE;
}

bool Ogl2XRenderer::framebuffer_create(
	GLuint& ogl_framebuffer_name)
{
	assert(ogl_framebuffer_name == GL_NONE);
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto gen_framebuffers = (is_arb ? ::glGenFramebuffers : ::glGenFramebuffersEXT);
	assert(gen_framebuffers != nullptr);

	gen_framebuffers(1, &ogl_framebuffer_name);
	assert(!OglRendererUtils::was_errors());

	return ogl_framebuffer_name != GL_NONE;
}

void Ogl2XRenderer::framebuffer_bind(
	const GLenum ogl_target,
	const GLuint ogl_framebuffer_name)
{
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto framebuffer_bind = (is_arb ? ::glBindFramebuffer : ::glBindFramebufferEXT);

	framebuffer_bind(ogl_target, ogl_framebuffer_name);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::framebuffer_blit(
	const int src_width,
	const int src_height,
	const int dst_width,
	const int dst_height,
	const bool is_linear_filter)
{
	assert(src_width > 0);
	assert(src_height > 0);
	assert(dst_width > 0);
	assert(dst_height > 0);

	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;

	const auto framebuffer_blit = (is_arb ? ::glBlitFramebuffer : ::glBlitFramebufferEXT);
	assert(framebuffer_blit != nullptr);

	const auto ogl_filter = (is_linear_filter ? GL_LINEAR : GL_NEAREST);

	framebuffer_blit(
		0,
		0,
		src_width,
		src_height,
		0,
		0,
		dst_width,
		dst_height,
		GL_COLOR_BUFFER_BIT,
		ogl_filter
	);

	assert(!OglRendererUtils::was_errors());
}

bool Ogl2XRenderer::renderbuffer_create(
	const int width,
	const int height,
	const int sample_count,
	const GLenum ogl_internal_format,
	GLuint& ogl_rb_name)
{
	assert(width > 0);
	assert(height > 0);
	assert(sample_count >= 0);
	assert(ogl_internal_format > GL_NONE);

	if (!renderbuffer_create(ogl_rb_name))
	{
		return false;
	}

	renderbuffer_bind(ogl_rb_name);


	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto renderbuffer_storage_multisample = (is_arb ? ::glRenderbufferStorageMultisample : ::glRenderbufferStorageMultisampleEXT);
	assert(renderbuffer_storage_multisample != nullptr);

	renderbuffer_storage_multisample(GL_RENDERBUFFER, sample_count, ogl_internal_format, width, height);
	assert(!OglRendererUtils::was_errors());

	renderbuffer_bind(GL_NONE);

	return true;
}

void Ogl2XRenderer::msaa_color_rb_destroy()
{
	renderbuffer_destroy(ogl_msaa_color_rb_);
}

void Ogl2XRenderer::msaa_depth_rb_destroy()
{
	renderbuffer_destroy(ogl_msaa_depth_rb_);
}

void Ogl2XRenderer::msaa_fbo_destroy()
{
	framebuffer_destroy(ogl_msaa_fbo_);
}

void Ogl2XRenderer::msaa_framebuffer_destroy()
{
	msaa_fbo_destroy();
	msaa_color_rb_destroy();
	msaa_depth_rb_destroy();
}

bool Ogl2XRenderer::msaa_color_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	return renderbuffer_create(width, height, sample_count, GL_RGBA8, ogl_msaa_color_rb_);
}

bool Ogl2XRenderer::msaa_depth_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	return renderbuffer_create(width, height, sample_count, GL_DEPTH_COMPONENT, ogl_msaa_depth_rb_);
}

bool Ogl2XRenderer::msaa_framebuffer_create()
{
	auto sample_count = aa_value_;

	if (sample_count <= 1)
	{
		sample_count = 0;
	}
	else if (sample_count > device_features_.msaa_max_value_)
	{
		sample_count = device_features_.msaa_max_value_;
	}

	if (!msaa_color_rb_create(downscale_width_, downscale_height_, sample_count))
	{
		return false;
	}

	if (!msaa_depth_rb_create(downscale_width_, downscale_height_, sample_count))
	{
		return false;
	}

	if (!framebuffer_create(ogl_msaa_fbo_))
	{
		return false;
	}

	framebuffer_bind(GL_FRAMEBUFFER, ogl_msaa_fbo_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;

	const auto framebuffer_renderbuffer = (is_arb ? ::glFramebufferRenderbuffer : ::glFramebufferRenderbufferEXT);
	assert(framebuffer_renderbuffer != nullptr);

	framebuffer_renderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ogl_msaa_color_rb_);
	framebuffer_renderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ogl_msaa_depth_rb_);

	const auto check_framebuffer_status = (is_arb ? ::glCheckFramebufferStatus : ::glCheckFramebufferStatusEXT);
	assert(check_framebuffer_status != nullptr);

	const auto framebuffer_status = check_framebuffer_status(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	framebuffer_bind(GL_FRAMEBUFFER, GL_NONE);

	return true;
}

void Ogl2XRenderer::downscale_color_rb_destroy()
{
	renderbuffer_destroy(ogl_downscale_color_rb_);
}

void Ogl2XRenderer::downscale_fbo_destroy()
{
	framebuffer_destroy(ogl_downscale_fbo_);
}

void Ogl2XRenderer::downscale_framebuffer_destroy()
{
	downscale_fbo_destroy();
	downscale_color_rb_destroy();
}

bool Ogl2XRenderer::downscale_color_rb_create(
	const int width,
	const int height)
{
	return renderbuffer_create(width, height, 0, GL_RGBA8, ogl_downscale_color_rb_);
}

bool Ogl2XRenderer::downscale_framebuffer_create()
{
	if (!downscale_color_rb_create(downscale_width_, downscale_height_))
	{
		return false;
	}

	if (!framebuffer_create(ogl_downscale_fbo_))
	{
		return false;
	}

	framebuffer_bind(GL_FRAMEBUFFER, ogl_downscale_fbo_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;

	const auto framebuffer_renderbuffer = (is_arb ? ::glFramebufferRenderbuffer : ::glFramebufferRenderbufferEXT);
	assert(framebuffer_renderbuffer != nullptr);

	framebuffer_renderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ogl_downscale_color_rb_);

	const auto check_framebuffer_status = (is_arb ? ::glCheckFramebufferStatus : ::glCheckFramebufferStatusEXT);
	assert(check_framebuffer_status != nullptr);

	const auto framebuffer_status = check_framebuffer_status(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	framebuffer_bind(GL_FRAMEBUFFER, GL_NONE);

	return true;
}

void Ogl2XRenderer::framebuffers_destroy()
{
	msaa_framebuffer_destroy();
	downscale_framebuffer_destroy();
}

bool Ogl2XRenderer::framebuffers_create()
{
	const auto is_downscale = (screen_width_ != downscale_width_ || screen_height_ != downscale_height_);
	const auto is_msaa = (aa_kind_ == RendererAaKind::ms && aa_value_ > RendererUtils::aa_get_min_value());
	const auto is_create_downscale = (is_downscale && is_msaa);

	if (!msaa_framebuffer_create())
	{
		return false;
	}

	if (is_create_downscale)
	{
		if (!downscale_framebuffer_create())
		{
			return false;
		}
	}

	return true;
}

void Ogl2XRenderer::framebuffers_blit()
{
	if (ogl_msaa_fbo_ == GL_NONE && ogl_downscale_fbo_ == GL_NONE)
	{
		return;
	}

	auto is_blit_filter_linear = (downscale_blit_filter_ == RendererFilterKind::linear);

	if (ogl_downscale_fbo_ != GL_NONE)
	{
		// MSAA FBO -> Non-MSAA FBO -> Default FBO
		//

		// Read: MSAA
		// Draw: Non-MSAA
		framebuffer_bind(GL_DRAW_FRAMEBUFFER, ogl_downscale_fbo_);

		framebuffer_blit(
			downscale_width_,
			downscale_height_,
			downscale_width_,
			downscale_height_,
			false
		);

		// Read: Non-MSAA
		// Draw: Default
		framebuffer_bind(GL_READ_FRAMEBUFFER, ogl_downscale_fbo_);
		framebuffer_bind(GL_DRAW_FRAMEBUFFER, GL_NONE);

		framebuffer_blit(
			downscale_width_,
			downscale_height_,
			screen_width_,
			screen_height_,
			is_blit_filter_linear
		);
	}
	else
	{
		// MSAA FBO -> Default FBO
		//

		// Read: MSAA
		// Draw: Default
		framebuffer_bind(GL_DRAW_FRAMEBUFFER, GL_NONE);

		framebuffer_blit(
			downscale_width_,
			downscale_height_,
			screen_width_,
			screen_height_,
			is_blit_filter_linear
		);
	}
}

void Ogl2XRenderer::framebuffers_bind()
{
	if (ogl_msaa_fbo_ == GL_NONE && ogl_downscale_fbo_ == GL_NONE)
	{
		return;
	}

	framebuffer_bind(GL_FRAMEBUFFER, ogl_msaa_fbo_);
}

void Ogl2XRenderer::aa_disable()
{
	if (ogl_msaa_fbo_ == GL_NONE)
	{
		return;
	}

	if (aa_value_ <= RendererUtils::aa_get_min_value())
	{
		return;
	}

	msaa_framebuffer_destroy();
	static_cast<void>(msaa_framebuffer_create());
}

bool Ogl2XRenderer::msaa_set(
	const int aa_value)
{
	if (ogl_msaa_fbo_ == GL_NONE)
	{
		error_message_ = "No off-screen framebuffer.";

		return false;
	}

	if (aa_kind_ == RendererAaKind::ms ||
		aa_value_ == aa_value)
	{
		return true;
	}

	aa_kind_ = RendererAaKind::ms;
	aa_value_ = aa_value;

	framebuffers_destroy();

	return framebuffers_create();
}

void Ogl2XRenderer::viewport_set_rectangle()
{
	detail::OglRendererUtils::viewport_set_rectangle(
		viewport_x_,
		viewport_y_,
		viewport_width_,
		viewport_height_
	);
}

void Ogl2XRenderer::viewport_set_depth_range()
{
	detail::OglRendererUtils::viewport_set_depth_range(
		viewport_min_depth_,
		viewport_max_depth_
	);
}

void Ogl2XRenderer::viewport_set_defaults()
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

void Ogl2XRenderer::culling_enabled()
{
	const auto ogl_function = (culling_is_enabled_ ? ::glEnable : ::glDisable);

	ogl_function(GL_CULL_FACE);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::culling_set_face()
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

void Ogl2XRenderer::culling_set_mode()
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

void Ogl2XRenderer::culling_set_defaults()
{
	culling_is_enabled_ = false;
	culling_enabled();

	culling_face_ = RendererCullingFace::counter_clockwise;
	culling_set_face();

	culling_mode_ = RendererCullingMode::back;
	culling_set_mode();
}

void Ogl2XRenderer::depth_set_test()
{
	const auto ogl_function = (depth_is_test_enabled_ ? ::glEnable : ::glDisable);

	ogl_function(GL_DEPTH_TEST);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::depth_set_write()
{
	::glDepthMask(depth_is_write_enabled_);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::depth_set_defaults()
{
	depth_is_test_enabled_ = false;
	depth_set_test();

	depth_is_write_enabled_ = false;
	depth_set_write();
}

void Ogl2XRenderer::blending_enable()
{
	const auto ogl_function = (blending_is_enabled_ ? ::glEnable : ::glDisable);

	ogl_function(GL_BLEND);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::blending_set_function()
{
	OglRendererUtils::blending_set_function(blending_src_factor_, blending_dst_factor_);
}

void Ogl2XRenderer::blending_set_defaults()
{
	blending_is_enabled_ = false;
	blending_enable();

	blending_src_factor_ = RendererBlendingFactor::src_alpha;
	blending_dst_factor_ = RendererBlendingFactor::one_minus_src_alpha;
	blending_set_function();
}

void Ogl2XRenderer::texture_2d_enable()
{
	::glEnable(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::texture_set()
{
	auto ogl_texture_name = GLuint{};

	if (texture_2d_current_)
	{
		ogl_texture_name = texture_2d_current_->ogl_id_;
	}

	OglRendererUtils::texture_2d_set(ogl_texture_name);
}

void Ogl2XRenderer::texture_set(
	Texture2dPtr new_texture_2d)
{
	if (texture_2d_current_ != new_texture_2d)
	{
		texture_2d_current_ = new_texture_2d;

		texture_set();
	}
}

void Ogl2XRenderer::texture_mipmap_generation_set_hint()
{
	if (!device_features_.mipmap_is_available_)
	{
		return;
	}

	if (ogl_device_features_.context_type_ == OglRendererUtilsContextType::core)
	{
		return;
	}

	::glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::texture_2d_set_defaults()
{
	texture_2d_is_enabled_ = true;
	texture_2d_enable();

	texture_2d_current_ = nullptr;
	texture_set();

	texture_mipmap_generation_set_hint();
}

void Ogl2XRenderer::sampler_set()
{
	assert(sampler_current_);

	for (auto& texture_2d : textures_2d_)
	{
		texture_2d->update_sampler_state(sampler_default_->state_);
	}
}

void Ogl2XRenderer::vertex_input_enable_client_state(
	const bool is_enabled,
	const GLenum state)
{
	const auto ogl_function = (is_enabled ? ::glEnableClientState : ::glDisableClientState);

	ogl_function(state);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::vertex_input_assign_default_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	assert(attribute_description.is_default_);

	::glVertexAttrib4fv(
		attribute_description.location_,
		glm::value_ptr(attribute_description.default_value_)
	);

	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::vertex_input_assign_regular_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	assert(!attribute_description.is_default_);

	auto ogl_component_count = GLint{};
	auto ogl_component_format = GLenum{};
	auto ogl_is_normalized = GLenum{};

	switch (attribute_description.format_)
	{
		case RendererVertexAttributeFormat::r8g8b8a8_unorm:
			ogl_is_normalized = true;
			ogl_component_count = 4;
			ogl_component_format = GL_UNSIGNED_BYTE;
			break;

		case RendererVertexAttributeFormat::r32g32_sfloat:
			ogl_component_count = 2;
			ogl_component_format = GL_FLOAT;
			break;

		case RendererVertexAttributeFormat::r32g32b32_sfloat:
			ogl_component_count = 3;
			ogl_component_format = GL_FLOAT;
			break;

		default:
			assert(!"Invalid format.");
			break;
	}

	vertex_input_enable_location(attribute_description.location_, true);

	auto vertex_buffer = static_cast<VertexBufferImplPtr>(attribute_description.vertex_buffer_);

	vertex_buffer->bind(true);

	const auto vertex_buffer_data = reinterpret_cast<const void*>(static_cast<std::intptr_t>(attribute_description.offset_));

	::glVertexAttribPointer(
		attribute_description.location_,
		ogl_component_count,
		ogl_component_format,
		ogl_is_normalized,
		attribute_description.stride_,
		vertex_buffer_data
	);

	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::vertex_input_assign_attribute(
	const RendererVertexAttributeDescription& attribute_description)
{
	if (attribute_description.is_default_)
	{
		vertex_input_assign_default_attribute(attribute_description);
	}
	else
	{
		vertex_input_assign_regular_attribute(attribute_description);
	}
}

void Ogl2XRenderer::vertex_input_assign()
{
	// Disable all.
	//
	for (int i = 0; i < device_features_.max_vertex_input_locations_; ++i)
	{
		vertex_input_enable_location(i, false);
	}

	if (vertex_input_current_ == nullptr)
	{
		return;
	}

	for (const auto& attribute_description : vertex_input_current_->attribute_descriptions_)
	{
		vertex_input_assign_attribute(attribute_description);
	}
}

void Ogl2XRenderer::vertex_input_enable_location(
	const int location)
{
	const auto is_enabled = vertex_input_enabled_locations_[location];
	const auto olg_function = (is_enabled ? ::glEnableVertexAttribArray : ::glDisableVertexAttribArray);

	olg_function(static_cast<GLuint>(location));
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::vertex_input_enable_location(
	const int location,
	const bool is_enabled)
{
	if (vertex_input_enabled_locations_[location] == is_enabled)
	{
		return;
	}

	vertex_input_enabled_locations_[location] = is_enabled;

	vertex_input_enable_location(location);
}

void Ogl2XRenderer::vertex_input_defaults()
{
	vertex_input_current_ = nullptr;

	vertex_input_enabled_locations_.clear();
	vertex_input_enabled_locations_.resize(device_features_.max_vertex_input_locations_);

	for (int i = 0; i < device_features_.max_vertex_input_locations_; ++i)
	{
		vertex_input_enable_location(i);
	}
}

void Ogl2XRenderer::command_execute_culling(
	const RendererCommandCulling& command)
{
	if (culling_is_enabled_ != command.is_enabled_)
	{
		culling_is_enabled_ = command.is_enabled_;

		culling_enabled();
	}
}

void Ogl2XRenderer::command_execute_depth_test(
	const RendererCommandDepthTest& command)
{
	if (depth_is_test_enabled_ != command.is_enabled_)
	{
		depth_is_test_enabled_ = command.is_enabled_;

		depth_set_test();
	}
}

void Ogl2XRenderer::command_execute_depth_write(
	const RendererCommandDepthWrite& command)
{
	if (depth_is_write_enabled_ != command.is_enabled_)
	{
		depth_is_write_enabled_ = command.is_enabled_;

		depth_set_write();
	}
}

void Ogl2XRenderer::command_execute_viewport(
	const RendererCommandViewport& command)
{
	assert(command.x_ < screen_width_);
	assert(command.y_ < screen_height_);
	assert(command.width_ <= screen_width_);
	assert(command.height_ <= screen_height_);
	assert((command.x_ + command.width_) <= screen_width_);
	assert((command.y_ + command.height_) <= screen_height_);

	if (viewport_width_ > device_features_.max_viewport_width_ ||
		viewport_height_ > device_features_.max_viewport_height_)
	{
		assert(!"Viewport dimensions out of range.");

		return;
	}

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

void Ogl2XRenderer::command_execute_blending(
	const RendererCommandBlending& command)
{
	if (blending_is_enabled_ != command.is_enabled_)
	{
		blending_is_enabled_ = command.is_enabled_;

		blending_enable();
	}
}

void Ogl2XRenderer::command_execute_blending_function(
	const RendererCommandBlendingFunction& command)
{
	if (blending_src_factor_ != command.src_factor_ ||
		blending_dst_factor_ != command.dst_factor_)
	{
		blending_src_factor_ = command.src_factor_;
		blending_dst_factor_ = command.dst_factor_;

		blending_set_function();
	}
}

void Ogl2XRenderer::command_execute_scissor(
	const RendererCommandScissor& command)
{
	if (scissor_is_enabled_ != command.is_enabled_)
	{
		scissor_is_enabled_ = command.is_enabled_;

		scissor_enable();
	}
}

void Ogl2XRenderer::command_execute_scissor_box(
	const RendererCommandScissorBox& command)
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

void Ogl2XRenderer::command_execute_texture(
	const RendererCommandTexture& command)
{
	texture_set(static_cast<Texture2dPtr>(command.texture_2d_));
}

void Ogl2XRenderer::command_execute_sampler(
	const RendererCommandSampler& command)
{
	assert(command.sampler_);

	auto sampler = static_cast<SamplerPtr>(command.sampler_);

	if (!sampler)
	{
		return;
	}

	sampler_current_ = sampler;
}

void Ogl2XRenderer::command_execute_vertex_input(
	const RendererCommandVertexInput& command)
{
	if (vertex_input_current_ != command.vertex_input_)
	{
		vertex_input_current_ = static_cast<VertexInputPtr>(command.vertex_input_);

		vertex_input_assign();
	}
}

void Ogl2XRenderer::command_execute_shader_stage(
	const RendererCommandShaderStage& command)
{
	if (command.shader_stage_ != nullptr)
	{
		command.shader_stage_->set_current();
	}
	else
	{
		OglShaderStage::unset_current();
	}
}

void Ogl2XRenderer::command_execute_shader_variable_int32(
	const RendererCommandShaderVariableInt32& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_variable_float32(
	const RendererCommandShaderVariableFloat32& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_variable_vec2(
	const RendererCommandShaderVariableVec2& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_variable_vec4(
	const RendererCommandShaderVariableVec4& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_variable_mat4(
	const RendererCommandShaderVariableMat4& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_variable_sampler_2d(
	const RendererCommandShaderVariableSampler2d& command)
{
	if (!command.variable_)
	{
		assert(!"Null variable.");

		return;
	}

	command.variable_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_draw_quads(
	const RendererCommandDrawQuads& command)
{
	assert(command.count_ > 0);
	assert(command.index_offset_ >= 0);
	assert(vertex_input_current_);

	const auto triangles_per_quad = 2;
	const auto triangle_count = command.count_ * triangles_per_quad;

	const auto indices_per_triangle = 3;
	const auto indices_per_quad = triangles_per_quad * indices_per_triangle;
	const auto index_count = indices_per_quad * command.count_;

	auto index_buffer = static_cast<IndexBufferImplPtr>(vertex_input_current_->index_buffer_);

	const auto index_byte_depth = index_buffer->get_byte_depth();
	const auto max_index_count = index_buffer->get_size() / index_byte_depth;

	const auto index_byte_offset = command.index_offset_ * index_byte_depth;
	assert(command.index_offset_ < max_index_count);
	assert(command.count_ <= max_index_count);
	assert((command.index_offset_ + command.count_) <= max_index_count);


	// Sampler state.
	//
	if (texture_2d_current_)
	{
		texture_2d_current_->update_sampler_state(sampler_current_->state_);
	}

	// Draw the quads.
	//

	const auto index_buffer_data = reinterpret_cast<const void*>(static_cast<std::intptr_t>(index_byte_offset));

	const auto ogl_element_type = OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
		index_buffer->get_byte_depth());

	index_buffer->bind(true);

	::glDrawElements(
		GL_TRIANGLES, // mode
		index_count, // count
		ogl_element_type, // type
		index_buffer_data // indices
	);

	assert(!OglRendererUtils::was_errors());
}

//
// Ogl2XRenderer
// ==========================================================================


} // detail
} // bstone
