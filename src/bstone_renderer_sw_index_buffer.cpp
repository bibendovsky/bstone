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


#include "bstone_precompiled.h"
#include "bstone_renderer_sw_index_buffer.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// RendererSwIndexBuffer
//

void RendererSwIndexBuffer::update(
	const RendererIndexBufferUpdateParam& param)
{
	auto renderer_utils = detail::RendererUtils{};

	if (!renderer_utils.validate_index_buffer_update_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return;
	}

	if (param.offset_ >= size_)
	{
		error_message_ = "Offset out of range.";

		return;
	}

	if (param.size_ > size_)
	{
		error_message_ = "Size out of range.";

		return;
	}

	if ((param.offset_ + param.size_) > size_)
	{
		error_message_ = "Block out of range.";

		return;
	}

	std::uninitialized_copy_n(
		static_cast<const std::uint8_t*>(param.data_),
		param.size_,
		data_.begin() + param.offset_
	);
}

const std::string& RendererSwIndexBuffer::get_error_message() const
{
	return error_message_;
}

bool RendererSwIndexBuffer::initialize(
	const RendererIndexBufferCreateParam& param)
{
	auto renderer_utils = detail::RendererUtils{};

	if (!renderer_utils.validate_index_buffer_create_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	byte_depth_ = param.byte_depth_;
	size_ = param.size_;
	data_.resize(size_);

	return true;
}

int RendererSwIndexBuffer::get_byte_depth() const
{
	return byte_depth_;
}

int RendererSwIndexBuffer::get_size() const
{
	return size_;
}

const void* RendererSwIndexBuffer::get_data() const
{
	return data_.data();
}

//
// RendererSwIndexBuffer
// ==========================================================================


} // bstone
