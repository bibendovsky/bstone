/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_string_view.h"

namespace bstone {

namespace detail {

namespace {

class StringViewException : public Exception
{
public:
	explicit StringViewException(const char* message) noexcept
		:
		Exception{"BSTONE_STRING_VIEW", message}
	{}

	~StringViewException() override = default;
};

} // namespace

[[noreturn]] void string_view_fail(const char* message)
{
	throw StringViewException{message};
}

} // namespace detail

} // namespace bstone
