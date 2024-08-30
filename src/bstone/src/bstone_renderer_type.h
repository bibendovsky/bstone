/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Common renderer stuff.
//


#ifndef BSTONE_RENDERER_INCLUDED
#define BSTONE_RENDERER_INCLUDED


namespace bstone
{


enum class RendererType
{
	auto_detect,

	software,

	gl_2_0,
	gl_3_2_core,

	gles_2_0,
}; // RendererType


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
