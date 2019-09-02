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

#include "bstone_detail_ogl_buffer_manager.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler_manager.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_ogl_vertex_input.h"
#include "bstone_detail_ogl_vertex_input_manager.h"
#include "bstone_detail_ogl_shader_manager.h"
#include "bstone_detail_ogl_shader_stage_manager.h"
#include "bstone_detail_ogl_extension_manager.h"


namespace bstone
{
namespace detail
{


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
	ogl_context_{},
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
}

Ogl2XRenderer::~Ogl2XRenderer()
{
	uninitialize_internal(true);
}

const std::string& Ogl2XRenderer::get_error_message() const noexcept
{
	return error_message_;
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

bool Ogl2XRenderer::probe()
{
	uninitialize_internal();

	const auto result = probe_or_initialize(true, RendererCreateParam{});

	uninitialize_internal();

	return result;
}

const RendererProbe& Ogl2XRenderer::probe_get() const noexcept
{
	return probe_;
}

bool Ogl2XRenderer::is_initialized() const noexcept
{
	return is_initialized_;
}

bool Ogl2XRenderer::initialize(
	const RendererCreateParam& param)
{
	if (probe_.kind_ == RendererKind::none)
	{
		uninitialize_internal();

		const auto probe_result = probe_or_initialize(true, RendererCreateParam{});

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
	assert(is_initialized_);

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
			error_message_ = "Unsupported blit filter.";

			return false;
	}

	if (!device_features_.framebuffer_is_available_)
	{
		error_message_ = "Off-screen framebuffer not supported.";

		return false;
	}

	if (ogl_msaa_fbo_ == 0)
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
			error_message_ = "Invalid anti-aliasing kind.";

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
			case RendererCommandId::culling:
				command_execute_culling(*command_buffer->read_culling());
				break;

			case RendererCommandId::depth_set_test:
				command_execute_depth_test(*command_buffer->read_depth_test());
				break;

			case RendererCommandId::depth_set_write:
				command_execute_depth_write(*command_buffer->read_depth_write());
				break;

			case RendererCommandId::viewport:
				command_execute_viewport(*command_buffer->read_viewport());
				break;

			case RendererCommandId::scissor:
				command_execute_scissor(*command_buffer->read_scissor());
				break;

			case RendererCommandId::scissor_set_box:
				command_execute_scissor_box(*command_buffer->read_scissor_box());
				break;

			case RendererCommandId::blending:
				command_execute_blending(*command_buffer->read_blending());
				break;

			case RendererCommandId::blending_func:
				command_execute_blending_func(*command_buffer->read_blending_func());
				break;

			case RendererCommandId::texture:
				command_execute_texture(*command_buffer->read_texture());
				break;

			case RendererCommandId::sampler:
				command_execute_sampler(*command_buffer->read_sampler());
				break;

			case RendererCommandId::vertex_input:
				command_execute_vertex_input(*command_buffer->read_vertex_input());
				break;

			case RendererCommandId::shader_stage:
				command_execute_shader_stage(*command_buffer->read_shader_stage());
				break;

			case RendererCommandId::shader_var_int32:
				command_execute_shader_var_int32(*command_buffer->read_shader_var_int32());
				break;

			case RendererCommandId::shader_var_float32:
				command_execute_shader_var_float32(*command_buffer->read_shader_var_float32());
				break;

			case RendererCommandId::shader_var_vec2:
				command_execute_shader_var_vec2(*command_buffer->read_shader_var_vec2());
				break;

			case RendererCommandId::shader_var_vec4:
				command_execute_shader_var_vec4(*command_buffer->read_shader_var_vec4());
				break;

			case RendererCommandId::shader_var_mat4:
				command_execute_shader_var_mat4(*command_buffer->read_shader_var_mat4());
				break;

			case RendererCommandId::shader_var_sampler2d:
				command_execute_shader_var_sampler_2d(*command_buffer->read_shader_var_sampler_2d());
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
	const RendererCreateParam& param)
{
	if (is_probe)
	{
		OglRendererUtils::create_probe_window_and_context(sdl_window_, sdl_gl_context_);
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

		OglRendererUtils::create_window_and_context(window_param, sdl_window_, sdl_gl_context_);

		aa_kind_ = param.aa_kind_;
		aa_value_ = param.aa_value_;
	}

	OglRendererUtils::window_get_drawable_size(
		sdl_window_.get(),
		screen_width_,
		screen_height_);

	if (screen_width_ == 0 || screen_height_ == 0)
	{
		error_message_ = "Failed to get screen size.";

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

	extension_manager_->probe(OglExtensionId::v2_0);

	if (!extension_manager_->has(OglExtensionId::v2_0))
	{
		error_message_ = "Failed to load OpenGL 2.0 symbols.";

		return false;
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
		error_message_ = "No vertex input locations.";

		return false;
	}

	OglRendererUtils::vsync_probe(device_features_);

	OglRendererUtils::vertex_input_vao_probe(
		extension_manager_.get(),
		ogl_device_features_
	);

	if (!is_probe)
	{
		ogl_context_ = std::move(OglContextFactory::create(
			RendererKind::ogl_2_x,
			device_features_,
			ogl_device_features_
		));
	}

	if (!is_probe)
	{
		if (device_features_.vsync_is_available_)
		{
			static_cast<void>(OglRendererUtils::vsync_set(param.is_vsync_));
		}

		if (!framebuffers_create())
		{
			return false;
		}
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

		device_info_ = OglRendererUtils::device_info_get();


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

void Ogl2XRenderer::renderbuffer_destroy(
	GLuint& ogl_renderbuffer_name)
{
	if (ogl_renderbuffer_name == 0)
	{
		return;
	}

	assert(device_features_.framebuffer_is_available_);

	renderbuffer_bind(0);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto delete_renderbuffers = (is_arb ? ::glDeleteRenderbuffers : ::glDeleteRenderbuffersEXT);
	assert(delete_renderbuffers != nullptr);

	delete_renderbuffers(1, &ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());
	ogl_renderbuffer_name = 0;
}

bool Ogl2XRenderer::renderbuffer_create(
	GLuint& ogl_renderbuffer_name)
{
	assert(ogl_renderbuffer_name == 0);
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto gen_renderbuffers = (is_arb ? ::glGenRenderbuffers : ::glGenRenderbuffersEXT);
	assert(gen_renderbuffers != nullptr);

	gen_renderbuffers(1, &ogl_renderbuffer_name);
	assert(!OglRendererUtils::was_errors());

	return ogl_renderbuffer_name != 0;
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
	if (ogl_msaa_fbo_ == 0)
	{
		return;
	}

	assert(device_features_.framebuffer_is_available_);

	framebuffer_bind(GL_FRAMEBUFFER, 0);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto delete_framebuffers = (is_arb ? ::glDeleteFramebuffers : ::glDeleteFramebuffersEXT);
	assert(delete_framebuffers != nullptr);

	delete_framebuffers(1, &ogl_framebuffer_name);
	assert(!OglRendererUtils::was_errors());
	ogl_framebuffer_name = 0;
}

bool Ogl2XRenderer::framebuffer_create(
	GLuint& ogl_framebuffer_name)
{
	assert(ogl_framebuffer_name == 0);
	assert(device_features_.framebuffer_is_available_);

	const auto is_arb = ogl_device_features_.framebuffer_is_arb_;
	const auto gen_framebuffers = (is_arb ? ::glGenFramebuffers : ::glGenFramebuffersEXT);
	assert(gen_framebuffers != nullptr);

	gen_framebuffers(1, &ogl_framebuffer_name);
	assert(!OglRendererUtils::was_errors());

	return ogl_framebuffer_name != 0;
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
	assert(ogl_internal_format > 0);

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

	renderbuffer_bind(0);

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

	framebuffer_bind(GL_FRAMEBUFFER, 0);

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

	framebuffer_bind(GL_FRAMEBUFFER, 0);

	return true;
}

void Ogl2XRenderer::framebuffers_destroy()
{
	msaa_framebuffer_destroy();
	downscale_framebuffer_destroy();
}

bool Ogl2XRenderer::framebuffers_create()
{
	if (!device_features_.framebuffer_is_available_)
	{
		return true;
	}

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
	if (ogl_msaa_fbo_ == 0)
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
	if (ogl_msaa_fbo_ == 0)
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

void Ogl2XRenderer::texture_set(
	RendererTexture2dPtr new_texture_2d)
{
	ogl_context_->texture_get_manager()->set(new_texture_2d);
}

void Ogl2XRenderer::command_execute_culling(
	const RendererCommandCulling& command)
{
	ogl_context_->culling_enable(command.is_enable_);
}

void Ogl2XRenderer::command_execute_depth_test(
	const RendererCommandDepthTest& command)
{
	ogl_context_->depth_test_enable(command.is_enable_);
}

void Ogl2XRenderer::command_execute_depth_write(
	const RendererCommandDepthWrite& command)
{
	ogl_context_->depth_write_enable(command.is_enable_);
}

void Ogl2XRenderer::command_execute_viewport(
	const RendererCommandViewport& command)
{
	ogl_context_->viewport_set(command.viewport_);
}

void Ogl2XRenderer::command_execute_blending(
	const RendererCommandBlending& command)
{
	ogl_context_->blending_enable(command.is_enable_);
}

void Ogl2XRenderer::command_execute_blending_func(
	const RendererCommandBlendingFunc& command)
{
	ogl_context_->blending_set_func(command.blending_func_);
}

void Ogl2XRenderer::command_execute_scissor(
	const RendererCommandScissor& command)
{
	ogl_context_->scissor_enable(command.is_enable_);
}

void Ogl2XRenderer::command_execute_scissor_box(
	const RendererCommandScissorBox& command)
{
	ogl_context_->scissor_set_box(command.scissor_box_);
}

void Ogl2XRenderer::command_execute_texture(
	const RendererCommandTexture& command)
{
	texture_set(command.texture_2d_);
}

void Ogl2XRenderer::command_execute_sampler(
	const RendererCommandSampler& command)
{
	ogl_context_->sampler_get_manager()->set(command.sampler_);
}

void Ogl2XRenderer::command_execute_vertex_input(
	const RendererCommandVertexInput& command)
{
	auto vertex_input = static_cast<OglVertexInputPtr>(command.vertex_input_);

	ogl_context_->vertex_input_get_manager()->set(vertex_input);
}

void Ogl2XRenderer::command_execute_shader_stage(
	const RendererCommandShaderStage& command)
{
	if (!command.shader_stage_)
	{
		throw Exception{"Null shader stage."};
	}

	command.shader_stage_->set();
}

void Ogl2XRenderer::command_execute_shader_var_int32(
	const RendererCommandShaderVarInt32& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_var_float32(
	const RendererCommandShaderVarFloat32& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_var_vec2(
	const RendererCommandShaderVarVec2& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_var_vec4(
	const RendererCommandShaderVarVec4& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_var_mat4(
	const RendererCommandShaderVarMat4& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_shader_var_sampler_2d(
	const RendererCommandShaderVarSampler2d& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void Ogl2XRenderer::command_execute_draw_quads(
	const RendererCommandDrawQuads& command)
{
	assert(command.count_ > 0);
	assert(command.index_offset_ >= 0);

	const auto triangles_per_quad = 2;
	const auto triangle_count = command.count_ * triangles_per_quad;

	const auto indices_per_triangle = 3;
	const auto indices_per_quad = triangles_per_quad * indices_per_triangle;
	const auto index_count = indices_per_quad * command.count_;

	auto index_buffer = ogl_context_->vertex_input_get_manager()->get_current_index_buffer();

	if (!index_buffer)
	{
		throw Exception{"Null index buffer."};
	}

	const auto index_byte_depth = index_buffer->get_byte_depth();
	const auto max_index_count = index_buffer->get_size() / index_byte_depth;

	const auto index_byte_offset = command.index_offset_ * index_byte_depth;
	assert(command.index_offset_ < max_index_count);
	assert(command.count_ <= max_index_count);
	assert((command.index_offset_ + command.count_) <= max_index_count);


	// Draw the quads.
	//
	const auto index_buffer_data = reinterpret_cast<const void*>(static_cast<std::intptr_t>(index_byte_offset));

	const auto ogl_element_type = OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
		index_buffer->get_byte_depth());

	index_buffer->set(true);

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
