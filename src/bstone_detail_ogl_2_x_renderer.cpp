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

#include "bstone_exception.h"
#include "bstone_renderer_limits.h"

#include "bstone_detail_ogl_buffer_manager.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler_manager.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_ogl_vertex_input_manager.h"
#include "bstone_detail_ogl_shader_manager.h"
#include "bstone_detail_ogl_shader_stage_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ogl2XRenderer
//

Ogl2XRenderer::Ogl2XRenderer(
	const RendererCreateParam& param)
	:
	sdl_window_{},
	sdl_gl_context_{},
	extension_manager_{},
	ogl_context_{},
	command_executor_{},
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
	ogl_downscale_color_rb_{}
{
	initialize(param);
}

Ogl2XRenderer::~Ogl2XRenderer()
{
	uninitialize_internal(true);
}

RendererKind Ogl2XRenderer::get_kind() const noexcept
{
	return RendererKind::ogl_2_x;
}

const std::string& Ogl2XRenderer::get_name() const noexcept
{
	static const auto result = std::string{"ogl2"};

	return result;
}

const std::string& Ogl2XRenderer::get_description() const noexcept
{
	static const auto result = std::string{"OpenGL 2.x"};

	return result;
}

void Ogl2XRenderer::fbo_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	const auto ogl_function = (::glDeleteFramebuffers ? ::glDeleteFramebuffers : ::glDeleteFramebuffersEXT);
	ogl_function(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::rbo_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	const auto ogl_function = (::glDeleteRenderbuffers ? ::glDeleteRenderbuffers : ::glDeleteRenderbuffersEXT);
	ogl_function(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void Ogl2XRenderer::initialize(
	const RendererCreateParam& param)
{
	OglRendererUtils::msaa_probe(device_features_, ogl_device_features_);

	aa_kind_ = param.aa_kind_;
	aa_value_ = param.aa_value_;

	auto window_param = RendererUtilsCreateWindowParam{};
	window_param.is_opengl_ = true;
	window_param.window_ = param.window_;
	window_param.aa_kind_ = aa_kind_;
	window_param.aa_value_ = aa_value_;

	if (window_param.window_.width_ == 0 || window_param.window_.height_ == 0)
	{
		window_param.window_.width_ = 1;
		window_param.window_.height_ = 1;
	}

	if (window_param.aa_kind_ == RendererAaKind::ms)
	{
		if (device_features_.msaa_is_available_)
		{
			if (aa_value_ < device_features_.msaa_min_value_)
			{
				aa_value_ = device_features_.msaa_min_value_;
			}

			if (aa_value_ > device_features_.msaa_max_value_)
			{
				aa_value_ = device_features_.msaa_max_value_;
			}

			if (device_features_.msaa_is_window_)
			{
				window_param.aa_value_ = aa_value_;
			}
			else
			{
				window_param.aa_kind_ = RendererAaKind::none;
				window_param.aa_value_ = 0;
				window_param.is_default_depth_buffer_disabled_ = true;
			}
		}
		else
		{
			window_param.aa_kind_ = RendererAaKind::none;
			window_param.aa_value_ = 0;
		}
	}

	OglRendererUtils::create_window_and_context(window_param, sdl_window_, sdl_gl_context_);

	OglRendererUtils::window_get_drawable_size(
		sdl_window_.get(),
		screen_width_,
		screen_height_);

	if (screen_width_ == 0 || screen_height_ == 0)
	{
		throw Exception{"Failed to get screen size."};
	}

	if (aa_kind_ == RendererAaKind::ms && device_features_.msaa_is_window_)
	{
		aa_value_ = OglRendererUtils::msaa_window_get_value();
	}

	downscale_width_ = param.downscale_width_;
	downscale_height_ = param.downscale_height_;

	if (downscale_width_ <= 0 || downscale_width_ > param.downscale_width_ ||
		downscale_height_ <= 0 || downscale_height_ > param.downscale_height_)
	{
		throw Exception{"Downscale dimensions out of range."};
	}

	extension_manager_ = detail::OglExtensionManagerFactory::create();

	if (extension_manager_ == nullptr)
	{
		throw Exception{"Failed to create an extension manager."};
	}

	extension_manager_->probe(OglExtensionId::v2_0);

	if (!extension_manager_->has(OglExtensionId::v2_0))
	{
		throw Exception{"Failed to load OpenGL 2.0 symbols."};
	}

	OglRendererUtils::renderer_features_set(device_features_);

	ogl_device_features_.context_kind_ = OglRendererUtils::context_get_kind();

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

	OglRendererUtils::sampler_probe(
		extension_manager_.get(),
		device_features_
	);

	OglRendererUtils::vertex_input_probe_max_locations(device_features_);

	OglRendererUtils::buffer_storage_probe(
		extension_manager_.get(),
		ogl_device_features_
	);

	OglRendererUtils::dsa_probe(
		extension_manager_.get(),
		ogl_device_features_
	);

	OglRendererUtils::sso_probe(
		extension_manager_.get(),
		ogl_device_features_
	);

	if (device_features_.vertex_input_max_locations_ <= 0)
	{
		throw Exception{"No vertex input locations."};
	}

	OglRendererUtils::vsync_probe(device_features_);

	OglRendererUtils::vertex_input_vao_probe(
		extension_manager_.get(),
		ogl_device_features_
	);

	ogl_context_ = OglContextFactory::create(
		RendererKind::ogl_2_x,
		device_features_,
		ogl_device_features_
	);

	command_executor_ = OglCommandExecutorFactory::create(ogl_context_.get());

	if (device_features_.vsync_is_available_)
	{
		static_cast<void>(OglRendererUtils::vsync_set(param.is_vsync_));
	}

	framebuffers_create();

	device_info_ = OglRendererUtils::device_info_get();


	// Present.
	//
	clear_buffers();
	present();
}

void Ogl2XRenderer::uninitialize()
{
	uninitialize_internal();
}

const RendererDeviceFeatures& Ogl2XRenderer::device_get_features() const noexcept
{
	return device_features_;
}

const RendererDeviceInfo& Ogl2XRenderer::device_get_info() const noexcept
{
	return device_info_;
}

bool Ogl2XRenderer::device_is_lost() const noexcept
{
	return false;
}

bool Ogl2XRenderer::device_is_ready_to_reset() const noexcept
{
	return true;
}

void Ogl2XRenderer::device_reset()
{
}

void Ogl2XRenderer::window_show(
	const bool is_visible)
{
	RendererUtils::show_window(sdl_window_.get(), is_visible);
}

const glm::mat4& Ogl2XRenderer::csc_get_texture() const noexcept
{
	return detail::OglRendererUtils::csc_get_texture();
}

const glm::mat4& Ogl2XRenderer::csc_get_projection() const noexcept
{
	return detail::OglRendererUtils::csc_get_projection();
}

bool Ogl2XRenderer::vsync_get() const noexcept
{
	if (!device_features_.vsync_is_available_)
	{
		return false;
	}

	return OglRendererUtils::vsync_get();
}

void Ogl2XRenderer::vsync_set(
	const bool is_enabled)
{
	if (!device_features_.vsync_is_available_)
	{
		throw Exception{"Not available."};
	}

	if (device_features_.vsync_is_requires_restart_)
	{
		throw Exception{"Requires restart."};
	}

	if (!OglRendererUtils::vsync_set(is_enabled))
	{
		throw Exception{"Not supported."};
	}
}

void Ogl2XRenderer::downscale_set(
	const int width,
	const int height,
	const RendererFilterKind blit_filter)
{
	if (width <= 0 ||
		width > screen_width_ ||
		height <= 0 ||
		height > screen_height_)
	{
		throw Exception{"Dimensions out of range."};
	}

	switch (blit_filter)
	{
		case RendererFilterKind::nearest:
		case RendererFilterKind::linear:
			break;

		default:
			throw Exception{"Unsupported blit filter."};
	}

	if (!device_features_.framebuffer_is_available_)
	{
		throw Exception{"Off-screen framebuffer not supported."};
	}

	if (ogl_msaa_fbo_ == 0)
	{
		throw Exception{"No off-screen framebuffer."};
	}

	downscale_width_ = width;
	downscale_height_ = height;
	downscale_blit_filter_ = blit_filter;

	framebuffers_destroy();

	framebuffers_create();
}

void Ogl2XRenderer::aa_set(
	const RendererAaKind aa_kind,
	const int aa_value)
{
	switch (aa_kind)
	{
		case RendererAaKind::none:
		case RendererAaKind::ms:
			break;

		default:
			throw Exception{"Invalid anti-aliasing kind."};
	}

	auto clamped_aa_value = aa_value;

	if (clamped_aa_value < RendererLimits::aa_min)
	{
		clamped_aa_value = RendererLimits::aa_min;
	}

	if (clamped_aa_value > RendererLimits::aa_max)
	{
		clamped_aa_value = RendererLimits::aa_max;
	}

	switch (aa_kind)
	{
		case RendererAaKind::none:
			aa_disable();
			return;

		case RendererAaKind::ms:
			msaa_set(clamped_aa_value);
			return;

		default:
			throw Exception{"Invalid anti-aliasing kind."};
	}
}

void Ogl2XRenderer::color_buffer_set_clear_color(
	const R8g8b8a8& color)
{
	OglRendererUtils::set_color_buffer_clear_color(color);
}

void Ogl2XRenderer::clear_buffers()
{
	framebuffers_bind();

	OglRendererUtils::clear_buffers();
}

void Ogl2XRenderer::present()
{
	framebuffers_blit();

	OglRendererUtils::swap_window(sdl_window_.get());
}

RendererIndexBufferPtr Ogl2XRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	return ogl_context_->buffer_get_manager()->index_buffer_create(param);
}

void Ogl2XRenderer::index_buffer_destroy(
	RendererIndexBufferPtr index_buffer)
{
	ogl_context_->buffer_get_manager()->buffer_destroy(index_buffer);
}

RendererVertexBufferPtr Ogl2XRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	return ogl_context_->buffer_get_manager()->vertex_buffer_create(param);
}

