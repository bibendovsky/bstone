/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CL_ARGS_INCLUDED
#define BSTONE_CL_ARGS_INCLUDED

#include <vector>
#include "bstone_int.h"
#include "bstone_span.h"
#include "bstone_string_view.h"

namespace bstone {

using ClArgsSpan = Span<const StringView>;

struct ClArgsOption
{
	StringView name{};
	ClArgsSpan args{};
};

class ClArgs
{
public:
	ClArgs();

	StringView operator[](int index) const;

	void initialize(int argc, char* const* argv);

	bool has_option(StringView option_name) const;
	bool has_option(const char* option_name) const;

	Int find_option(StringView option_name) const;

	Int get_count() const noexcept;
	StringView get_argument(Int index) const;
	StringView get_option_value(StringView option_name) const;
	StringView get_option_value(const char* option_name) const;
	void get_option_values(StringView option_name, StringView& value1, StringView& value2) const;

private:
	using StringViews = std::vector<StringView>;

	StringViews args_{};

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
}; // ClArgs

} // bstone

#endif // !BSTONE_CL_ARGS_INCLUDED
