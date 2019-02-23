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


class Ogl1XRenderer;
using Ogl1XRendererPtr = Ogl1XRenderer*;


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


	void execute_command_sets(
		const RendererCommandSets& command_sets) override;


private:
	// =========================================================================
	// IndexBuffer
	//

	class IndexBuffer :
		public RendererIndexBuffer
	{
	public:
		using Data = std::vector<std::uint8_t>;


		std::string error_message_;

		int count_;
		int byte_depth_;
		int size_in_bytes_;
		GLenum data_type_;

		Data data_;


		IndexBuffer() = default;

		IndexBuffer(
			const IndexBuffer& rhs) = delete;

		~IndexBuffer() override = default;


		void update(
			const RendererIndexBufferUpdateParam& param) override;


		bool initialize(
			const RendererIndexBufferCreateParam& param);
	}; // IndexBuffer

	using IndexBufferPtr = IndexBuffer*;
	using IndexBufferUPtr = std::unique_ptr<IndexBuffer>;

	using IndexBuffers = std::list<IndexBufferUPtr>;

	//
	// IndexBuffer
	// =========================================================================


	// =========================================================================
	// VertexBuffer
	//

	class VertexBuffer :
		public RendererVertexBuffer
	{
	public:
		using Data = std::vector<RendererVertex>;


		std::string error_message_;

		int count_;
		Data data_;


		VertexBuffer() = default;

		VertexBuffer(
			const VertexBuffer& rhs) = delete;

		~VertexBuffer() override = default;


		void update(
			const RendererVertexBufferUpdateParam& param) override;


		bool initialize(
			const RendererVertexBufferCreateParam& param);
	}; // VertexBuffer

	using VertexBufferPtr = VertexBuffer*;
	using VertexBufferUPtr = std::unique_ptr<VertexBuffer>;

	using VertexBuffers = std::list<VertexBufferUPtr>;

	//
	// VertexBuffer
	// =========================================================================


	// =========================================================================
	// Texture2d
	//

	using TextureBuffer = std::vector<RendererColor32>;
	using TextureBufferPtr = TextureBuffer*;

	class Texture2d :
		public RendererTexture2d
	{
	public:
		Ogl1XRendererPtr renderer_;
		std::string error_message_;

		bool is_npot_;

		int width_;
		int height_;

		int actual_width_;
		int actual_height_;

		const std::uint8_t* indexed_pixels_;
		const bool* indexed_alphas_;

		GLuint ogl_id_;


		Texture2d(
			Ogl1XRendererPtr renderer);

		Texture2d(
			const Texture2d& rhs) = delete;

		~Texture2d() override;


		void update(
			const RendererTexture2dUpdateParam& param) override;


		bool initialize(
			const RendererTexture2dCreateParam& param);

		void uninitialize_internal();

		void update_internal(
			const int mipmap_level);
	}; // Texture2d

	using Texture2dPtr = Texture2d*;
	using Texture2dUPtr = std::unique_ptr<Texture2d>;

	using Textures2d = std::list<Texture2dUPtr>;

	//
	// Texture2d
	// =========================================================================


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

	void update_indexed_textures();


	void execute_command_set_2d();

	void execute_command_update_palette(
		const RendererCommand::UpdatePalette& command);

	void execute_command_draw_quads(
		const RendererCommand::DrawQuads& command);
}; // OglRenderer


} // bstone


#endif // !BSTONE_OGL_1_X_RENDERER_INCLUDED
