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
// OpenGL vertex buffer object (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_vertex_buffer.h"

#include "bstone_exception.h"

#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglVertexBuffer
//

OglVertexBuffer::OglVertexBuffer() = default;

OglVertexBuffer::~OglVertexBuffer() = default;

//
// OglVertexBuffer
// ==========================================================================


// ==========================================================================
// GenericOglVertexBuffer
//

class GenericOglVertexBuffer :
	public OglVertexBuffer
{
public:
	GenericOglVertexBuffer(
		const OglContextPtr ogl_context,
		const RendererVertexBufferCreateParam& param);

	~GenericOglVertexBuffer() override = default;


	RendererBufferKind get_kind() const noexcept override;

	RendererBufferUsageKind get_usage_kind() const noexcept override;

	int get_size() const noexcept override;

	void update(
		const RendererBufferUpdateParam& param) override;

	void bind(
		const bool is_binded) override;


private:
	detail::OglBufferUPtr ogl_buffer_;


	void initialize(
		const OglContextPtr ogl_context,
		const RendererVertexBufferCreateParam& param);
}; // GenericOglVertexBuffer

using GenericOglVertexBufferPtr = GenericOglVertexBuffer*;
using GenericOglVertexBufferUPtr = std::unique_ptr<GenericOglVertexBuffer>;

//
// GenericOglVertexBuffer
// ==========================================================================


// ==========================================================================
// GenericOglVertexBuffer
//

GenericOglVertexBuffer::GenericOglVertexBuffer(
	const OglContextPtr ogl_context,
	const RendererVertexBufferCreateParam& param)
	:
	ogl_buffer_{}
{
	initialize(ogl_context, param);
}

RendererBufferKind GenericOglVertexBuffer::get_kind() const noexcept
{
	return ogl_buffer_->get_kind();
}

RendererBufferUsageKind GenericOglVertexBuffer::get_usage_kind() const noexcept
{
	return ogl_buffer_->get_usage_kind();
}

void GenericOglVertexBuffer::update(
	const RendererBufferUpdateParam& param)
{
	ogl_buffer_->update(param);
}

void GenericOglVertexBuffer::initialize(
	const OglContextPtr ogl_context,
	const RendererVertexBufferCreateParam& param)
{
	RendererUtils::validate_vertex_buffer_create_param(param);

	auto buffer_param = OglBufferFactory::InitializeParam{};
	buffer_param.kind_ = RendererBufferKind::vertex;
	buffer_param.usage_kind_ = param.usage_kind_;
	buffer_param.size_ = param.size_;

	ogl_buffer_ = detail::OglBufferFactory::create(ogl_context, buffer_param);
}

int GenericOglVertexBuffer::get_size() const noexcept
{
	return ogl_buffer_->get_size();
}

void GenericOglVertexBuffer::bind(
	const bool is_binded)
{
	ogl_buffer_->bind(is_binded);
}

//
// GenericOglVertexBuffer
// ==========================================================================


// ==========================================================================
// OglVertexBufferFactory
//

OglVertexBufferUPtr OglVertexBufferFactory::create(
	const OglContextPtr ogl_context,
	const RendererVertexBufferCreateParam& param)
{
	return std::make_unique<GenericOglVertexBuffer>(ogl_context, param);
}

//
// OglVertexBufferFactory
// ==========================================================================


} // detail
} // bstone