void Ogl2XRenderer::vertex_buffer_destroy(
	RendererVertexBufferPtr vertex_buffer)
{
	ogl_context_->buffer_get_manager()->buffer_destroy(vertex_buffer);
}

RendererVertexInputPtr Ogl2XRenderer::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	return ogl_context_->vertex_input_get_manager()->create(param);
}

void Ogl2XRenderer::vertex_input_destroy(
	RendererVertexInputPtr vertex_input)
{
	ogl_context_->vertex_input_get_manager()->destroy(vertex_input);
}

RendererShaderPtr Ogl2XRenderer::shader_create(
	const RendererShaderCreateParam& param)
{
	return ogl_context_->shader_get_manager()->create(param);
}

void Ogl2XRenderer::shader_destroy(
	const RendererShaderPtr shader)
{
	return ogl_context_->shader_get_manager()->destroy(shader);
}

RendererShaderStagePtr Ogl2XRenderer::shader_stage_create(
	const RendererShaderStageCreateParam& param)
{
	return ogl_context_->shader_stage_get_manager()->create(param);
}

void Ogl2XRenderer::shader_stage_destroy(
	const RendererShaderStagePtr shader_stage)
{
	return ogl_context_->shader_stage_get_manager()->destroy(shader_stage);
}

void Ogl2XRenderer::execute_commands(
	const RendererCommandManagerPtr command_manager)
{
	command_executor_->execute(command_manager);
}

