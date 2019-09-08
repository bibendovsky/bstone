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
// OpenGL index buffer object (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_index_buffer.h"

#include "bstone_exception.h"

#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglIndexBuffer
//

OglIndexBuffer::OglIndexBuffer() = default;

OglIndexBuffer::~OglIndexBuffer() = default;

//
// OglIndexBuffer
// =========================================================================


// =========================================================================
// GenericOglIndexBuffer
//

class GenericOglIndexBuffer :
	public OglIndexBuffer
{
public:
	GenericOglIndexBuffer(
		const OglBufferManagerPtr ogl_buffer_manager,
		const RendererIndexBufferCreateParam& param);

	~GenericOglIndexBuffer() override = default;


	RendererBufferKind get_kind() const noexcept override;

	RendererBufferUsageKind get_usage_kind() const noexcept override;

	int get_byte_depth() const noexcept override;

	int get_size() const noexcept override;

	void update(
		const RendererBufferUpdateParam& param) override;

	void set(
		const bool is_set) override;


private:
	int byte_depth_;
	OglBufferUPtr ogl_buffer_;


	void initialize(
		const OglBufferManagerPtr ogl_buffer_manager,
		const RendererIndexBufferCreateParam& param);
}; // GenericOglIndexBuffer

using GenericOglIndexBufferPtr = GenericOglIndexBuffer*;
using GenericOglIndexBufferUPtr = std::unique_ptr<GenericOglIndexBuffer>;

//
// GenericOglIndexBuffer
// =========================================================================


// ==========================================================================
// GenericOglIndexBuffer
//

GenericOglIndexBuffer::GenericOglIndexBuffer(
	const OglBufferManagerPtr ogl_buffer_manager,
	const RendererIndexBufferCreateParam& param)
	:
	byte_depth_{},
	ogl_buffer_{}
{
	initialize(ogl_buffer_manager, param);
}

void GenericOglIndexBuffer::update(
	const RendererBufferUpdateParam& param)
{
	ogl_buffer_->update(param);
}

void GenericOglIndexBuffer::initialize(
	const OglBufferManagerPtr ogl_buffer_manager,
	const RendererIndexBufferCreateParam& param)
{
	RendererUtils::validate_index_buffer_create_param(param);

	auto buffer_param = OglBufferFactory::InitializeParam{};
	buffer_param.kind_ = RendererBufferKind::index;
	buffer_param.usage_kind_ = param.usage_kind_;
	buffer_param.size_ = param.size_;

	byte_depth_ = param.byte_depth_;
	ogl_buffer_ = OglBufferFactory::create(ogl_buffer_manager, buffer_param);
}

RendererBufferKind GenericOglIndexBuffer::get_kind() const noexcept
{
	return ogl_buffer_->get_kind();
}

RendererBufferUsageKind GenericOglIndexBuffer::get_usage_kind() const noexcept
{
	return ogl_buffer_->get_usage_kind();
}

int GenericOglIndexBuffer::get_byte_depth() const noexcept
{
	return byte_depth_;
}

int GenericOglIndexBuffer::get_size() const noexcept
{
	return ogl_buffer_->get_size();
}

void GenericOglIndexBuffer::set(
	const bool is_set)
{
	ogl_buffer_->set(is_set);
}

//
// GenericOglIndexBuffer
// ==========================================================================


// =========================================================================
// OglIndexBufferFactory
//

OglIndexBufferUPtr OglIndexBufferFactory::create(
	const OglBufferManagerPtr ogl_buffer_manager,
	const RendererIndexBufferCreateParam& param)
{
	return std::make_unique<GenericOglIndexBuffer>(ogl_buffer_manager, param);
}

//
// OglIndexBufferFactory
// =========================================================================


} // detail
} // bstone
