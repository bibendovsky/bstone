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
// OpenGL device features.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OGL_DEVICE_FEATURES_INCLUDED
#define BSTONE_DETAIL_OGL_DEVICE_FEATURES_INCLUDED


#include "bstone_ogl_api.h"
#include "bstone_detail_ogl_context_kind.h"


namespace bstone
{
namespace detail
{


struct OglDeviceFeatures
{
	OglContextKind context_kind_;

	PFNGLGENERATEMIPMAPPROC mipmap_function_;

	bool framebuffer_is_arb_;

	bool vao_is_available_;
}; // OglDeviceFeatures


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_DEVICE_FEATURES_INCLUDED
