/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// OpenGL context kind.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_CONTEXT_KIND_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_CONTEXT_KIND_INCLUDED


namespace bstone
{
namespace detail
{


enum class Ren3dGlContextKind
{
	invalid,
	none,
	core,
	compatibility,
	es,
}; // Ren3dGlContextKind


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_CONTEXT_KIND_INCLUDED
