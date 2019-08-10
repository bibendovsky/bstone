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
// OpenGL vertex buffer implementation.
//


#ifndef BSTONE_RENDERER_OGL_VERTEX_BUFFER_INCLUDED
#define BSTONE_RENDERER_OGL_VERTEX_BUFFER_INCLUDED


#include "bstone_detail_ogl_buffer.h"
#include "bstone_renderer_vertex_buffer_impl.h"


namespace bstone
{


class OglStateImpl;
using OglStateImplPtr = OglStateImpl*;


// ==========================================================================
// RendererOglVertexBuffer
//

class RendererOglVertexBuffer :
	public RendererVertexBufferImpl
{
public:
	RendererOglVertexBuffer(
		const OglStateImplPtr ogl_state);

	RendererOglVertexBuffer(
		const RendererOglVertexBuffer& rhs) = delete;

	~RendererOglVertexBuffer() override = default;


	RendererBufferUsageKind get_usage_kind() const override;

	int get_size() const override;

	void update(
		const RendererVertexBufferUpdateParam& param) override;

	void bind(
		const bool is_binded) override;


	const std::string& get_error_message() const override;

	bool initialize(
		const RendererVertexBufferCreateParam& param) override;


private:
	OglStateImplPtr ogl_state_;

	std::string error_message_;

	detail::OglBufferUPtr ogl_buffer_;
}; // RendererOglVertexBuffer

using RendererOglVertexBufferPtr = RendererOglVertexBuffer*;
using RendererOglVertexBufferUPtr = std::unique_ptr<RendererOglVertexBuffer>;

//
// RendererOglVertexBuffer
// ==========================================================================


} // bstone


#endif // !BSTONE_RENDERER_OGL_VERTEX_BUFFER_INCLUDED
