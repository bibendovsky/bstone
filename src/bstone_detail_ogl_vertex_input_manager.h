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
// OpenGL vertex input manager (implementation interface).
//


#ifndef BSTONE_DETAIL_OGL_VERTEX_INPUT_MANAGER_INCLUDED
#define BSTONE_DETAIL_OGL_VERTEX_INPUT_MANAGER_INCLUDED


#include <memory>


namespace bstone
{


class RendererIndexBuffer;
using RendererIndexBufferPtr = RendererIndexBuffer*;

class RendererVertexInput;
using RendererVertexInputPtr = RendererVertexInput*;

struct RendererVertexInputCreateParam;


namespace detail
{


class OglContext;
using OglContextPtr = OglContext*;


// ==========================================================================
// OglVertexInputManager
//

class OglVertexInputManager
{
protected:
	OglVertexInputManager();


public:
	virtual ~OglVertexInputManager();


	virtual OglContextPtr ogl_context_get() const noexcept = 0;


	virtual RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) = 0;

	virtual void vertex_input_destroy(
		const RendererVertexInputPtr vertex_input) = 0;

	virtual void vertex_input_set(
		const RendererVertexInputPtr vertex_input) = 0;

	virtual RendererVertexInputPtr vertex_input_get_current() const noexcept = 0;

	virtual RendererIndexBufferPtr vertex_input_current_get_index_buffer() const noexcept = 0;


	virtual void vertex_input_location_enable(
		const int location,
		const bool is_enabled) = 0;

	virtual void vertex_input_location_assign_begin() = 0;

	virtual void vertex_input_location_assign_end() = 0;
}; // OglVertexInputManager

using OglVertexInputManagerPtr = OglVertexInputManager*;
using OglVertexInputManagerUPtr = std::unique_ptr<OglVertexInputManager>;

//
// OglVertexInputManager
// ==========================================================================


// ==========================================================================
// OglVertexInputManagerFactory
//

struct OglVertexInputManagerFactory final
{
	static OglVertexInputManagerUPtr create(
		const OglContextPtr ogl_context);
}; // OglVertexInputManagerFactory

//
// OglVertexInputManagerFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_VERTEX_INPUT_MANAGER_INCLUDED
