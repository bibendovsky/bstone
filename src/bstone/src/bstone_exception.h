/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Exception classes.

#ifndef BSTONE_EXCEPTION_INCLUDED
#define BSTONE_EXCEPTION_INCLUDED

#include <exception>
#include <source_location>
#include <type_traits>

namespace bstone {

class Exception : public std::exception
{
public:
	~Exception() override;
};

// ==========================================================================

class SourceException : public Exception
{
public:
	~SourceException() override;

	virtual const std::source_location& get_source_location() const noexcept = 0;
};

// ==========================================================================

class StaticSourceException : public SourceException
{
public:
	StaticSourceException(const std::source_location& source_location, const char* message);
	explicit StaticSourceException(const std::source_location& source_location) noexcept;
	StaticSourceException(const StaticSourceException& rhs) noexcept;
	StaticSourceException& operator=(const StaticSourceException& rhs) noexcept;
	~StaticSourceException() override;

	const std::source_location& get_source_location() const noexcept override;
	const char* what() const noexcept override;

	void swap(StaticSourceException& rhs) noexcept;

	[[noreturn]] static void fail(const std::source_location& source_location, const char* message);
	[[noreturn]] static void fail_nested(const std::source_location& source_location);

private:
	std::source_location source_location_{};
	const char* message_{""};
};

// ==========================================================================

class DynamicSourceException : public SourceException
{
public:
	DynamicSourceException(const std::source_location& source_location, const char* message);
	DynamicSourceException(const DynamicSourceException& rhs);
	DynamicSourceException& operator=(const DynamicSourceException& rhs);
	~DynamicSourceException() override;

	const std::source_location& get_source_location() const noexcept override;
	const char* what() const noexcept override;

	void swap(DynamicSourceException& rhs) noexcept;

	[[noreturn]] static void fail(const std::source_location& source_location, const char* message);

private:
	using Counter = int;

	struct ControlBlock
	{
		Counter counter;
		char* message;
	};

	static_assert(std::is_trivial<ControlBlock>::value, "Expected a trivial type.");

private:
	std::source_location source_location_{};
	ControlBlock* control_block_{}; // {ControlBlock}{null-terminated message}
};

// ==========================================================================

#ifndef BSTONE_MAKE_SOURCE_LOCATION
	#define BSTONE_MAKE_SOURCE_LOCATION() std::source_location::current()
#endif
// ==========================================================================

#ifndef BSTONE_THROW_STATIC_SOURCE
	#define BSTONE_THROW_STATIC_SOURCE(message) \
	::bstone::StaticSourceException::fail(BSTONE_MAKE_SOURCE_LOCATION(), message)
#endif

#ifndef BSTONE_THROW_DYNAMIC_SOURCE
	#define BSTONE_THROW_DYNAMIC_SOURCE(message) \
	::bstone::DynamicSourceException::fail(BSTONE_MAKE_SOURCE_LOCATION(), message)
#endif

#ifndef BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
	#define BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED \
	catch (...) { ::bstone::StaticSourceException::fail_nested(BSTONE_MAKE_SOURCE_LOCATION()); }
#endif

} // namespace bstone

#endif // BSTONE_EXCEPTION_INCLUDED