RendererTexture2dPtr Ogl2XRenderer::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	return ogl_context_->texture_get_manager()->create(param);
}

void Ogl2XRenderer::texture_2d_destroy(
	RendererTexture2dPtr texture_2d)
{
	ogl_context_->texture_get_manager()->destroy(texture_2d);
}

RendererSamplerPtr Ogl2XRenderer::sampler_create(
	const RendererSamplerCreateParam& param)
{
	return ogl_context_->sampler_get_manager()->create(param);
}

void Ogl2XRenderer::sampler_destroy(
	RendererSamplerPtr sampler)
{
	ogl_context_->sampler_get_manager()->destroy(sampler);
}

void Ogl2XRenderer::uninitialize_internal(
	const bool is_dtor)
{
	command_executor_ = {};
	ogl_context_ = {};
	extension_manager_ = {};

	framebuffers_destroy();

	if (sdl_gl_context_)
	{
		OglRendererUtils::make_context_current(sdl_window_.get(), nullptr);
	}

	sdl_gl_context_ = {};
	sdl_window_ = {};

	device_info_ = {};
	device_features_ = {};
	ogl_device_features_ = {};
	screen_width_ = {};
	screen_height_ = {};
	downscale_width_ = {};
	downscale_height_ = {};
	downscale_blit_filter_ = {};
}

