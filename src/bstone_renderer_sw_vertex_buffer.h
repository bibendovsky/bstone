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
// Software vertex buffer.
//


#ifndef BSTONE_RENDERER_SW_VERTEX_BUFFER_INCLUDED
#define BSTONE_RENDERER_SW_VERTEX_BUFFER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{


// ==========================================================================
// RendererSwVertexBuffer
//

class RendererSwVertexBuffer :
	public RendererVertexBuffer
{
public:
	RendererSwVertexBuffer() = default;

	RendererSwVertexBuffer(
		const RendererSwVertexBuffer& rhs) = delete;

	~RendererSwVertexBuffer() override = default;


	void update(
		const RendererVertexBufferUpdateParam& param) override;


	bool initialize(
		const RendererVertexBufferCreateParam& param);

	const std::string& get_error_message() const;

	const void* get_data() const;


private:
	using Data = std::vector<std::uint8_t>;


	std::string error_message_;

	int size_;
	Data data_;
}; // VertexBuffer

using RendererSwVertexBufferPtr = RendererSwVertexBuffer*;
using RendererSwVertexBufferUPtr = std::unique_ptr<RendererSwVertexBuffer>;

//
// RendererSwVertexBuffer
// ==========================================================================


} // bstone


#endif // !BSTONE_RENDERER_SW_VERTEX_BUFFER_INCLUDED
