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
// OpenGL buffer.
//


#ifndef BSTONE_RENDERER_OGL_BUFFER_INCLUDED
#define BSTONE_RENDERER_OGL_BUFFER_INCLUDED


#include "bstone_renderer.h"
#include "bstone_ogl_raii.h"


namespace bstone
{


// =========================================================================
// RendererOglBuffer
//

class RendererOglBuffer;
using RendererOglBufferPtr = RendererOglBuffer*;


class RendererOglBuffer final
{
public:
	struct InitializeParam
	{
		RendererBufferKind kind_;
		RendererBufferUsageKind usage_kind_;
		int size_;
	}; // InitializeParam

	struct UpdateParam
	{
		int offset_;
		int size_;
		const void* data_;
	}; // UpdateParam


	RendererOglBuffer();

	RendererOglBuffer(
		const RendererOglBuffer& rhs) = delete;

	~RendererOglBuffer();


	RendererBufferKind get_kind() const noexcept;

	RendererBufferUsageKind get_usage_kind() const noexcept;

	int get_size() const noexcept;

	void bind();

	void unbind_target();

	void update(
		const UpdateParam& param);


	const std::string& get_error_message() const;

	bool initialize(
		const InitializeParam& param);

	bool is_initialized() const noexcept;


	static void reset_bindings();


private:
	std::string error_message_;

	RendererBufferKind kind_;
	RendererBufferUsageKind usage_kind_;
	int size_;
	OglBufferRaii ogl_name_raii_;
	GLenum ogl_target_;
	RendererOglBufferPtr* binding_;


	static RendererOglBufferPtr index_binding_;
	static RendererOglBufferPtr vertex_binding_;


	bool validate_param(
		const InitializeParam& param);

	bool validate_param(
		const UpdateParam& param);

	static GLenum ogl_get_target(
		const RendererBufferKind kind);

	static GLenum ogl_get_usage(
		const RendererBufferUsageKind usage_kind);


	void bind(
		RendererOglBufferPtr ogl_buffer);

	void unbind(
		RendererOglBufferPtr ogl_buffer);

	void uninitialize();
}; // RendererOglBuffer

using RendererOglBufferUPtr = std::unique_ptr<RendererOglBuffer>;

//
// RendererOglBuffer
// =========================================================================


} // bstone


#endif // !BSTONE_RENDERER_OGL_BUFFER_INCLUDED
