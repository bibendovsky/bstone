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
// OpenGL vertex array object (implementation interface).
//


#ifndef BSTONE_DETAIL_OGL_VAO_INCLUDED
#define BSTONE_DETAIL_OGL_VAO_INCLUDED


#include <memory>


namespace bstone
{


class RendererBuffer;
using RendererBufferPtr = RendererBuffer*;


namespace detail
{


class OglState;
using OglStatePtr = OglState*;

class OglVaoManager;
using OglVaoManagerPtr = OglVaoManager*;


// ==========================================================================
// OglVao
//

class OglVao
{
protected:
	OglVao();


public:
	virtual ~OglVao();


	virtual void bind() = 0;


	virtual bool index_buffer_set_current(
		const RendererBufferPtr index_buffer) = 0;


	virtual void enable_location(
		const int location,
		const bool is_enable) = 0;
}; // OglVao

using OglVaoPtr = OglVao*;
using OglVaoUPtr = std::unique_ptr<OglVao>;

//
// OglVao
// ==========================================================================


// ==========================================================================
// OglVaoDeleter
//

class OglVaoDeleter
{
public:
	OglVaoDeleter(
		const OglStatePtr ogl_state);

	void operator()(
		const OglVaoPtr resource);


private:
	const OglStatePtr ogl_state_;
}; // OglVaoDeleter

using OglVaoResource = std::unique_ptr<OglVao, OglVaoDeleter>;

//
// OglVaoDeleter
// ==========================================================================


// ==========================================================================
// OglVaoFactory
//

struct OglVaoFactory final
{
	static OglVaoUPtr create(
		const OglVaoManagerPtr manager);
}; // OglVaoFactory

//
// OglVaoFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_VAO_INCLUDED
