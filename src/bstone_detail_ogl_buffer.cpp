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
// OpenGL buffer object (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_state.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglBuffer
//

OglBuffer::OglBuffer()
	:
	error_message_{},
	kind_{},
	usage_kind_{},
	size_{},
	ogl_name_raii_{},
	ogl_target_{}
{
}

OglBuffer::~OglBuffer()
{
	uninitialize();
}

RendererBufferKind OglBuffer::get_kind() const noexcept
{
	return kind_;
}

RendererBufferUsageKind OglBuffer::get_usage_kind() const noexcept
{
	return usage_kind_;
}

int OglBuffer::get_size() const noexcept
{
	return size_;
}

void OglBuffer::bind()
{
	bind(this);
}

void OglBuffer::unbind_target()
{
	bind(nullptr);
}

void OglBuffer::update(
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

const std::string& OglBuffer::get_error_message() const
{
	return error_message_;
}

bool OglBuffer::initialize(
	const InitializeParam& param)
{
	if (!validate_param(param))
	{
		return false;
	}

	ogl_state_ = param.ogl_state_;

	auto ogl_name = GLuint{};
	::glGenBuffers(1, &ogl_name);
	assert(!detail::OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		error_message_ = "Failed to create OpenGL buffer object.";

		return false;
	}

	auto ogl_name_raii = OglBufferHandle{ogl_name};

	const auto olg_target = ogl_get_target(param.kind_);
	const auto olg_usage = ogl_get_usage(param.usage_kind_);

	::glBindBuffer(olg_target, ogl_name_raii.get());
	assert(!detail::OglRendererUtils::was_errors());

	::glBufferData(olg_target, param.size_, nullptr, olg_usage);
	assert(!detail::OglRendererUtils::was_errors());

	kind_ = param.kind_;
	usage_kind_ = param.usage_kind_;
	size_ = param.size_;
	ogl_name_raii_ = std::move(ogl_name_raii);
	ogl_target_ = olg_target;

	bind(this);

	return true;
}

bool OglBuffer::is_initialized() const noexcept
{
	return ogl_name_raii_ != nullptr;
}

GLuint OglBuffer::get_ogl_name() const noexcept
{
	return ogl_name_raii_.get();
}

bool OglBuffer::validate_param(
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

	if (param.ogl_state_ == nullptr)
	{
		error_message_ = "Null OpenGL state.";

		return false;
	}

	if (!param.ogl_state_->is_initialized())
	{
		error_message_ = "OpenGL state not initialized.";

		return false;
	}

	return true;
}

bool OglBuffer::validate_param(
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

	if (param.offset_ > size_)
	{
		error_message_ = "Offset out of range.";

		return false;
	}

	if (param.size_ > size_)
	{
		error_message_ = "Size out of range.";

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

GLenum OglBuffer::ogl_get_target(
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

GLenum OglBuffer::ogl_get_usage(
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

void OglBuffer::bind(
	OglBufferPtr ogl_buffer)
{
	if (ogl_buffer)
	{
		ogl_state_->buffer_bind(ogl_buffer);
	}
	else
	{
		ogl_state_->buffer_unbind(kind_);
	}
}

void OglBuffer::uninitialize()
{
	if (!is_initialized())
	{
		return;
	}

	bind(nullptr);

	kind_ = {};
	usage_kind_ = {};
	size_ = 0;
	ogl_name_raii_ = {};
	ogl_target_ = {};
}

//
// OglBuffer
// =========================================================================


} // detail
} // bstone
