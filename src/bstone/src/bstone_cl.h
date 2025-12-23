/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Command-line.

#ifndef BSTONE_CL_INCLUDED
#define BSTONE_CL_INCLUDED

#include <vector>
#include <string_view>
#include "bstone_span.h"

namespace bstone {

using ClArgs = Span<const std::string_view>;

struct ClOption
{
	std::string_view name{};
	ClArgs args{};
};

using ClOptions = Span<const ClOption>;

class Cl
{
public:
	Cl();

	std::string_view operator[](int index) const;

	void initialize(int argc, char* const* argv);

	ClOptions get_options() const noexcept;

	bool has_option(std::string_view option_name) const;

	ClOption find_option(std::string_view option_name) const;

	std::intptr_t get_count() const noexcept;
	std::string_view get_argument(std::intptr_t index) const;
	std::string_view get_option_value(std::string_view option_name) const;
	void get_option_values(std::string_view option_name, std::string_view& value1, std::string_view& value2) const;

private:
	using StringViews = std::vector<std::string_view>;
	using Options = std::vector<ClOption>;

	StringViews args_{};
	ClArgs params_{};
	Options options_{};
};

} // bstone

#endif // !BSTONE_CL_INCLUDED
