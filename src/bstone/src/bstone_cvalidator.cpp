/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ascii.h"
#include "bstone_cvalidator.h"
#include "bstone_exception.h"

namespace bstone {

void CValidator::validate_name(StringView name)
try {
	if (name.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty name.");
	}

	if (ascii::is_decimal(*name.begin()))
	{
		BSTONE_THROW_STATIC_SOURCE("Name starts with a decimal digit.");
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
			BSTONE_THROW_STATIC_SOURCE("Name character out of range.");
		}
	}

	if (!has_alpha_or_underscore)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected at least one underscore or alpha character for name.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
