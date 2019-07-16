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


#include "bstone_precompiled.h"
#include "bstone_renderer_ogl_buffer.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{


// =========================================================================
// RendererOglBuffer
//

RendererOglBuffer* RendererOglBuffer::index_binding_;
RendererOglBuffer* RendererOglBuffer::vertex_binding_;


RendererOglBuffer::RendererOglBuffer()
	:
	error_message_{},
	kind_{},
	usage_kind_{},
	size_{},
	ogl_name_raii_{},
	ogl_target_{},
	binding_{}
{
}

RendererOglBuffer::~RendererOglBuffer()
{
	uninitialize();
}

RendererBufferKind RendererOglBuffer::get_kind() const noexcept
{
	return kind_;
}

RendererBufferUsageKind RendererOglBuffer::get_usage_kind() const noexcept
{
	return usage_kind_;
}

int RendererOglBuffer::get_size() const noexcept
{
	return size_;
}

void RendererOglBuffer::bind()
{
	bind(this);
}

void RendererOglBuffer::unbind_target()
{
	bind(nullptr);
}

void RendererOglBuffer::update(
	const UpdateParam& param)
{
	if (!validate_param(param))
	{
		return;
	}

	if (param.size_ == 0)
	{
		return;
	}

	bind(this);

	::glBufferSubData(
		ogl_target_,
		param.offset_,
		param.size_,
		param.data_
	);

	assert(!detail::OglRendererUtils::was_errors());
}

const std::string& RendererOglBuffer::get_error_message() const
{
	return error_message_;
}

bool RendererOglBuffer::initialize(
	const InitializeParam& param)
{
	if (!validate_param(param))
	{
		return false;
	}

	auto ogl_name = GLuint{};
	::glGenBuffers(1, &ogl_name);
	assert(!detail::OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		error_message_ = "Failed to create OpenGL buffer object.";

		return false;
	}

	auto ogl_name_raii = OglBufferRaii{ogl_name};

	const auto olg_target = ogl_get_target(param.kind_);
	const auto olg_usage = ogl_get_usage(param.usage_kind_);

	::glBindBuffer(olg_target, ogl_name_raii);
	assert(!detail::OglRendererUtils::was_errors());

	::glBufferData(olg_target, param.size_, nullptr, olg_usage);
	assert(!detail::OglRendererUtils::was_errors());

	kind_ = param.kind_;
	usage_kind_ = param.usage_kind_;
	size_ = param.size_;
	ogl_name_raii_ = std::move(ogl_name_raii);
	ogl_target_ = olg_target;

	switch (kind_)
	{
		case RendererBufferKind::index:
			binding_ = &index_binding_;
			break;

		case RendererBufferKind::vertex:
			binding_ = &vertex_binding_;
			break;

		default:
			break;
	}

	bind(this);

	return true;
}

bool RendererOglBuffer::is_initialized() const noexcept
{
	return ogl_name_raii_ != 0;
}

void RendererOglBuffer::reset_bindings()
{
	index_binding_ = nullptr;
	vertex_binding_ = nullptr;

	::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	::glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool RendererOglBuffer::validate_param(
	const InitializeParam& param)
{
	switch (param.kind_)
	{
		case RendererBufferKind::index:
		case RendererBufferKind::vertex:
			break;

		default:
			error_message_ = "Invalid kind.";

			return false;
	}

	switch (param.usage_kind_)
	{
		case RendererBufferUsageKind::stream_draw:
		case RendererBufferUsageKind::static_draw:
		case RendererBufferUsageKind::dynamic_draw:
			break;

		default:
			error_message_ = "Invalid usage kind.";

			return false;
	}

	if (param.size_ <= 0)
	{
		error_message_ = "Non-positive size.";

		return false;
	}

	return true;
}

bool RendererOglBuffer::validate_param(
	const UpdateParam& param)
{
	if (param.offset_ < 0)
	{
		error_message_ = "Negative offset.";

		return false;
	}

	if (param.size_ < 0)
	{
		error_message_ = "Negative size.";

		return false;
	}

	if (param.offset_ > param.size_)
	{
		error_message_ = "Offset out of range.";

		return false;
	}

	if ((param.offset_ + param.size_) > size_)
	{
		error_message_ = "End offset out of range.";

		return false;
	}

	if (param.size_ > 0 && param.data_ == nullptr)
	{
		error_message_ = "Null data.";

		return false;
	}

	return true;
}

GLenum RendererOglBuffer::ogl_get_target(
	const RendererBufferKind kind)
{
	switch (kind)
	{
		case RendererBufferKind::index:
			return GL_ELEMENT_ARRAY_BUFFER;

		case RendererBufferKind::vertex:
			return GL_ARRAY_BUFFER;

		default:
			assert(!"Invalid kind.");

			return 0;
	}
}

GLenum RendererOglBuffer::ogl_get_usage(
	const RendererBufferUsageKind usage_kind)
{
	switch (usage_kind)
	{
		case RendererBufferUsageKind::stream_draw:
			return GL_STREAM_DRAW;

		case RendererBufferUsageKind::static_draw:
			return GL_STATIC_DRAW;

		case RendererBufferUsageKind::dynamic_draw:
			return GL_DYNAMIC_DRAW;

		default:
			assert(!"Invalid usage kind.");

			return 0;
	}
}

void RendererOglBuffer::bind(
	RendererOglBufferPtr ogl_buffer)
{
	if (*binding_ == ogl_buffer)
	{
		return;
	}

	const auto ogl_name = (ogl_buffer != nullptr ? ogl_buffer->ogl_name_raii_.get() : 0);

	::glBindBuffer(ogl_target_, ogl_name);
	assert(!detail::OglRendererUtils::was_errors());

	*binding_ = ogl_buffer;
}

void RendererOglBuffer::unbind(
	RendererOglBufferPtr ogl_buffer)
{
	if (*binding_ != ogl_buffer)
	{
		return;
	}

	bind(nullptr);
}

void RendererOglBuffer::uninitialize()
{
	if (!is_initialized())
	{
		return;
	}

	unbind(this);

	kind_ = {};
	usage_kind_ = {};
	size_ = 0;
	ogl_name_raii_ = {};
	ogl_target_ = {};
	binding_ = {};
}

//
// RendererOglBuffer
// =========================================================================


} // bstone
