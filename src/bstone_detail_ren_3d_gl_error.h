/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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

	//
	// Asserts if there was any OpenGL error.
	//
	static void ensure_assert() noexcept;
}; // Ren3dGlError

//
// Ren3dGlError
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_ERROR_INCLUDED
