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
// OpenGL buffer object (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_detail_ren_3d_gl_buffer.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// Ren3dGlBufferImpl
//

struct Ren3dGlBufferException :
	public Exception
{
	explicit Ren3dGlBufferException(
		const char* message)
		:
		Exception{"GL_BUF", message}
	{
	}
}; // Ren3dGlBufferException


class Ren3dGlBufferImpl final :
	public Ren3dGlBuffer
{
public:
	Ren3dGlBufferImpl(
		const Ren3dGlContextPtr context,
		const Ren3dCreateBufferParam& param);

	~Ren3dGlBufferImpl() override;


	Ren3dBufferKind get_kind() const noexcept override;

	Ren3dBufferUsageKind get_usage_kind() const noexcept override;

	int get_size() const noexcept override;

	void update(
		const Ren3dUpdateBufferParam& param) override;

	void set(
		const bool is_set) override;


private:
	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void buffer_deleter(
		GLuint gl_name) noexcept;

	using BufferResource = UniqueResource<GLuint, buffer_deleter>;

	const Ren3dGlContextPtr context_;
	const Ren3dGlDeviceFeatures& gl_device_features_;

	Ren3dBufferKind kind;
	Ren3dBufferUsageKind usage_kind_;
	int size_;
	BufferResource buffer_resource_;
	GLenum gl_target_;


	void validate(
		const Ren3dCreateBufferParam& param);

	void validate(
		const Ren3dUpdateBufferParam& param);

	static GLenum gl_get_target(
		const Ren3dBufferKind kind);

	static GLenum gl_get_usage(
		const Ren3dBufferUsageKind usage_kind);
}; // Ren3dGlBufferImpl

//
// Ren3dGlBufferImpl
// =========================================================================


// =========================================================================
// Ren3dGlBufferImpl
//

Ren3dGlBufferImpl::Ren3dGlBufferImpl(
	const Ren3dGlContextPtr context,
	const Ren3dCreateBufferParam& param)
try
	:
	context_{context},
	gl_device_features_{context_->get_gl_device_features()},
	kind{},
	usage_kind_{},
	size_{},
	buffer_resource_{},
	gl_target_{}
{
	validate(param);

	auto gl_name = GLuint{};

	if (gl_device_features_.is_dsa_available_)
	{
		glCreateBuffers(1, &gl_name);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glGenBuffers(1, &gl_name);
		Ren3dGlError::ensure_debug();
	}

	buffer_resource_.reset(gl_name);

	if (!buffer_resource_)
	{
		fail("Failed to create a resource.");
	}

	kind = param.kind;
	usage_kind_ = param.usage_kind_;
	size_ = param.size_;
	gl_target_ = gl_get_target(param.kind);

	const auto gl_usage = gl_get_usage(param.usage_kind_);

	if (gl_device_features_.is_dsa_available_ &&
		gl_device_features_.is_buffer_storage_available_)
	{
		glNamedBufferStorage(buffer_resource_.get(), param.size_, nullptr, GL_DYNAMIC_STORAGE_BIT);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		set(true);

		glBufferData(gl_target_, param.size_, nullptr, gl_usage);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlBufferImpl::~Ren3dGlBufferImpl() = default;

Ren3dBufferKind Ren3dGlBufferImpl::get_kind() const noexcept
{
	return kind;
}

Ren3dBufferUsageKind Ren3dGlBufferImpl::get_usage_kind() const noexcept
{
	return usage_kind_;
}

int Ren3dGlBufferImpl::get_size() const noexcept
{
	return size_;
}

void Ren3dGlBufferImpl::set(
	const bool is_set)
try
{
	const auto gl_name = (is_set ? buffer_resource_.get() : 0);

	glBindBuffer(gl_target_, gl_name);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlBufferImpl::update(
	const Ren3dUpdateBufferParam& param)
try
{
	validate(param);

	if (param.size_ == 0)
	{
		return;
	}

	if (gl_device_features_.is_dsa_available_)
	{
		glNamedBufferSubData(
			buffer_resource_.get(),
			param.offset_,
			param.size_,
			param.data_
		);

		Ren3dGlError::ensure_debug();
	}
	else
	{
		set(true);

		glBufferSubData(
			gl_target_,
			param.offset_,
			param.size_,
			param.data_
		);

		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void Ren3dGlBufferImpl::fail(
	const char* message)
{
	throw Ren3dGlBufferException{message};
}

[[noreturn]]
void Ren3dGlBufferImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlBufferException{message});
}

void Ren3dGlBufferImpl::buffer_deleter(
	GLuint gl_name) noexcept
{
	glDeleteBuffers(1, &gl_name);
	Ren3dGlError::ensure_assert();
}

void Ren3dGlBufferImpl::validate(
	const Ren3dCreateBufferParam& param)
try
{
	switch (param.kind)
	{
		case Ren3dBufferKind::index:
		case Ren3dBufferKind::vertex:
			break;

		default:
			fail("Unsupported kind.");
	}

	switch (param.usage_kind_)
	{
		case Ren3dBufferUsageKind::draw_streaming:
		case Ren3dBufferUsageKind::draw_static:
		case Ren3dBufferUsageKind::draw_dynamic:
			break;

		default:
			fail("Unsupported usage kind.");
	}

	if (param.size_ <= 0)
	{
		fail("Size out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlBufferImpl::validate(
	const Ren3dUpdateBufferParam& param)
try
{
	if (param.offset_ < 0)
	{
		fail("Offset out of range.");
	}

	if (param.size_ < 0)
	{
		fail("Size out of range.");
	}

	if (param.offset_ > size_)
	{
		fail("Offset out of range.");
	}

	if (param.size_ > size_)
	{
		fail("Size out of range.");
	}

	if ((param.offset_ + param.size_) > size_)
	{
		fail("End offset out of range.");
	}

	if (param.size_ > 0 && !param.data_)
	{
		fail("Null data.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

GLenum Ren3dGlBufferImpl::gl_get_target(
	const Ren3dBufferKind kind)
try
{
	switch (kind)
	{
		case Ren3dBufferKind::index:
			return GL_ELEMENT_ARRAY_BUFFER;

		case Ren3dBufferKind::vertex:
			return GL_ARRAY_BUFFER;

		default:
			fail("Unsupported kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

GLenum Ren3dGlBufferImpl::gl_get_usage(
	const Ren3dBufferUsageKind usage_kind)
try
{
	switch (usage_kind)
	{
		case Ren3dBufferUsageKind::draw_streaming:
			return GL_STREAM_DRAW;

		case Ren3dBufferUsageKind::draw_static:
			return GL_STATIC_DRAW;

		case Ren3dBufferUsageKind::draw_dynamic:
			return GL_DYNAMIC_DRAW;

		default:
			fail("Unsupported usage kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlBufferImpl
// =========================================================================


// =========================================================================
// Ren3dGlBufferFactory
//

Ren3dGlBufferUPtr Ren3dGlBufferFactory::create(
	const Ren3dGlContextPtr context,
	const Ren3dCreateBufferParam& param)
{
	return std::make_unique<Ren3dGlBufferImpl>(context, param);
}

//
// Ren3dGlBufferFactory
// =========================================================================


} // detail
} // bstone
