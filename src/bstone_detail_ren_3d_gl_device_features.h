/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL device features.
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_DEVICE_FEATURES_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_DEVICE_FEATURES_INCLUDED


#include "bstone_detail_ren_3d_gl_context_kind.h"


namespace bstone
{
namespace detail
{


struct Ren3dGlDeviceFeatures
{
	Ren3dGlContextKind context_kind_;

	bool is_mipmap_ext_;
	bool is_framebuffer_available_;
	bool is_framebuffer_ext_;
	bool is_vao_available_;
	bool is_buffer_storage_available_;
	bool is_dsa_available_;
	bool is_sso_available_;
}; // Ren3dGlDeviceFeatures


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_DEVICE_FEATURES_INCLUDED
