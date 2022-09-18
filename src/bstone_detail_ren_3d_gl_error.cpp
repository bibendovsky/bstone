/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL error manager (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_detail_ren_3d_gl_error.h"

#include <cassert>

#include <iomanip>
#include <string>
#include <sstream>

#include "bstone_exception.h"

#include "bstone_detail_ren_3d_gl_api.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// GlErrorException
//

class GlErrorException :
	public Exception
{
public:
	explicit GlErrorException(
		const char* message) noexcept
		:
		Exception{"GL_ERROR", message}
	{
	}
}; // GlErrorException

//
// GlErrorException
// ==========================================================================


[[noreturn]]
void fail(
	const char* message)
{
	throw GlErrorException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(GlErrorException{message});
}


const char* gl_error_code_get_name(
	GLenum gl_error_code) noexcept
{
	switch (gl_error_code)
	{
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";

		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";

		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";

		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";

		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";

		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";

		case GL_CONTEXT_LOST:
			return "GL_CONTEXT_LOST";

		default:
			return "???";
	}
}

std::string gl_error_code_get_code_string(
	GLenum gl_error_code)
{
	auto oss = std::ostringstream{};

	oss <<
		"0x" <<
		std::setw(4) <<
		std::setfill('0') <<
		std::hex <<
		static_cast<int>(gl_error_code)
	;

	return oss.str();
}

std::string gl_error_code_get_message(
	GLenum gl_error_code)
{
	return
		std::string{} +
		gl_error_code_get_name(gl_error_code) +
		" (" +
		gl_error_code_get_code_string(gl_error_code) +
		")."
	;
}


// ==========================================================================
// Ren3dGlError
//

void Ren3dGlError::ensure()
try
{
	if (!glGetError)
	{
		fail("Null \"glGetError\".");
	}

	const auto gl_error_code = glGetError();

	if (gl_error_code != GL_NO_ERROR)
	{
		const auto message = gl_error_code_get_message(gl_error_code);
		fail(message.c_str());
	}
}
catch (...)
{
	fail_nested(__func__);
}

#ifdef NDEBUG
void Ren3dGlError::ensure_debug()
{
}
#else
void Ren3dGlError::ensure_debug()
try
{
	ensure();
}
catch (...)
{
	fail_nested(__func__);
}
#endif // NDEBUG

void Ren3dGlError::ensure_assert() noexcept
{
	assert(glGetError);

#ifndef NDEBUG
	const auto error_code = glGetError();
#endif // !NDEBUG

	assert(error_code == GL_NO_ERROR);
}

//
// Ren3dGlError
// ==========================================================================


}
} // bstone
