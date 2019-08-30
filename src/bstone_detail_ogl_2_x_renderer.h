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
#include "bstone_ogl_unique_resources.h"
#include "bstone_detail_ogl_extension_manager.h"
#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_shader_stage.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_index_buffer.h"
#include "bstone_detail_ogl_vertex_buffer.h"
#include "bstone_detail_ogl_context.h"


namespace bstone
{
namespace detail
{


class Ogl2XRenderer;
using Ogl2XRendererPtr = Ogl2XRenderer*;


class Ogl2XRenderer :
	public Renderer
{
public:
	Ogl2XRenderer();

	~Ogl2XRenderer() override;


	const std::string& get_error_message() const override;


	RendererKind get_kind() const override;

	const std::string& get_name() const override;

	const std::string& get_description() const override;


	bool probe() override;

	const RendererProbe& probe_get() const override;


	bool is_initialized() const override;

	bool initialize(
		const RendererInitializeParam& param) override;

	void uninitialize() override;


	const RendererDeviceFeatures& device_get_features() const override;

	const RendererDeviceInfo& device_get_info() const override;

	bool device_is_lost() const override;

	bool device_is_ready_to_reset() const override;

	void device_reset() override;


	void window_show(
		const bool is_visible) override;


	const glm::mat4& csc_get_texture() const override;

	const glm::mat4& csc_get_projection() const override;


	bool vsync_get() const override;

	bool vsync_set(
		const bool is_enabled) override;


	bool downscale_set(
		const int width,
		const int height,
		const RendererFilterKind blit_filter) override;


	bool aa_set(
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
		const RendererShader::CreateParam& param) override;

	void shader_destroy(
		const RendererShaderPtr shader) override;


	RendererShaderStagePtr shader_stage_create(
		const RendererShaderStage::CreateParam& param) override;

	void shader_stage_destroy(
		const RendererShaderStagePtr shader) override;


	void execute_commands(
		const RendererCommandManagerPtr command_manager) override;


private:
	using Shaders = std::list<detail::OglShaderUPtr>;
	using ShaderStages = std::list<detail::OglShaderStageUPtr>;


	bool is_initialized_;
	std::string error_message_;

	RendererProbe probe_;

	SdlWindowUPtr sdl_window_;
	SdlGlContextUPtr sdl_gl_context_;

	OglExtensionManagerUPtr extension_manager_;
	OglContextUPtr ogl_context_;

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

	GLuint ogl_msaa_fbo_;
	GLuint ogl_msaa_color_rb_;
	GLuint ogl_msaa_depth_rb_;

	GLuint ogl_downscale_fbo_;
	GLuint ogl_downscale_color_rb_;

	int viewport_x_;
	int viewport_y_;
	int viewport_width_;
	int viewport_height_;
	float viewport_min_depth_;
	float viewport_max_depth_;

	bool scissor_is_enabled_;
	int scissor_x_;
	int scissor_y_;
	int scissor_width_;
	int scissor_height_;

	bool culling_is_enabled_;
	RendererCullingFace culling_face_;
	RendererCullingMode culling_mode_;

	bool depth_is_test_enabled_;
	bool depth_is_write_enabled_;

	bool blending_is_enabled_;
	RendererBlendingFactor blending_src_factor_;
	RendererBlendingFactor blending_dst_factor_;


	bool probe_or_initialize(
		const bool is_probe,
		const RendererInitializeParam& param);


	void uninitialize_internal(
		const bool is_dtor = false);


	void renderbuffer_destroy(
		GLuint& ogl_renderbuffer_name);

	bool renderbuffer_create(
		GLuint& ogl_renderbuffer_name);

	bool renderbuffer_create(
		const int width,
		const int height,
		const int sample_count,
		const GLenum ogl_internal_format,
		GLuint& ogl_rb_name);

	void renderbuffer_bind(
		const GLuint ogl_renderbuffer_name);


	void framebuffer_destroy(
		GLuint& ogl_framebuffer_name);

	bool framebuffer_create(
		GLuint& ogl_framebuffer_name);

	void framebuffer_bind(
		const GLenum ogl_target,
		const GLuint ogl_framebuffer_name);

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

	bool msaa_color_rb_create(
		const int width,
		const int height,
		const int sample_count);

	bool msaa_depth_rb_create(
		const int width,
		const int height,
		const int sample_count);

	bool msaa_framebuffer_create();


	void downscale_color_rb_destroy();

	void downscale_fbo_destroy();

	void downscale_framebuffer_destroy();

	bool downscale_color_rb_create(
		const int width,
		const int height);

	bool downscale_framebuffer_create();


	void framebuffers_destroy();

	bool framebuffers_create();

	void framebuffers_blit();

	void framebuffers_bind();


	void aa_disable();

	bool msaa_set(
		const int aa_value);


	void viewport_set_rectangle();

	void viewport_set_depth_range();

	void viewport_set_defaults();


	void scissor_enable();

	void scissor_set_box();

	void scissor_set_defaults();


	void culling_enabled();

	void culling_set_face();

	void culling_set_mode();

	void culling_set_defaults();


	void depth_set_test();

	void depth_set_write();

	void depth_set_defaults();


	void blending_enable();

	void blending_set_function();

	void blending_set_defaults();


	void texture_set(
		RendererTexture2dPtr new_texture_2d);


	void command_execute_culling(
		const RendererCommandCulling& command);

	void command_execute_depth_test(
		const RendererCommandDepthTest& command);

	void command_execute_depth_write(
		const RendererCommandDepthWrite& command);

	void command_execute_blending(
		const RendererCommandBlending& command);

	void command_execute_blending_function(
		const RendererCommandBlendingFunction& command);

	void command_execute_viewport(
		const RendererCommandViewport& command);

	void command_execute_scissor(
		const RendererCommandScissor& command);

	void command_execute_scissor_box(
		const RendererCommandScissorBox& command);

	void command_execute_texture(
		const RendererCommandTexture& command);

	void command_execute_sampler(
		const RendererCommandSampler& command);

	void command_execute_vertex_input(
		const RendererCommandVertexInput& command);

	void command_execute_shader_stage(
		const RendererCommandShaderStage& command);

	void command_execute_shader_variable_int32(
		const RendererCommandShaderVariableInt32& command);

	void command_execute_shader_variable_float32(
		const RendererCommandShaderVariableFloat32& command);

	void command_execute_shader_variable_vec2(
		const RendererCommandShaderVariableVec2& command);

	void command_execute_shader_variable_vec4(
		const RendererCommandShaderVariableVec4& command);

	void command_execute_shader_variable_mat4(
		const RendererCommandShaderVariableMat4& command);

	void command_execute_shader_variable_sampler_2d(
		const RendererCommandShaderVariableSampler2d& command);

	void command_execute_draw_quads(
		const RendererCommandDrawQuads& command);
}; // OglRenderer


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_2_X_RENDERER_INCLUDED
