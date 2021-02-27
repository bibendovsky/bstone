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

struct Ren3dGlBufferImplException :
	public Exception
{
	explicit Ren3dGlBufferImplException(
		const char* const message)
		:
		Exception{std::string{"[GL_BUF] "} + message}
	{
	}
}; // Ren3dGlBufferImplException


struct Ren3dGlBufferImplCreateException :
	public Exception
{
	explicit Ren3dGlBufferImplCreateException(
		const char* const message)
		:
		Exception{std::string{"[GL_BUF_INIT] "} + message}
	{
	}
}; // Ren3dGlBufferImplCreateException

struct Ren3dGlBufferImplUpdateException :
	public Exception
{
	explicit Ren3dGlBufferImplUpdateException(
		const char* const message)
		:
		Exception{std::string{"[GL_BUF_UPD] "} + message}
	{
	}
}; // Ren3dGlBufferImplUpdateException


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
	static void buffer_deleter(
		const GLuint& gl_name) noexcept;

	using BufferResource = UniqueResource<GLuint, buffer_deleter>;

	const Ren3dGlContextPtr context_;
	const Ren3dGlDeviceFeatures& gl_device_features_;

	Ren3dBufferKind kind_;
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
	:
	context_{context},
	gl_device_features_{context_->get_gl_device_features()},
	kind_{},
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
		throw Ren3dGlBufferImplCreateException{"Failed to create a resource."};
	}

	kind_ = param.kind_;
	usage_kind_ = param.usage_kind_;
	size_ = param.size_;
	gl_target_ = gl_get_target(param.kind_);

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

Ren3dGlBufferImpl::~Ren3dGlBufferImpl() = default;

Ren3dBufferKind Ren3dGlBufferImpl::get_kind() const noexcept
{
	return kind_;
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
{
	const auto gl_name = (is_set ? buffer_resource_.get() : 0);

	glBindBuffer(gl_target_, gl_name);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlBufferImpl::update(
	const Ren3dUpdateBufferParam& param)
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

void Ren3dGlBufferImpl::buffer_deleter(
	const GLuint& gl_name) noexcept
{
	glDeleteBuffers(1, &gl_name);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlBufferImpl::validate(
	const Ren3dCreateBufferParam& param)
{
	switch (param.kind_)
	{
		case Ren3dBufferKind::index:
		case Ren3dBufferKind::vertex:
			break;

		default:
			throw Ren3dGlBufferImplCreateException{"Unsupported kind."};
	}

	switch (param.usage_kind_)
	{
		case Ren3dBufferUsageKind::draw_streaming:
		case Ren3dBufferUsageKind::draw_static:
		case Ren3dBufferUsageKind::draw_dynamic:
			break;

		default:
			throw Ren3dGlBufferImplCreateException{"Unsupported usage kind."};
	}

	if (param.size_ <= 0)
	{
		throw Ren3dGlBufferImplCreateException{"Size out of range."};
	}
}

void Ren3dGlBufferImpl::validate(
	const Ren3dUpdateBufferParam& param)
{
	if (param.offset_ < 0)
	{
		throw Ren3dGlBufferImplUpdateException{"Offset out of range."};
	}

	if (param.size_ < 0)
	{
		throw Ren3dGlBufferImplUpdateException{"Size out of range."};
	}

	if (param.offset_ > size_)
	{
		throw Ren3dGlBufferImplUpdateException{"Offset out of range."};
	}

	if (param.size_ > size_)
	{
		throw Ren3dGlBufferImplUpdateException{"Size out of range."};
	}

	if ((param.offset_ + param.size_) > size_)
	{
		throw Ren3dGlBufferImplUpdateException{"End offset out of range."};
	}

	if (param.size_ > 0 && !param.data_)
	{
		throw Ren3dGlBufferImplUpdateException{"Null data."};
	}
}

GLenum Ren3dGlBufferImpl::gl_get_target(
	const Ren3dBufferKind kind)
{
	switch (kind)
	{
		case Ren3dBufferKind::index:
			return GL_ELEMENT_ARRAY_BUFFER;

		case Ren3dBufferKind::vertex:
			return GL_ARRAY_BUFFER;

		default:
			throw Ren3dGlBufferImplException{"Unsupported kind."};
	}
}

GLenum Ren3dGlBufferImpl::gl_get_usage(
	const Ren3dBufferUsageKind usage_kind)
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
			throw Ren3dGlBufferImplException{"Unsupported usage kind."};
	}
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
