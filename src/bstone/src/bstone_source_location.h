/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Information about the source code: file name, line number, function name.

#ifndef BSTONE_SOURCE_LOCATION_INCLUDED
#define BSTONE_SOURCE_LOCATION_INCLUDED

#include "bstone_utility.h"

namespace bstone {

class SourceLocation
{
public:
	constexpr SourceLocation() = default;
	SourceLocation(const char* file_name, int line, const char* function_name);

	constexpr const char* get_file_name() const noexcept;
	constexpr int get_line() const noexcept;
	constexpr const char* get_function_name() const noexcept;

	constexpr void swap(SourceLocation& rhs) noexcept;

private:
	int line_{};
	const char* file_name_{""};
	const char* function_name_{""};
};

// --------------------------------------------------------------------------

constexpr const char* SourceLocation::get_file_name() const noexcept
{
	return file_name_;
}

constexpr int SourceLocation::get_line() const noexcept
{
	return line_;
}

constexpr const char* SourceLocation::get_function_name() const noexcept
{
	return function_name_;
}

constexpr void SourceLocation::swap(SourceLocation& rhs) noexcept
{
	bstone::swop(line_, rhs.line_);
	bstone::swop(file_name_, rhs.file_name_);
	bstone::swop(function_name_, rhs.function_name_);
}

// ==========================================================================

#ifndef BSTONE_MAKE_SOURCE_LOCATION
#define BSTONE_MAKE_SOURCE_LOCATION() ::bstone::SourceLocation{__FILE__, __LINE__, __func__}
#endif

} // namespace bstone

#endif // BSTONE_SOURCE_LOCATION_INCLUDED
