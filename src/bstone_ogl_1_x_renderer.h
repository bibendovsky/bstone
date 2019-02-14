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


	void set_2d_projection_matrix(
		const int width,
		const int height) override;


	ObjectId vertex_buffer_create(
		const int vertex_count) override;

	void vertex_buffer_destroy(
		ObjectId id) override;

	void vertex_buffer_update(
		ObjectId id,
		const int offset,
		const int count,
		const RendererVertex* const vertices) override;


private:
	using VertexBuffer = std::vector<RendererVertex>;
	using VertexBuffers = std::list<VertexBuffer>;


	bool is_initialized_;
	std::string error_message_;

	RendererPath probe_renderer_path_;

	SdlWindowPtr sdl_window_;
	SdlGlContext sdl_gl_context_;

	Mat4F two_d_projection_matrix_;

	VertexBuffers vertex_buffers_;


	bool probe_or_initialize(
		const bool is_probe,
		const RendererPath probe_renderer_path,
		const RendererInitializeParam& param);

	void uninitialize_internal(
		const bool is_dtor = false);
}; // OglRenderer


} // bstone


#endif // !BSTONE_OGL_1_X_RENDERER_INCLUDED
