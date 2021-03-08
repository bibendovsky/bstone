/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// OpenGL 3D renderer (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_INCLUDED


#include <list>

#include "bstone_ren_3d.h"
#include "bstone_ren_3d_cmd_buffer.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_buffer.h"
#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_extension_mgr.h"
#include "bstone_detail_ren_3d_gl_shader.h"
#include "bstone_detail_ren_3d_gl_shader_stage.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{
namespace detail
{


class Ren3dGl final :
	public Ren3d
{
public:
	Ren3dGl(
		const Ren3dCreateParam& param);

	~Ren3dGl() override;


	Ren3dKind get_kind() const noexcept override;

	const std::string& get_name() const noexcept override;

	const std::string& get_description() const noexcept override;


	const Ren3dDeviceFeatures& get_device_features() const noexcept override;

	const Ren3dDeviceInfo& get_device_info() const noexcept override;


	void set_window_mode(
		const Ren3dSetWindowModeParam& param) override;

	void set_window_title(
		const std::string& title_utf8) override;

	void show_window(
		const bool is_visible) override;


	bool get_vsync() const noexcept override;

	void enable_vsync(
		const bool is_enabled) override;


	void set_anti_aliasing(
		const Ren3dAaKind aa_kind,
		const int aa_value) override;


	void read_pixels_rgb_888(
		void* buffer,
		bool& is_flipped_vertically) override;


	void present() override;


	Ren3dBufferUPtr create_buffer(
		const Ren3dCreateBufferParam& param) override;

	Ren3dTexture2dUPtr create_texture_2d(
		const Ren3dCreateTexture2dParam& param) override;

	Ren3dSamplerUPtr create_sampler(
		const Ren3dCreateSamplerParam& param) override;

	Ren3dVertexInputUPtr create_vertex_input(
		const Ren3dCreateVertexInputParam& param) override;

	Ren3dShaderUPtr create_shader(
		const Ren3dCreateShaderParam& param) override;

	Ren3dShaderStageUPtr create_shader_stage(
		const Ren3dCreateShaderStageParam& param) override;

	void submit_commands(
		Ren3dCmdBufferPtr* const command_buffers,
		const int command_buffer_count) override;


private:
	static void fbo_deleter(
		const GLuint& gl_name) noexcept;

	using FboResource = UniqueResource<GLuint, fbo_deleter>;


	static void rbo_deleter(
		const GLuint& gl_name) noexcept;

	using RboResource = UniqueResource<GLuint, rbo_deleter>;


	using Shaders = std::list<Ren3dGlShaderUPtr>;
	using ShaderStages = std::list<Ren3dGlShaderStageUPtr>;


	Ren3dKind kind_;
	std::string name_;
	std::string description_;

	Ren3dDeviceInfo device_info_;
	Ren3dDeviceFeatures device_features_;
	Ren3dGlDeviceFeatures gl_device_features_;

	int screen_width_;
	int screen_height_;

	Ren3dAaKind aa_kind_;
	int aa_value_;

	SdlWindowUPtr sdl_window_;
	SdlGlContextUPtr sdl_gl_context_;

	Ren3dGlExtensionMgrUPtr extension_manager_;
	Ren3dGlContextUPtr context_;

	FboResource msaa_fbo_;
	RboResource msaa_color_rb_;
	RboResource msaa_depth_rb_;


	void set_name_and_description();


	RboResource create_renderbuffer();

	RboResource create_renderbuffer(
		const int width,
		const int height,
		const int sample_count,
		const GLenum gl_internal_format);

	void bind_renderbuffer(
		const GLuint gl_name);


	FboResource create_framebuffer();

	void bind_framebuffer(
		const GLenum gl_target,
		const GLuint gl_name);

	void blit_framebuffer(
		const int src_width,
		const int src_height,
		const int dst_width,
		const int dst_height,
		const bool is_linear_filter);


	void destroy_msaa_color_rb();

	void destroy_msaa_depth_rb();

	void destroy_msaa_fbo();

	void destroy_msaa_framebuffer();

	void create_msaa_color_rb(
		const int width,
		const int height,
		const int sample_count);

	void create_msaa_depth_rb(
		const int width,
		const int height,
		const int sample_count);

	void create_msaa_framebuffer();


	void destroy_framebuffers();

	void create_framebuffers();

	void blit_framebuffers();

	void bind_framebuffers();

	void bind_framebuffers_for_read_pixels();


	void disable_aa();

	void set_msaa(
		const int aa_value);


	void submit_clear(
		const Ren3dClearCmd& command);

	void submit_culling(
		const Ren3dEnableCullingCmd& command);

	void submit_enable_depth_test(
		const Ren3dEnableDepthTestCmd& command);

	void submit_enable_depth_write(
		const Ren3dEnableDepthWriteCmd& command);

	void submit_enable_blending(
		const Ren3dEnableBlendingCmd& command);

	void submit_set_blending_func(
		const Ren3dSetBlendingFuncCmd& command);

	void submit_set_viewport(
		const Ren3dSetViewportCmd& command);

	void submit_enable_scissor(
		const Ren3dEnableScissorCmd& command);

	void submit_set_scissor_box(
		const Ren3dSetScissorBoxCmd& command);

	void submit_set_texture(
		const Ren3dSetTextureCmd& command);

	void submit_set_sampler(
		const Ren3dSetSamplerCmd& command);

	void submit_set_vertex_input(
		const Ren3dSetVertexInputCmd& command);

	void submit_set_shader_stage(
		const Ren3dSetShaderStageCmd& command);

	void submit_set_int32_uniform(
		const Ren3dSetInt32UniformCmd& command);

	void submit_set_float32_uniform(
		const Ren3dSetFloat32UniformCmd& command);

	void submit_set_vec2_uniform(
		const Ren3dSetVec2UniformCmd& command);

	void submit_set_vec4_uniform(
		const Ren3dSetVec4UniformCmd& command);

	void submit_set_mat4_uniform(
		const Ren3dSetMat4UniformCmd& command);

	void submit_set_sampler_2d_uniform(
		const Ren3dSetSampler2dUniformCmd& command);

	void submit_draw_indexed(
		const Ren3dDrawIndexedCmd& command);
}; // Ren3dGl


using Ren3dGlUPtr = std::unique_ptr<Ren3dGl>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_INCLUDED
