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
// Software index buffer.
//


#ifndef BSTONE_RENDERER_SW_INDEX_BUFFER_INCLUDED
#define BSTONE_RENDERER_SW_INDEX_BUFFER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{


// =========================================================================
// RendererSwIndexBuffer
//

class RendererSwIndexBuffer :
	public RendererIndexBuffer
{
public:
	RendererSwIndexBuffer() = default;

	RendererSwIndexBuffer(
		const RendererSwIndexBuffer& rhs) = delete;

	~RendererSwIndexBuffer() override = default;


	void update(
		const RendererIndexBufferUpdateParam& param) override;


	const std::string& get_error_message() const;

	bool initialize(
		const RendererIndexBufferCreateParam& param);

	int get_byte_depth() const;

	int get_size() const;

	const void* get_data() const;


private:
	using Data = std::vector<std::uint8_t>;


	std::string error_message_;

	int byte_depth_;
	Data data_;
}; // RendererSwIndexBuffer

using RendererSwIndexBufferPtr = RendererSwIndexBuffer*;
using RendererSwIndexBufferUPtr = std::unique_ptr<RendererSwIndexBuffer>;

//
// RendererSwIndexBuffer
// =========================================================================


} // bstone


#endif // !BSTONE_RENDERER_SW_INDEX_BUFFER_INCLUDED
