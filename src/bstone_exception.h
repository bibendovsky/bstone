/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Base exception.
//

#ifndef BSTONE_EXCEPTION_INCLUDED
#define BSTONE_EXCEPTION_INCLUDED

#include <deque>
#include <exception>
#include <memory>
#include <string>

namespace bstone {

class Exception : public std::exception
{
public:
	Exception(const char* file_name, int line, const char* function_name) noexcept;
	Exception(const char* context, const char* message) noexcept;
	Exception(const Exception& rhs) noexcept;
	~Exception() override;

	const char* what() const noexcept override;

private:
	using What = std::unique_ptr<char[]>;

	What what_{};
};

// ==========================================================================

class StaticException : public std::exception
{
public:
	StaticException(
		const char* file_name,
		int line,
		const char* function_name) noexcept;

	StaticException(
		const char* file_name,
		int line,
		const char* function_name,
		const char* message) noexcept;

	~StaticException() override = default;

	const char* get_file_name() const noexcept;
	int get_line() const noexcept;
	const char* get_function_name() const noexcept;
	const char* get_message() const noexcept;

	const char* what() const noexcept override;

private:
	const char* file_name_{};
	int line_{};
	const char* function_name_{};
	const char* message_{};
};

// --------------------------------------------------------------------------

[[noreturn]] void static_fail(
	const char* file_name,
	int line,
	const char* function_name,
	const char* message);

[[noreturn]] void static_fail_nested(
	const char* file_name,
	int line,
	const char* function_name);

#if !defined(BSTONE_STATIC_THROW)
#define BSTONE_STATIC_THROW(message) static_fail(__FILE__, __LINE__, __func__, message)
#endif

#if !defined(BSTONE_FUNC_STATIC_THROW_NESTED)
#define BSTONE_FUNC_STATIC_THROW_NESTED catch (...) { static_fail_nested(__FILE__, __LINE__, __func__); }
#endif

// ==========================================================================

using ExceptionMessages = std::deque<std::string>;

ExceptionMessages extract_exception_messages();

std::string get_nested_message();

} // bstone


#endif // !BSTONE_EXCEPTION_INCLUDED