Ogl2XRenderer::RboResource Ogl2XRenderer::renderbuffer_create()
{
	if (!device_features_.framebuffer_is_available_)
	{
		throw Exception{"Framebuffer not available."};
	}

	const auto ogl_function = (ogl_device_features_.framebuffer_is_ext_ ? ::glGenRenderbuffersEXT : ::glGenRenderbuffers);

	auto ogl_name = GLuint{};
	ogl_function(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		throw Exception{"Failed to create OpenGL renderbuffer object."};
	}

	return RboResource{ogl_name};
}

void Ogl2XRenderer::renderbuffer_bind(
	const GLuint ogl_renderbuffer_name)
{
	const auto ogl_func = (ogl_device_features_.framebuffer_is_ext_ ? ::glBindRenderbufferEXT : ::glBindRenderbuffer);

	ogl_func(GL_RENDERBUFFER, ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());
}

Ogl2XRenderer::FboResource Ogl2XRenderer::framebuffer_create()
{
	if (!device_features_.framebuffer_is_available_)
	{
		throw Exception{"Framebuffer not available."};
	}

	const auto ogl_func = (ogl_device_features_.framebuffer_is_ext_ ? ::glGenFramebuffersEXT : ::glGenFramebuffers);

	auto ogl_name = GLuint{};
	ogl_func(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		throw Exception{"Failed to create OpenGL framebuffer object."};
	}

	return FboResource{ogl_name};
}

void Ogl2XRenderer::framebuffer_bind(
	const GLenum ogl_target,
	const GLuint ogl_name)
{
	assert(device_features_.framebuffer_is_available_);

	const auto ogl_func = (ogl_device_features_.framebuffer_is_ext_ ? ::glBindFramebufferEXT : ::glBindFramebuffer);

	ogl_func(ogl_target, ogl_name);
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

	const auto ogl_func = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glBlitFramebufferEXT :
		::glBlitFramebuffer
	);

	const auto ogl_filter = (is_linear_filter ? GL_LINEAR : GL_NEAREST);

	ogl_func(
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

Ogl2XRenderer::RboResource Ogl2XRenderer::renderbuffer_create(
	const int width,
	const int height,
	const int sample_count,
	const GLenum ogl_internal_format)
{
	assert(width > 0);
	assert(height > 0);
	assert(sample_count >= 0);
	assert(ogl_internal_format > 0);

	auto rbo_resource = renderbuffer_create();
	renderbuffer_bind(rbo_resource.get());

	assert(device_features_.framebuffer_is_available_);

	const auto ogl_func = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glRenderbufferStorageMultisampleEXT :
		::glRenderbufferStorageMultisample
	);

	ogl_func(GL_RENDERBUFFER, sample_count, ogl_internal_format, width, height);
	assert(!OglRendererUtils::was_errors());

	renderbuffer_bind(0);

	return rbo_resource;
}

void Ogl2XRenderer::msaa_color_rb_destroy()
{
	ogl_msaa_color_rb_.reset();
}

void Ogl2XRenderer::msaa_depth_rb_destroy()
{
	ogl_msaa_depth_rb_.reset();
}

void Ogl2XRenderer::msaa_fbo_destroy()
{
	ogl_msaa_fbo_.reset();
}

void Ogl2XRenderer::msaa_framebuffer_destroy()
{
	msaa_fbo_destroy();
	msaa_color_rb_destroy();
	msaa_depth_rb_destroy();
}

void Ogl2XRenderer::msaa_color_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	ogl_msaa_color_rb_ = renderbuffer_create(width, height, sample_count, GL_RGBA8);
}

void Ogl2XRenderer::msaa_depth_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	ogl_msaa_depth_rb_ = renderbuffer_create(width, height, sample_count, GL_DEPTH_COMPONENT);
}

void Ogl2XRenderer::msaa_framebuffer_create()
{
	msaa_color_rb_create(downscale_width_, downscale_height_, aa_value_);
	msaa_depth_rb_create(downscale_width_, downscale_height_, aa_value_);

	ogl_msaa_fbo_ = framebuffer_create();
	framebuffer_bind(GL_FRAMEBUFFER, ogl_msaa_fbo_);

	const auto framebuffer_renderbuffer_func = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glFramebufferRenderbufferEXT :
		::glFramebufferRenderbuffer
	);

	framebuffer_renderbuffer_func(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER,
		ogl_msaa_color_rb_
	);

	framebuffer_renderbuffer_func(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		ogl_msaa_depth_rb_
	);

	const auto check_framebuffer_status_func = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glCheckFramebufferStatusEXT :
		::glCheckFramebufferStatus
	);

	const auto framebuffer_status = check_framebuffer_status_func(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw Exception{"Incomplete framebuffer object."};
	}

	framebuffer_bind(GL_FRAMEBUFFER, 0);
}

