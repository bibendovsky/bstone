/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ascii.h"
#include "bstone_cvalidator.h"
#include "bstone_exception.h"
#include <cmath>

namespace bstone {

void CValidator::validate_name(std::string_view name)
try {
	if (name.empty())
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

void CValidator::validate_float32_category(float value)
try
{
	switch (std::fpclassify(value))
	{
		case FP_NORMAL:
		case FP_ZERO:
			break;
		case FP_SUBNORMAL:
			BSTONE_THROW_STATIC_SOURCE("Subnormal float32.");
		case FP_INFINITE:
			BSTONE_THROW_STATIC_SOURCE("Infinite float32.");
		case FP_NAN:
			BSTONE_THROW_STATIC_SOURCE("NaN float32.");
		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported float32 category.");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
