/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer

#ifndef BSTONE_GL_R3R_INCLUDED
#define BSTONE_GL_R3R_INCLUDED

#include "bstone_r3r.h"

namespace bstone {

R3rUPtr make_gl_r3r(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_INCLUDED
