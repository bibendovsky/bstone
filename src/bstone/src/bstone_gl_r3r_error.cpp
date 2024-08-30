/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Error Utils

#include "bstone_gl_r3r_api.h"

#include <iterator>
#include <string>

#include "bstone_ascii.h"
#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_gl_r3r_error.h"

namespace bstone {

namespace {

class GlR3rErrorImpl
{
public:
	void enable_checking(bool is_enable) noexcept;
	void check_optionally();
	void ensure_no_errors();

private:
	bool is_checking_enabled_{};

private:
	static const char* get_code_name(GLenum gl_error_code) noexcept;
	static void append_code(GLenum gl_code, std::string& chars);
	void ensure_no_errors_internal();
};

// --------------------------------------------------------------------------

void GlR3rErrorImpl::enable_checking(bool is_enable) noexcept
{
	is_checking_enabled_ = is_enable;
}

void GlR3rErrorImpl::check_optionally()
try {
	if (!is_checking_enabled_)
	{
		return;
	}

	ensure_no_errors_internal();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rErrorImpl::ensure_no_errors()
try {
	ensure_no_errors_internal();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const char* GlR3rErrorImpl::get_code_name(GLenum gl_error_code) noexcept
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
		default: return "GL_UNKNOWN";
	}
}

void GlR3rErrorImpl::append_code(GLenum gl_code, std::string& chars)
{
	constexpr auto max_number_chars = 8;
	char number_chars[max_number_chars];

	if (!chars.empty())
	{
		chars += ", ";
	}

	chars += get_code_name(gl_code);
	chars += " (0x";

	const auto number_chars_first = std::begin(number_chars);
	const auto number_chars_last = to_chars(gl_code, number_chars_first, std::end(number_chars), 16);
	ascii::to_upper(number_chars_first, number_chars_last);
	chars.append(number_chars_first, number_chars_last);
	chars += ')';
}

void GlR3rErrorImpl::ensure_no_errors_internal()
{
	if (glGetError == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetError\".");
	}

	constexpr auto max_errors = 32;

	GLenum error_codes[max_errors + 1] = {};
	auto error_count = 0;

	for (auto i = 0; i <= max_errors; ++i)
	{
		const auto gl_error_code = glGetError();

		if (gl_error_code == GL_NO_ERROR)
		{
			break;
		}
		else
		{
			error_codes[error_count++] = gl_error_code;
		}
	}

	if (error_count == 0)
	{
		return;
	}

	auto message = std::string{};
	message.reserve(2048);

	for (auto i = 0; i < error_count; ++i)
	{
		append_code(error_codes[i], message);
	}

	if (error_count > max_errors)
	{
		message += " (too many errors)";
	}

	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

// ==========================================================================

GlR3rErrorImpl gl_r3r_error_impl{};

} // namespace

// ==========================================================================

void GlR3rError::enable_checking(bool is_enable)
{
	gl_r3r_error_impl.enable_checking(is_enable);
}

void GlR3rError::ensure_no_errors()
{
	gl_r3r_error_impl.ensure_no_errors();
}

void GlR3rError::check_optionally()
{
	gl_r3r_error_impl.check_optionally();
}

void GlR3rError::ensure_no_errors_assert()
{
#ifndef NDEBUG
	BSTONE_ASSERT(glGetError != nullptr);

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

	BSTONE_ASSERT(!was_any_error);
#endif
}

} // namespace bstone
