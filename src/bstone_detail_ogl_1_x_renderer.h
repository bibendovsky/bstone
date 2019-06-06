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


#ifndef BSTONE_DETAIL_OGL_1_X_RENDERER_INCLUDED
#define BSTONE_DETAIL_OGL_1_X_RENDERER_INCLUDED


#include <array>
#include <list>
#include <vector>
#include "bstone_detail_ogl_extension_manager.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_ogl.h"
#include "bstone_renderer.h"
#include "bstone_renderer_sw_index_buffer.h"
#include "bstone_renderer_sw_vertex_buffer.h"


namespace bstone
{
namespace detail
{


class Ogl1XRenderer;
using Ogl1XRendererPtr = Ogl1XRenderer*;


class Ogl1XRenderer :
	public Renderer
{
public:
	Ogl1XRenderer();

	Ogl1XRenderer(
		Ogl1XRenderer&& rhs);

	~Ogl1XRenderer() override;


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


	RendererPath get_path() const override;


	const RendererDeviceFeatures& device_get_features() const override;

	bool device_is_lost() const override;

	bool device_is_ready_to_reset() const override;

	void device_reset() override;


	void window_show(
		const bool is_visible) override;


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


	void execute_commands(
		const RendererCommandManagerPtr command_manager) override;


private:
	using IndexBuffers = std::list<RendererSwIndexBufferUPtr>;
	using VertexBuffers = std::list<RendererSwVertexBufferUPtr>;


	// =========================================================================
	// Texture2d
	//

	class Texture2d :
		public RendererTexture2d
	{
	public:
		Ogl1XRendererPtr renderer_;
		std::string error_message_;

		RendererPixelFormat storage_pixel_format_;

		int width_;
		int height_;

		int mipmap_count_;

		RendererSamplerState sampler_state_;

		GLuint ogl_id_;


		Texture2d(
			Ogl1XRendererPtr renderer);

		Texture2d(
			const Texture2d& rhs) = delete;

		~Texture2d() override;


		void update(
			const RendererTexture2dUpdateParam& param) override;

		void generate_mipmaps() override;


		bool initialize(
			const RendererTexture2dCreateParam& param);

		void uninitialize_internal();

		void upload_mipmap(
			const int mipmap_level,
			const int width,
			const int height,
			const R8g8b8a8CPtr src_pixels);


		void set_mag_filter();

		void set_min_filter();

		void set_address_mode(
			const RendererAddressMode address_mode);

		void set_address_mode_u();

		void set_address_mode_v();

		void set_anisotropy();

		void update_sampler_state(
			const RendererSamplerState& new_sampler_state);

		void set_sampler_state_defaults();
	}; // Texture2d

	using Texture2dPtr = Texture2d*;
	using Texture2dUPtr = std::unique_ptr<Texture2d>;

	using Textures2d = std::list<Texture2dUPtr>;

	//
	// Texture2d
	// =========================================================================

	// =========================================================================
	// Sampler
	//

	class Sampler final :
		public RendererSampler
	{
	public:
		Ogl1XRendererPtr renderer_;

		RendererSamplerState state_;


		Sampler(
			Ogl1XRendererPtr renderer);

		Sampler(
			const Sampler& rhs) = delete;

		~Sampler() override;


		void update(
			const RendererSamplerUpdateParam& param) override;


		bool initialize(
			const RendererSamplerCreateParam& param);
	}; // Sampler

	using SamplerPtr = Sampler*;
	using SamplerUPtr = std::unique_ptr<Sampler>;

	using Samplers = std::list<SamplerUPtr>;

	//
	// Sampler
	// =========================================================================

	// =========================================================================
	// VertexInput
	//

	class VertexInput final :
		public RendererVertexInput
	{
	public:
		Ogl1XRendererPtr renderer_;
		std::string error_message_;

		RendererIndexBufferPtr index_buffer_;
		RendererVertexAttributeDescriptions attribute_descriptions_;


		VertexInput(
			Ogl1XRendererPtr renderer);

		VertexInput(
			const VertexInput& rhs) = delete;

		~VertexInput() override;


		bool initialize(
			const RendererVertexInputCreateParam& param);
	}; // VertexInput

	using VertexInputPtr = VertexInput*;
	using VertexInputUPtr = std::unique_ptr<VertexInput>;

	using VertexInputs = std::list<VertexInputUPtr>;

	//
	// VertexInput
	// =========================================================================


	bool is_initialized_;
	std::string error_message_;

	RendererProbe probe_;

	SdlWindowUPtr sdl_window_;
	SdlGlContextUPtr sdl_gl_context_;

	OglExtensionManagerUPtr extension_manager_;

	RendererDeviceFeatures device_features_;
	detail::OglRendererUtilsDeviceFeatures ogl_device_features_;

	int screen_width_;
	int screen_height_;

	RendererAaKind aa_kind_;
	int aa_value_;

	GLuint ogl_internal_fbo_;
	GLuint ogl_internal_color_rb_;
	GLuint ogl_internal_depth_stencil_rb_;

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

	bool texture_2d_is_enabled_;

	bool fog_is_enabled_;
	glm::vec4 fog_color_;
	float fog_start_distance_;
	float fog_end_distance_;

	glm::mat4 matrix_model_;
	glm::mat4 matrix_view_;
	glm::mat4 matrix_model_view_;
	glm::mat4 matrix_projection_;

	glm::mat4 matrix_texture_;

	IndexBuffers index_buffers_;
	VertexBuffers vertex_buffers_;

	RendererUtils::TextureBuffer texture_buffer_;

	Textures2d textures_2d_;
	Texture2dPtr texture_2d_current_;

	Samplers samplers_;
	SamplerPtr sampler_current_;
	SamplerUPtr sampler_default_;

	VertexInputs vertex_inputs_;
	VertexInputPtr vertex_input_current_;
	bool vertex_input_is_position_enabled_;
	bool vertex_input_is_color_enabled_;
	bool vertex_input_is_texture_coordinates_enabled_;


	bool probe_or_initialize(
		const bool is_probe,
		const RendererInitializeParam& param);


	bool create_default_sampler();

	void uninitialize_internal(
		const bool is_dtor = false);


	void destroy_renderbuffer(
		GLuint& ogl_renderbuffer_name);

	bool create_renderbuffer(
		GLuint& ogl_renderbuffer_name);

	void bind_renderbuffer(
		const GLenum ogl_target,
		const GLuint ogl_renderbuffer_name);


	void destroy_framebuffer(
		GLuint& ogl_framebuffer_name);

	bool create_framebuffer(
		GLuint& ogl_framebuffer_name);

	void bind_framebuffer(
		const GLenum ogl_target,
		const GLuint ogl_framebuffer_name);


	void destroy_internal_color_rb();

	void destroy_internal_depth_stencil_rb();

	void destroy_internal_fbo();

	void destroy_internal_framebuffer();

	bool create_internal_color_rb(
		const int width,
		const int height);

	bool create_internal_depth_stencil_rb(
		const int width,
		const int height);

	bool create_internal_framebuffer();

	void blit_internal_framebuffer();


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


	void texture_2d_enable();

	void texture_set();

	void texture_set(
		Texture2dPtr new_texture_2d);

	void texture_mipmap_generation_set_hint();

	void texture_2d_set_defaults();


	void fog_enable();

	void fog_set_mode();

	void fog_set_color();

	void fog_set_distances();

	void fog_set_hint();

	void fog_set_defaults();


	void matrix_set_model();

	void matrix_set_view();

	void matrix_set_model_view();

	void matrix_set_projection();

	void matrix_set_texture();

	void matrix_set_defaults();


	void sampler_set();


	void vertex_input_enable_client_state(
		const bool is_enabled,
		const GLenum state);

	void vertex_input_enable_position();

	void vertex_input_enable_color();

	void vertex_input_enable_texture_coordinates();

	void vertex_input_enable_position(
		const bool is_enabled);

	void vertex_input_enable_color(
		const bool is_enabled);

	void vertex_input_enable_texture_coordinates(
		const bool is_enabled);

	void vertex_input_assign();

	void vertex_input_defaults();


	bool initialize_internal_framebuffer();


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

	void command_execute_fog(
		const RendererCommandFog& command);

	void command_execute_fog_color(
		const RendererCommandFogColor& command);

	void command_execute_fog_distances(
		const RendererCommandFogDistances& command);

	void command_execute_matrix_model(
		const RendererCommandMatrixModel& command);

	void command_execute_matrix_view(
		const RendererCommandMatrixView& command);

	void command_execute_matrix_model_view(
		const RendererCommandMatrixModelView& command);

	void command_execute_matrix_projection(
		const RendererCommandMatrixProjection& command);

	void command_execute_texture(
		const RendererCommandTexture& command);

	void command_execute_sampler(
		const RendererCommandSampler& command);

	void command_execute_vertex_input(
		const RendererCommandVertexInput& command);

	void command_execute_draw_quads(
		const RendererCommandDrawQuads& command);
}; // OglRenderer


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_1_X_RENDERER_INCLUDED
