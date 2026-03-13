/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Error Utils

#include "bstone_gl_r3r_error.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_string_builder.h"
#include "bstone_gl_r3r_api.h"

namespace bstone {

namespace {

class GlR3rErrorImpl
{
public:
	static void enable_checking(bool is_enable);
	static void check_optionally();
	static void ensure_no_errors();

private:
	static bool is_checking_enabled_;

private:
	static const char* get_code_name(GLenum gl_error_code);
	static void append_code(GLenum gl_code, StringBuilder& string_builder);
	static void ensure_no_errors_internal();
};

// -------------------------------------

bool GlR3rErrorImpl::is_checking_enabled_{};

// -------------------------------------

void GlR3rErrorImpl::enable_checking(bool is_enable)
{
	is_checking_enabled_ = is_enable;
}

void GlR3rErrorImpl::check_optionally()
{
	if (!is_checking_enabled_)
	{
		return;
	}
	ensure_no_errors_internal();
}

void GlR3rErrorImpl::ensure_no_errors()
{
	ensure_no_errors_internal();
}

const char* GlR3rErrorImpl::get_code_name(GLenum gl_error_code)
{
#define BSTONE_MACRO(x) case x: return #x
	switch (gl_error_code)
	{
		BSTONE_MACRO(GL_INVALID_ENUM);
		BSTONE_MACRO(GL_INVALID_VALUE);
		BSTONE_MACRO(GL_INVALID_OPERATION);
		BSTONE_MACRO(GL_INVALID_FRAMEBUFFER_OPERATION);
		BSTONE_MACRO(GL_OUT_OF_MEMORY);
		BSTONE_MACRO(GL_STACK_UNDERFLOW);
		BSTONE_MACRO(GL_STACK_OVERFLOW);
		default: return "GL_UNKNOWN";
	}
#undef BSTONE_MACRO
}

void GlR3rErrorImpl::append_code(GLenum gl_code, StringBuilder& string_builder)
{
	if (!string_builder.is_empty())
	{
		string_builder.add(", ");
	}
	string_builder.add(get_code_name(gl_code));
	string_builder.add(" (0x{:04X})", gl_code);
}

void GlR3rErrorImpl::ensure_no_errors_internal()
{
	if (glGetError == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetError\".");
	}
	constexpr int max_errors = 32;
	GLenum error_codes[max_errors + 1];
	int error_count = 0;
	for (int i = 0; i <= max_errors; ++i)
	{
		const GLenum gl_error_code = glGetError();
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
	StringBuilder string_builder{};
	string_builder.reserve(2048);
	for (int i = 0; i < error_count; ++i)
	{
		append_code(error_codes[i], string_builder);
	}
	if (error_count > max_errors)
	{
		string_builder.add(" (too many errors)");
	}
	BSTONE_THROW_DYNAMIC_SOURCE(string_builder.get_string().c_str());
}

} // namespace

// =======================================

void GlR3rError::enable_checking(bool is_enable)
{
	GlR3rErrorImpl::enable_checking(is_enable);
}

void GlR3rError::ensure_no_errors()
try
{
	GlR3rErrorImpl::ensure_no_errors();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rError::check_optionally()
try
{
	GlR3rErrorImpl::check_optionally();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rError::ensure_no_errors_assert()
{
#ifndef NDEBUG
	BSTONE_ASSERT(glGetError != nullptr);
	bool was_any_error = false;
	for (int i = 0; i < 32; ++i)
	{
		const GLenum error_code = glGetError();
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
