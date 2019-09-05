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


#ifndef BSTONE_DETAIL_OGL_2_X_RENDERER_INCLUDED
#define BSTONE_DETAIL_OGL_2_X_RENDERER_INCLUDED


#include <array>
#include <list>
#include <vector>

#include "bstone_unique_resource.h"

#include "bstone_detail_ogl_extension_manager.h"
#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_index_buffer.h"
#include "bstone_detail_ogl_vertex_buffer.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_command_executor.h"


namespace bstone
{
namespace detail
{


class Ogl2XRenderer :
	public Renderer
{
public:
	Ogl2XRenderer(
		const RendererCreateParam& param);

	~Ogl2XRenderer() override;


	RendererKind get_kind() const noexcept override;

	const std::string& get_name() const noexcept override;

	const std::string& get_description() const noexcept override;


	const RendererDeviceFeatures& device_get_features() const noexcept override;

	const RendererDeviceInfo& device_get_info() const noexcept override;

	bool device_is_lost() const noexcept override;

	bool device_is_ready_to_reset() const noexcept override;

	void device_reset() override;


	void window_show(
		const bool is_visible) override;


	const glm::mat4& csc_get_texture() const noexcept override;

	const glm::mat4& csc_get_projection() const noexcept override;


	bool vsync_get() const noexcept override;

	void vsync_set(
		const bool is_enabled) override;


	void downscale_set(
		const int width,
		const int height,
		const RendererFilterKind blit_filter) override;


	void aa_set(
		const RendererAaKind aa_kind,
		const int aa_value) override;


	void color_buffer_set_clear_color(
		const R8g8b8a8& color) override;

	void clear_buffers() override;

	void present() override;


	RendererIndexBufferPtr index_buffer_create(
		const RendererIndexBufferCreateParam& param) override;

	void index_buffer_destroy(
		RendererIndexBufferPtr index_buffer) override;


	RendererVertexBufferPtr vertex_buffer_create(
		const RendererVertexBufferCreateParam& param) override;

	void vertex_buffer_destroy(
		RendererVertexBufferPtr vertex_buffer) override;


	RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) override;

	void texture_2d_destroy(
		RendererTexture2dPtr texture_2d) override;


	RendererSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) override;

	void sampler_destroy(
		RendererSamplerPtr sampler) override;


	RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) override;

	void vertex_input_destroy(
		RendererVertexInputPtr vertex_input) override;


	RendererShaderPtr shader_create(
		const RendererShaderCreateParam& param) override;

	void shader_destroy(
		const RendererShaderPtr shader) override;


	RendererShaderStagePtr shader_stage_create(
		const RendererShaderStageCreateParam& param) override;

	void shader_stage_destroy(
		const RendererShaderStagePtr shader) override;


	void execute_commands(
		const RendererCommandManagerPtr command_manager) override;


private:
	static void fbo_resource_deleter(
		const GLuint& ogl_name) noexcept;

	using FboResource = UniqueResource<GLuint, fbo_resource_deleter>;


	static void rbo_resource_deleter(
		const GLuint& ogl_name) noexcept;

	using RboResource = UniqueResource<GLuint, rbo_resource_deleter>;


	using Shaders = std::list<detail::OglShaderUPtr>;
	using ShaderStages = std::list<detail::OglShaderStageUPtr>;


	SdlWindowUPtr sdl_window_;
	SdlGlContextUPtr sdl_gl_context_;

	OglExtensionManagerUPtr extension_manager_;
	OglContextUPtr ogl_context_;
	OglCommandExecutorUPtr command_executor_;

	RendererDeviceInfo device_info_;
	RendererDeviceFeatures device_features_;
	detail::OglDeviceFeatures ogl_device_features_;

	int screen_width_;
	int screen_height_;

	int downscale_width_;
	int downscale_height_;
	RendererFilterKind downscale_blit_filter_;

	RendererAaKind aa_kind_;
	int aa_value_;

	FboResource ogl_msaa_fbo_;
	RboResource ogl_msaa_color_rb_;
	RboResource ogl_msaa_depth_rb_;

	FboResource ogl_downscale_fbo_;
	RboResource ogl_downscale_color_rb_;


	void initialize(
		const RendererCreateParam& param);

	void uninitialize();


	void uninitialize_internal(
		const bool is_dtor = false);


	RboResource renderbuffer_create();

	RboResource renderbuffer_create(
		const int width,
		const int height,
		const int sample_count,
		const GLenum ogl_internal_format);

	void renderbuffer_bind(
		const GLuint ogl_name);


	FboResource framebuffer_create();

	void framebuffer_bind(
		const GLenum ogl_target,
		const GLuint ogl_name);

	void framebuffer_blit(
		const int src_width,
		const int src_height,
		const int dst_width,
		const int dst_height,
		const bool is_linear_filter);


	void msaa_color_rb_destroy();

	void msaa_depth_rb_destroy();

	void msaa_fbo_destroy();

	void msaa_framebuffer_destroy();

	void msaa_color_rb_create(
		const int width,
		const int height,
		const int sample_count);

	void msaa_depth_rb_create(
		const int width,
		const int height,
		const int sample_count);

	void msaa_framebuffer_create();


	void downscale_color_rb_destroy();

	void downscale_fbo_destroy();

	void downscale_framebuffer_destroy();

	void downscale_color_rb_create(
		const int width,
		const int height);

	void downscale_framebuffer_create();


	void framebuffers_destroy();

	void framebuffers_create();

	void framebuffers_blit();

	void framebuffers_bind();


	void aa_disable();

	void msaa_set(
		const int aa_value);
}; // Ogl2XRenderer


using Ogl2XRendererPtr = Ogl2XRenderer*;
using Ogl2XRendererUPtr = std::unique_ptr<Ogl2XRenderer>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_2_X_RENDERER_INCLUDED
