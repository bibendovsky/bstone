/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// OpenGL error manager (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_detail_ren_3d_gl_error.h"

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
// GlErrorCodeException
//

class GlErrorNullException :
	public Exception
{
public:
	explicit GlErrorNullException()
		:
		Exception{"[GL_ERR] Null \"glGetError\"."}
	{
	}
}; // GlErrorCodeException

//
// GlErrorCodeException
// ==========================================================================


// ==========================================================================
// GlErrorCodeException
//

class GlErrorCodeException :
	public Exception
{
public:
	explicit GlErrorCodeException(
		const GLenum gl_error_code)
		:
		Exception{get_message(gl_error_code)}
	{
	}


private:
	static const char* get_gl_error_code_name_string(
		const GLenum gl_error_code) noexcept
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

	static std::string get_gl_enum_number_string(
		const GLenum gl_error_code)
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

	static std::string get_message(
		const GLenum gl_error_code)
	{
		return
			std::string{"[GL_ERR] "} +
			get_gl_error_code_name_string(gl_error_code) +
			" (" +
			get_gl_enum_number_string(gl_error_code) +
			")."
		;
	}
}; // GlErrorCodeException

//
// GlErrorCodeException
// ==========================================================================


// ==========================================================================
// Ren3dGlError
//

void Ren3dGlError::ensure()
{
	if (!glGetError)
	{
		throw GlErrorNullException{};
	}

	const auto gl_error_code = glGetError();

	if (gl_error_code != GL_NO_ERROR)
	{
		throw GlErrorCodeException{gl_error_code};
	}
}

void Ren3dGlError::ensure_debug()
{
#if _DEBUG
	ensure();
#endif // _DEBUG
}

//
// Ren3dGlError
// ==========================================================================


}
} // bstone
