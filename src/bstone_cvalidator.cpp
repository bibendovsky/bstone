/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ascii.h"
#include "bstone_cvalidator.h"
#include "bstone_exception.h"

namespace bstone {

class CValidatorException : public Exception
{
public:
	explicit CValidatorException(const char* message) noexcept
		:
		Exception{"BSTONE_CVALIDATOR", message}
	{}
};

// ==========================================================================

void CValidator::validate_name(StringView name)
try
{
	if (name.is_empty())
	{
		fail("Empty name.");
	}

	if (ascii::is_decimal(*name.begin()))
	{
		fail("Name starts with a decimal digit.");
	}

	auto has_alpha_or_underscore = false;

	for (const auto& ch : name)
	{
		if (ascii::is_lower(ch) || ascii::is_upper(ch) || ch == '_')
		{
			has_alpha_or_underscore = true;
		}
		else if (ascii::is_decimal(ch))
		{
		}
		else
		{
			fail("Name character out of range.");
		}
	}

	if (!has_alpha_or_underscore)
	{
		fail("Expected at least one underscore or alpha character for name.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void CValidator::fail(const char* message)
{
	throw CValidatorException{message};
}

[[noreturn]] void CValidator::fail_nested(const char* message)
{
	std::throw_with_nested(CValidatorException{message});
}
} // namespace bstone
