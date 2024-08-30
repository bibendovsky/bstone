/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Error Utils

#ifndef BSTONE_GL_R3R_ERROR_INCLUDED
#define BSTONE_GL_R3R_ERROR_INCLUDED

namespace bstone {

struct GlR3rError
{
	// Enables checking for OpenGL errors.
	static void enable_checking(bool is_enable);

	// Clears OpenGL error flags and throws an exception if there were any error.
	static void ensure_no_errors();

	// Checks for OpenGL errors only if check is enabled.
	static void check_optionally();

	// Checks for OpenGL errors only if NDEBUG not defined.
	static void ensure_no_errors_assert();
};

} // namespace bstone

#endif // BSTONE_GL_R3R_ERROR_INCLUDED
