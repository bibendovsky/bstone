/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated string view.

#include "bstone_exception.h"
#include "bstone_zstring_view.h"

namespace bstone {

namespace {

class ZStringViewException : public Exception
{
public:
	explicit ZStringViewException(const char* message)
		:
		Exception{"BSTONE_ZSTRING_VIEW", message}
	{}

	~ZStringViewException() override = default;
};

} // namespace

namespace detail {

[[noreturn]] void zstring_view_fail(const char* message)
{
	throw ZStringViewException{message};
}

[[noreturn]] void zstring_view_fail_non_null_terminated()
{
	zstring_view_fail("Non-null-terminated string.");
}

} // namespace detail

} // namespace bstone
