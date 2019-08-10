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
// OpenGL vertex buffer object (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED
#define BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED


#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_vertex_buffer.h"


namespace bstone
{
namespace detail
{


class OglState;
using OglStatePtr = OglState*;


// ==========================================================================
// OglVertexBuffer
//

class OglVertexBuffer :
	public VertexBuffer
{
public:
	OglVertexBuffer(
		const OglStatePtr ogl_state);

	OglVertexBuffer(
		const OglVertexBuffer& rhs) = delete;

	~OglVertexBuffer() override = default;


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
	OglStatePtr ogl_state_;

	std::string error_message_;

	detail::OglBufferUPtr ogl_buffer_;
}; // OglVertexBuffer

using OglVertexBufferPtr = OglVertexBuffer*;
using OglVertexBufferUPtr = std::unique_ptr<OglVertexBuffer>;

//
// OglVertexBuffer
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED
