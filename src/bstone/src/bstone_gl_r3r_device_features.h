/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Device Features

#ifndef BSTONE_GL_R3R_DEVICE_FEATURES_INCLUDED
#define BSTONE_GL_R3R_DEVICE_FEATURES_INCLUDED

#include "bstone_sys_gl_context_profile.h"

namespace bstone {

struct GlR3rDeviceFeatures
{
	sys::GlContextProfile context_profile;

	bool is_mipmap_ext;
	bool is_framebuffer_available;
	bool is_framebuffer_ext;
	bool is_vao_available;
	bool is_buffer_storage_available;
	bool is_dsa_available;
	bool is_sso_available;
};

} // namespace bstone

#endif // BSTONE_GL_R3R_DEVICE_FEATURES_INCLUDED
