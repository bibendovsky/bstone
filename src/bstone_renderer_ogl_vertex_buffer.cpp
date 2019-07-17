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
// OpenGL vertex buffer.
//


#include "bstone_precompiled.h"
#include "bstone_renderer_ogl_vertex_buffer.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// RendererOglVertexBuffer
//

RendererBufferUsageKind RendererOglVertexBuffer::get_usage_kind() const
{
	return ogl_buffer_->get_usage_kind();
}

void RendererOglVertexBuffer::update(
	const RendererVertexBufferUpdateParam& param)
{
	auto buffer_param = RendererOglBuffer::UpdateParam{};
	buffer_param.offset_ = param.offset_;
	buffer_param.size_ = param.size_;
	buffer_param.data_ = param.data_;

	ogl_buffer_->update(buffer_param);
}

bool RendererOglVertexBuffer::initialize(
	const RendererVertexBufferCreateParam& param)
{
	auto renderer_utils = detail::RendererUtils{};

	if (!renderer_utils.validate_vertex_buffer_create_param(param))
	{
		error_message_ = renderer_utils.get_error_message();

		return false;
	}

	auto ogl_buffer = RendererOglBufferUPtr{new RendererOglBuffer{}};

	auto buffer_param = RendererOglBuffer::InitializeParam{};
	buffer_param.kind_ = RendererBufferKind::vertex;
	buffer_param.usage_kind_ = param.usage_kind_;
	buffer_param.size_ = param.size_;

	if (!ogl_buffer->initialize(buffer_param))
	{
		error_message_ = ogl_buffer->get_error_message();

		return false;
	}

	ogl_buffer_ = std::move(ogl_buffer);

	return true;
}

const std::string& RendererOglVertexBuffer::get_error_message() const
{
	return error_message_;
}

int RendererOglVertexBuffer::get_size() const
{
	return ogl_buffer_->get_size();
}

void RendererOglVertexBuffer::bind(
	const bool is_binded)
{
	if (is_binded)
	{
		ogl_buffer_->bind();
	}
	else
	{
		ogl_buffer_->unbind_target();
	}
}

//
// RendererOglVertexBuffer
// ==========================================================================


} // bstone
