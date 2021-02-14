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
// OpenGL error manager (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_ERROR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_ERROR_INCLUDED


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlError
//

struct Ren3dGlError
{
	//
	// Throws an exception on OpenGL error for any build.
	//
	static void ensure();

	//
	// Throws an exception on OpenGL error for debug build.
	//
	static void ensure_debug();
}; // Ren3dGlError

//
// Ren3dGlError
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_ERROR_INCLUDED
