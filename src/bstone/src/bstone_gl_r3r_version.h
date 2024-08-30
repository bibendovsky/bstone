/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Version

#ifndef BSTONE_GL_R3R_VERSION_INCLUDED
#define BSTONE_GL_R3R_VERSION_INCLUDED

#include <string>

namespace bstone {

struct GlR3rVersion
{
	bool is_es;

	int major;
	int minor;
	int release;
	std::string vendor;
};

} // namespace bstone

#endif // BSTONE_GL_R3R_VERSION_INCLUDED
