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


#ifndef BSTONE_OGL_1_X_RENDERER_INCLUDED
#define BSTONE_OGL_1_X_RENDERER_INCLUDED


#include <list>
#include <vector>
#include "bstone_ogl.h"
#include "bstone_ogl_renderer.h"
#include "bstone_ogl_renderer_utils.h"


namespace bstone
{


class Ogl1XRenderer :
	public OglXRenderer
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


	bool probe(
		const RendererPath renderer_path) override;

	RendererPath get_probe_path() const override;


	bool is_initialized() const override;

	bool initialize(
		const RendererInitializeParam& param) override;

	void uninitialize() override;


	RendererPath get_path() const override;


	void window_show(
		const bool is_visible) override;


	void color_buffer_set_clear_color(
		const RendererColor32& color) override;

	void set_2d_projection_matrix(
		const int width,
		const int height) override;


	RendererObjectId index_buffer_create(
		const int index_count) override;

	void index_buffer_destroy(
		RendererObjectId id) override;

	void index_buffer_update(
		RendererObjectId id,
		const int offset,
		const int count,
		const void* const indices) override;


	RendererObjectId vertex_buffer_create(
		const int vertex_count) override;

	void vertex_buffer_destroy(
		RendererObjectId id) override;

	void vertex_buffer_update(
		RendererObjectId id,
		const int offset,
		const int count,
		const RendererVertex* const vertices) override;


	RendererObjectId texture_2d_create(
		const RendererTextureCreateParam& param) override;

	void texture_2d_destroy(
		RendererObjectId texture_id) override;

	void texture_2d_update(
		RendererObjectId texture_id,
		const RendererTextureUpdateParam& param) override;


	void execute_command_sets(
		const RendererCommandSets& command_sets) override;


private:
	class IndexBuffer
	{
	public:
		using Data = std::vector<std::uint8_t>;


		int count_;
		int byte_depth_;
		int size_in_bytes_;
		GLenum data_type_;

		Data data_;
	}; // IndexBuffer

	using IndexBuffers = std::list<IndexBuffer>;


	using VertexBuffer = std::vector<RendererVertex>;
	using VertexBuffers = std::list<VertexBuffer>;

	using TextureBuffer = std::vector<RendererColor32>;

	class Texture2d
	{
	public:
		int width_;
		int height_;

		int actual_width_;
		int actual_height_;

		float actual_u_;
		float actual_v_;

		const std::uint8_t* indexed_pixels_;
		const bool* indexed_alphas_;

		GLuint ogl_id_;
	}; // Texture2d

	using Textures2d = std::list<Texture2d>;


	bool is_initialized_;
	std::string error_message_;

	RendererPath probe_renderer_path_;

	SdlWindowPtr sdl_window_;
	SdlGlContext sdl_gl_context_;

	RendererPalette palette_;

	Mat4F two_d_projection_matrix_;

	IndexBuffers index_buffers_;
	VertexBuffers vertex_buffers_;

	TextureBuffer texture_buffer_;
	Textures2d textures_2d_;


	bool probe_or_initialize(
		const bool is_probe,
		const RendererPath probe_renderer_path,
		const RendererInitializeParam& param);

	void uninitialize_internal(
		const bool is_dtor = false);

	void update_indexed_texture(
		const int mipmap_level,
		const Texture2d& texture_2d);

	void update_indexed_textures();


	void execute_command_set_2d();

	void execute_command_update_palette(
		const RendererCommand::UpdatePalette& command);

	void execute_command_draw_quads(
		const RendererCommand::DrawQuads& command);
}; // OglRenderer


} // bstone


#endif // !BSTONE_OGL_1_X_RENDERER_INCLUDED
