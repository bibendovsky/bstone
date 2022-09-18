/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL version.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_VERSION_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_VERSION_INCLUDED


#include <string>


namespace bstone
{
namespace detail
{


struct Ren3dGlVersion
{
	bool is_es_;

	int major_;
	int minor_;
	int release_;
	std::string vendor_;
}; // Ren3dGlVersion


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_VERSION_INCLUDED
