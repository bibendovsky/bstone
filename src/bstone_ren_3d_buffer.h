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
// 3D renderer's buffer.
//


#ifndef BSTONE_REN_3D_BUFFER_INCLUDED
#define BSTONE_REN_3D_BUFFER_INCLUDED


#include <memory>


namespace bstone
{


// ==========================================================================
// Ren3dBuffer
//

enum class Ren3dBufferKind
{
	none,
	index,
	vertex,
}; // Ren3dBufferKind

enum class Ren3dBufferUsageKind
{
	none,

	// Target - drawing; updates - continuous.
	draw_streaming,

	// Target - drawing; updates - seldom.
	draw_static,

	// Target - drawing; updates - often.
	draw_dynamic,
}; // Ren3dBufferUsageKind

struct Ren3dCreateBufferParam
{
	Ren3dBufferKind kind_;
	Ren3dBufferUsageKind usage_kind_;
	int size_;
}; // Ren3dCreateBufferParam

struct Ren3dUpdateBufferParam
{
	int offset_;
	int size_;
	const void* data_;
}; // Ren3dUpdateBufferParam


class Ren3dBuffer
{
protected:
	Ren3dBuffer() = default;


public:
	virtual ~Ren3dBuffer() = default;


	virtual Ren3dBufferKind get_kind() const noexcept = 0;

	virtual Ren3dBufferUsageKind get_usage_kind() const noexcept = 0;

	virtual int get_size() const noexcept = 0;

	virtual void update(
		const Ren3dUpdateBufferParam& param) = 0;
}; // Ren3dBuffer

using Ren3dBufferPtr = Ren3dBuffer*;
using Ren3dBufferUPtr = std::unique_ptr<Ren3dBuffer>;

//
// Ren3dBuffer
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_BUFFER_INCLUDED
