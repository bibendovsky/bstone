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
// OpenGL renderer (implementation).
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_detail_ogl_renderer.h"

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
// OglRenderer
//

OglRenderer::OglRenderer(
	const RendererCreateParam& param)
	:
	kind_{},
	name_{},
	description_{},
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
	aa_kind_{},
	aa_value_{},
	ogl_msaa_fbo_{},
	ogl_msaa_color_rb_{},
	ogl_msaa_depth_rb_{}
{
	initialize(param);
}

OglRenderer::~OglRenderer()
{
	uninitialize_internal();
}

RendererKind OglRenderer::get_kind() const noexcept
{
	return kind_;
}

const std::string& OglRenderer::get_name() const noexcept
{
	return name_;
}

const std::string& OglRenderer::get_description() const noexcept
{
	return description_;
}

void OglRenderer::fbo_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	const auto ogl_function = (::glDeleteFramebuffers ? ::glDeleteFramebuffers : ::glDeleteFramebuffersEXT);
	ogl_function(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void OglRenderer::rbo_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	const auto ogl_function = (::glDeleteRenderbuffers ? ::glDeleteRenderbuffers : ::glDeleteRenderbuffersEXT);
	ogl_function(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void OglRenderer::set_name_and_description()
{
	switch (kind_)
	{
		case RendererKind::ogl_2:
			name_ = "GL2";
			description_ = "OpenGL 2.0+";
			break;

		case RendererKind::ogl_3_2_core:
			name_ = "GL3.2C";
			description_ = "OpenGL 3.2 core";
			break;

		case RendererKind::ogl_es_2_0:
			name_ = "GLES2.0";
			description_ = "OpenGL ES 2.0";
			break;

		default:
			throw Exception{"Unsupported renderer kind."};
	}
}

void OglRenderer::initialize(
	const RendererCreateParam& param)
{
	switch (param.renderer_kind_)
	{
		case RendererKind::ogl_2:
		case RendererKind::ogl_3_2_core:
		case RendererKind::ogl_es_2_0:
			break;

		default:
			throw Exception{"Unsupported renderer kind."};
	}

	kind_ = param.renderer_kind_;

	OglRendererUtils::msaa_probe(kind_, device_features_, ogl_device_features_);

	aa_kind_ = param.aa_kind_;
	aa_value_ = param.aa_value_;

	auto window_param = RendererUtilsCreateWindowParam{};
	window_param.renderer_kind_ = kind_;
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
			if (aa_value_ <= 0)
			{
				aa_value_ = device_features_.msaa_max_value_;
			}

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

	extension_manager_ = detail::OglExtensionManagerFactory::create();

	if (extension_manager_ == nullptr)
	{
		throw Exception{"Failed to create an extension manager."};
	}

	switch (kind_)
	{
		case RendererKind::ogl_2:
			extension_manager_->probe(OglExtensionId::v2_0);

			if (!extension_manager_->has(OglExtensionId::v2_0))
			{
				throw Exception{"Failed to load OpenGL 2.0 symbols."};
			}

			break;

		case RendererKind::ogl_3_2_core:
			extension_manager_->probe(OglExtensionId::v3_2);

			if (!extension_manager_->has(OglExtensionId::v3_2))
			{
				throw Exception{"Failed to load OpenGL 3.2 core symbols."};
			}

			break;

		case RendererKind::ogl_es_2_0:
			extension_manager_->probe(OglExtensionId::es_v2_0);

			if (!extension_manager_->has(OglExtensionId::es_v2_0))
			{
				throw Exception{"Failed to load OpenGL ES 2.0 symbols."};
			}

			break;

		default:
			throw Exception{"Unsupported renderer kind."};
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
		kind_,
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

	set_name_and_description();


	// Present.
	//
	clear_buffers();
	present();
}

void OglRenderer::uninitialize()
{
	uninitialize_internal();
}

const RendererDeviceFeatures& OglRenderer::device_get_features() const noexcept
{
	return device_features_;
}

const RendererDeviceInfo& OglRenderer::device_get_info() const noexcept
{
	return device_info_;
}

void OglRenderer::window_set_mode(
	const RendererWindowSetModeParam& param)
{
	RendererUtils::window_set_mode(sdl_window_.get(), param);

	const auto size_changed = (screen_width_ != param.width_ || screen_height_ != param.height_);

	screen_width_ = param.width_;
	screen_height_ = param.height_;

	if (size_changed && ogl_device_features_.framebuffer_is_available_)
	{
		msaa_framebuffer_destroy();
		msaa_framebuffer_create();
	}
}

void OglRenderer::window_set_title(
	const std::string& title_utf8)
{
	RendererUtils::window_set_title(sdl_window_.get(), title_utf8);
}

void OglRenderer::window_show(
	const bool is_visible)
{
	RendererUtils::window_show(sdl_window_.get(), is_visible);
}

const glm::mat4& OglRenderer::csc_get_texture() const noexcept
{
	return detail::OglRendererUtils::csc_get_texture();
}

const glm::mat4& OglRenderer::csc_get_projection() const noexcept
{
	return detail::OglRendererUtils::csc_get_projection();
}

bool OglRenderer::vsync_get() const noexcept
{
	if (!device_features_.vsync_is_available_)
	{
		return false;
	}

	return OglRendererUtils::vsync_get();
}

void OglRenderer::vsync_set(
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

void OglRenderer::aa_set(
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

void OglRenderer::color_buffer_set_clear_color(
	const R8g8b8a8& color)
{
	OglRendererUtils::set_color_buffer_clear_color(color);
}

void OglRenderer::clear_buffers()
{
	framebuffers_bind();

	OglRendererUtils::clear_buffers();
}

void OglRenderer::present()
{
	framebuffers_blit();

	OglRendererUtils::swap_window(sdl_window_.get());
}

RendererIndexBufferPtr OglRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	return ogl_context_->buffer_get_manager()->index_buffer_create(param);
}

void OglRenderer::index_buffer_destroy(
	RendererIndexBufferPtr index_buffer)
{
	ogl_context_->buffer_get_manager()->buffer_destroy(index_buffer);
}

RendererVertexBufferPtr OglRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	return ogl_context_->buffer_get_manager()->vertex_buffer_create(param);
}

void OglRenderer::vertex_buffer_destroy(
	RendererVertexBufferPtr vertex_buffer)
{
	ogl_context_->buffer_get_manager()->buffer_destroy(vertex_buffer);
}

RendererVertexInputPtr OglRenderer::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	return ogl_context_->vertex_input_get_manager()->create(param);
}

void OglRenderer::vertex_input_destroy(
	RendererVertexInputPtr vertex_input)
{
	ogl_context_->vertex_input_get_manager()->destroy(vertex_input);
}

RendererShaderPtr OglRenderer::shader_create(
	const RendererShaderCreateParam& param)
{
	return ogl_context_->shader_get_manager()->create(param);
}

void OglRenderer::shader_destroy(
	const RendererShaderPtr shader)
{
	return ogl_context_->shader_get_manager()->destroy(shader);
}

RendererShaderStagePtr OglRenderer::shader_stage_create(
	const RendererShaderStageCreateParam& param)
{
	return ogl_context_->shader_stage_get_manager()->create(param);
}

void OglRenderer::shader_stage_destroy(
	const RendererShaderStagePtr shader_stage)
{
	return ogl_context_->shader_stage_get_manager()->destroy(shader_stage);
}

void OglRenderer::execute_commands(
	const RendererCommandManagerPtr command_manager)
{
	command_executor_->execute(command_manager);
}

RendererTexture2dPtr OglRenderer::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	return ogl_context_->texture_get_manager()->create(param);
}

void OglRenderer::texture_2d_destroy(
	RendererTexture2dPtr texture_2d)
{
	ogl_context_->texture_get_manager()->destroy(texture_2d);
}

RendererSamplerPtr OglRenderer::sampler_create(
	const RendererSamplerCreateParam& param)
{
	return ogl_context_->sampler_get_manager()->create(param);
}

void OglRenderer::sampler_destroy(
	RendererSamplerPtr sampler)
{
	ogl_context_->sampler_get_manager()->destroy(sampler);
}

void OglRenderer::uninitialize_internal()
{
	kind_ = {};
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
}

OglRenderer::RboResource OglRenderer::renderbuffer_create()
{
	if (!ogl_device_features_.framebuffer_is_available_)
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

void OglRenderer::renderbuffer_bind(
	const GLuint ogl_renderbuffer_name)
{
	const auto ogl_func = (ogl_device_features_.framebuffer_is_ext_ ? ::glBindRenderbufferEXT : ::glBindRenderbuffer);

	ogl_func(GL_RENDERBUFFER, ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());
}

OglRenderer::FboResource OglRenderer::framebuffer_create()
{
	if (!ogl_device_features_.framebuffer_is_available_)
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

void OglRenderer::framebuffer_bind(
	const GLenum ogl_target,
	const GLuint ogl_name)
{
	assert(ogl_device_features_.framebuffer_is_available_);

	const auto ogl_func = (ogl_device_features_.framebuffer_is_ext_ ? ::glBindFramebufferEXT : ::glBindFramebuffer);

	ogl_func(ogl_target, ogl_name);
	assert(!OglRendererUtils::was_errors());
}

void OglRenderer::framebuffer_blit(
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

	assert(ogl_device_features_.framebuffer_is_available_);

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

OglRenderer::RboResource OglRenderer::renderbuffer_create(
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

	assert(ogl_device_features_.framebuffer_is_available_);

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

void OglRenderer::msaa_color_rb_destroy()
{
	ogl_msaa_color_rb_.reset();
}

void OglRenderer::msaa_depth_rb_destroy()
{
	ogl_msaa_depth_rb_.reset();
}

void OglRenderer::msaa_fbo_destroy()
{
	ogl_msaa_fbo_.reset();
}

void OglRenderer::msaa_framebuffer_destroy()
{
	msaa_fbo_destroy();
	msaa_color_rb_destroy();
	msaa_depth_rb_destroy();
}

void OglRenderer::msaa_color_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	ogl_msaa_color_rb_ = renderbuffer_create(width, height, sample_count, GL_RGBA8);
}

void OglRenderer::msaa_depth_rb_create(
	const int width,
	const int height,
	const int sample_count)
{
	ogl_msaa_depth_rb_ = renderbuffer_create(width, height, sample_count, GL_DEPTH_COMPONENT);
}

void OglRenderer::msaa_framebuffer_create()
{
	msaa_color_rb_create(screen_width_, screen_height_, aa_value_);
	msaa_depth_rb_create(screen_width_, screen_height_, aa_value_);

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

void OglRenderer::framebuffers_destroy()
{
	msaa_framebuffer_destroy();
}

void OglRenderer::framebuffers_create()
{
	if (!ogl_device_features_.framebuffer_is_available_)
	{
		return;
	}

	const auto is_msaa = (aa_kind_ == RendererAaKind::ms && aa_value_ >= RendererLimits::aa_min);

	msaa_framebuffer_create();
}

void OglRenderer::framebuffers_blit()
{
	if (ogl_msaa_fbo_ == 0)
	{
		return;
	}

	// MSAA FBO -> Default FBO
	//

	// Read: MSAA
	// Draw: Default
	framebuffer_bind(GL_DRAW_FRAMEBUFFER, 0);

	framebuffer_blit(
		screen_width_,
		screen_height_,
		screen_width_,
		screen_height_,
		false
	);
}

void OglRenderer::framebuffers_bind()
{
	if (ogl_msaa_fbo_ == 0)
	{
		return;
	}

	framebuffer_bind(GL_FRAMEBUFFER, ogl_msaa_fbo_);
}

void OglRenderer::aa_disable()
{
	aa_kind_ = RendererAaKind::none;

	if (ogl_msaa_fbo_ == 0)
	{
		return;
	}

	msaa_framebuffer_destroy();
	static_cast<void>(msaa_framebuffer_create());
}

void OglRenderer::msaa_set(
	const int aa_value)
{
	if (device_features_.msaa_is_requires_restart_)
	{
		throw Exception{"Requires restart."};
	}

	if (!ogl_device_features_.framebuffer_is_available_)
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
// OglRenderer
// ==========================================================================


} // detail
} // bstone
