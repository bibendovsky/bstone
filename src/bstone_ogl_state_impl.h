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
// OpenGL state implementation.
//


#ifndef BSTONE_OGL_STATE_IMPL_INCLUDED
#define BSTONE_OGL_STATE_IMPL_INCLUDED


#include "bstone_renderer_ogl_buffer.h"


namespace bstone
{


// =========================================================================
// OglStateImpl
//

class OglStateImpl
{
public:
	OglStateImpl();

	OglStateImpl(
		const OglStateImpl& rhs) = delete;

	virtual ~OglStateImpl() = 0;


	virtual const std::string& get_error_message() const noexcept = 0;

	virtual void initialize() = 0;

	virtual bool is_initialized() const noexcept = 0;


	virtual void buffer_bind(
		const RendererOglBufferPtr buffer) = 0;

	virtual void buffer_unbind(
		const RendererBufferKind target) = 0;
}; // RendererOglBuffer


using OglStateImplPtr = OglStateImpl*;
using OglStateImplUPtr = std::unique_ptr<OglStateImpl>;

//
// OglStateImpl
// =========================================================================


// =========================================================================
// OglStateImplFactory
//


struct OglStateImplFactory
{
	static OglStateImplUPtr create(
		const RendererPath renderer_kind);
}; // OglStateImplFactory


//
// OglStateImplFactory
// =========================================================================


} // bstone


#endif // !BSTONE_OGL_STATE_IMPL_INCLUDED