void Ogl2XRenderer::downscale_color_rb_destroy()
{
	ogl_downscale_color_rb_.reset();
}

void Ogl2XRenderer::downscale_fbo_destroy()
{
	ogl_downscale_fbo_.reset();
}

void Ogl2XRenderer::downscale_framebuffer_destroy()
{
	downscale_fbo_destroy();
	downscale_color_rb_destroy();
}

void Ogl2XRenderer::downscale_color_rb_create(
	const int width,
	const int height)
{
	ogl_downscale_color_rb_ = renderbuffer_create(width, height, 0, GL_RGBA8);
}

void Ogl2XRenderer::downscale_framebuffer_create()
{
	downscale_color_rb_create(downscale_width_, downscale_height_);
	ogl_downscale_fbo_ = framebuffer_create();

	framebuffer_bind(GL_FRAMEBUFFER, ogl_downscale_fbo_);

	const auto framebuffer_renderbuffer = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glFramebufferRenderbufferEXT :
		::glFramebufferRenderbuffer
	);

	framebuffer_renderbuffer(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER,
		ogl_downscale_color_rb_
	);

	const auto check_framebuffer_status = (
		ogl_device_features_.framebuffer_is_ext_ ?
		::glCheckFramebufferStatusEXT :
		::glCheckFramebufferStatus
	);

	const auto framebuffer_status = check_framebuffer_status(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw Exception{"Incomplete framebuffer object."};
	}

	framebuffer_bind(GL_FRAMEBUFFER, 0);
}

void Ogl2XRenderer::framebuffers_destroy()
{
	msaa_framebuffer_destroy();
	downscale_framebuffer_destroy();
}

void Ogl2XRenderer::framebuffers_create()
{
	if (!device_features_.framebuffer_is_available_)
	{
		return;
	}

	const auto is_downscale = (screen_width_ != downscale_width_ || screen_height_ != downscale_height_);
	const auto is_msaa = (aa_kind_ == RendererAaKind::ms && aa_value_ >= RendererLimits::aa_min);
	const auto is_create_downscale = (is_downscale && is_msaa);

	msaa_framebuffer_create();

	if (is_create_downscale)
	{
		downscale_framebuffer_create();
	}
}

void Ogl2XRenderer::framebuffers_blit()
{
	if (ogl_msaa_fbo_ == 0 && ogl_downscale_fbo_ == 0)
	{
		return;
	}

	auto is_blit_filter_linear = (downscale_blit_filter_ == RendererFilterKind::linear);

	if (ogl_downscale_fbo_ != 0)
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
		framebuffer_bind(GL_DRAW_FRAMEBUFFER, 0);

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
		framebuffer_bind(GL_DRAW_FRAMEBUFFER, 0);

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
	if (ogl_msaa_fbo_ == 0 && ogl_downscale_fbo_ == 0)
	{
		return;
	}

	framebuffer_bind(GL_FRAMEBUFFER, ogl_msaa_fbo_);
}

void Ogl2XRenderer::aa_disable()
{
	aa_kind_ = RendererAaKind::none;

	if (ogl_msaa_fbo_ == 0)
	{
		return;
	}

	msaa_framebuffer_destroy();
	static_cast<void>(msaa_framebuffer_create());
}

void Ogl2XRenderer::msaa_set(
	const int aa_value)
{
	if (device_features_.msaa_is_requires_restart_)
	{
		throw Exception{"Requires restart."};
	}

	if (!device_features_.framebuffer_is_available_)
	{
		throw Exception{"Framebuffer not available."};
	}

	if (aa_kind_ == RendererAaKind::ms && aa_value_ == aa_value)
	{
		return;
	}

	aa_kind_ = RendererAaKind::ms;
	aa_value_ = aa_value;

	framebuffers_destroy();

	framebuffers_create();
}

//
// Ogl2XRenderer
// ==========================================================================


} // detail
} // bstone
