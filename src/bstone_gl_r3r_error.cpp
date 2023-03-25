/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Error Utils

#include <cassert>
#include <string>
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_error.h"

namespace bstone {

namespace {

const char* gl_error_code_get_name(GLenum gl_error_code) noexcept
{
	switch (gl_error_code)
	{
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
		default: return "GL_???";
	}
}

void gl_error_code_append(GLenum gl_code, std::string& chars)
{
	constexpr auto max_number_chars = 8;
	char number_chars[max_number_chars];

	if (!chars.empty())
	{
		chars += ", ";
	}

	chars += gl_error_code_get_name(gl_code);
	chars += " (0x";

	const auto number_char_count = char_conv::to_chars(
		gl_code,
		make_span(number_chars),
		16,
		char_conv::ToCharsFormat::uppercase_value);

	chars.append(number_chars, static_cast<std::size_t>(number_char_count));
	chars += ')';
}

} // namespace

// ==========================================================================

void GlR3rError::ensure()
try
{
	if (glGetError == nullptr)
	{
		BSTONE_STATIC_THROW("Null \"glGetError\".");
	}

	auto message = std::string{};

	for (auto i = 0; i < 32; ++i)
	{
		const auto gl_error_code = glGetError();

		if (gl_error_code == GL_NO_ERROR)
		{
			break;
		}
		else
		{
			if (i == 0)
			{
				message.reserve(2048);
			}

			gl_error_code_append(gl_error_code, message);
		}
	}

	if (!message.empty())
	{
		BSTONE_STATIC_THROW(message.c_str());
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

#ifdef NDEBUG
void GlR3rError::ensure_debug()
{}
#else
void GlR3rError::ensure_debug()
try
{
	ensure();
}
BSTONE_STATIC_THROW_NESTED_FUNC
#endif // NDEBUG

void GlR3rError::ensure_assert()
{
	assert(glGetError != nullptr);

	auto was_any_error = false;

	for (auto i = 0; i < 32; ++i)
	{
		const auto error_code = glGetError();

		if (error_code == GL_NO_ERROR)
		{
			break;
		}

		was_any_error = true;
	}

	assert(!was_any_error);
}

} // namespace bstone
