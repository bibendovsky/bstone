/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Command-line.

#ifndef BSTONE_CL_INCLUDED
#define BSTONE_CL_INCLUDED

#include <vector>
#include "bstone_span.h"
#include "bstone_string_view.h"

namespace bstone {

using ClArgs = Span<const StringView>;

struct ClOption
{
	StringView name{};
	ClArgs args{};
};

using ClOptions = Span<const ClOption>;

class Cl
{
public:
	Cl();

	StringView operator[](int index) const;

	void initialize(int argc, char* const* argv);

	ClOptions get_options() const noexcept;

	bool has_option(StringView option_name) const;

	ClOption find_option(StringView option_name) const;

	std::intptr_t get_count() const noexcept;
	StringView get_argument(std::intptr_t index) const;
	StringView get_option_value(StringView option_name) const;
	void get_option_values(StringView option_name, StringView& value1, StringView& value2) const;

private:
	using StringViews = std::vector<StringView>;
	using Options = std::vector<ClOption>;

	StringViews args_{};
	ClArgs params_{};
	Options options_{};
};

} // bstone

#endif // !BSTONE_CL_INCLUDED
