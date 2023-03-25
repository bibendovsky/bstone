/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Error Utils

#if !defined(BSTONE_GL_R3R_ERROR_INCLUDED)
#define BSTONE_GL_R3R_ERROR_INCLUDED

namespace bstone {

struct GlR3rError
{
	// Throws an exception on OpenGL error for any build.
	// Also clears all error flags.
	static void ensure();

	// Throws an exception on OpenGL error for debug build.
	// Also clears all error flags.
	static void ensure_debug();

	// Asserts if there was any OpenGL error.
	// Also clears all error flags.
	static void ensure_assert();
};

} // namespace bstone

#endif // BSTONE_GL_R3R_ERROR_INCLUDED
