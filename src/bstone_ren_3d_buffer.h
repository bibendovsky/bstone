/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	Ren3dBufferKind kind;
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
public:
	Ren3dBuffer() noexcept = default;

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
