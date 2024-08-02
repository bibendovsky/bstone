/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context profile.

#ifndef BSTONE_SYS_GL_CONTEXT_PROFILE_INCLUDED
#define BSTONE_SYS_GL_CONTEXT_PROFILE_INCLUDED

namespace bstone {
namespace sys {

enum class GlContextProfile
{
	none,
	compatibility,
	core,
	es,
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_CONTEXT_PROFILE_INCLUDED
