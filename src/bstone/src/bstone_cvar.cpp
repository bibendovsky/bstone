/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <algorithm>
#include <exception>
#include <iterator>
#include <limits>
#include <utility>
#include "bstone_algorithm.h"
#include "bstone_char_conv.h"
#include "bstone_cvalidator.h"
#include "bstone_cvar.h"
#include "bstone_exception.h"
#include "bstone_span.h"

namespace bstone {

CVar::CVar(
	CVarInt32Tag,
	StringView name,
	CVarFlags flags,
	std::int32_t default_value,
	std::int32_t min_value,
	std::int32_t max_value)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		min_value,
		max_value,
		{}}
{}

CVar::CVar(
	CVarInt32Tag,
	StringView name,
	CVarFlags flags,
	std::int32_t default_value,
	CVarInt32Values values)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		std::numeric_limits<std::int32_t>::min(),
		std::numeric_limits<std::int32_t>::max(),
		values}
{}

CVar::CVar(CVarInt32Tag, StringView name, CVarFlags flags, std::int32_t default_value)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		std::numeric_limits<std::int32_t>::min(),
		std::numeric_limits<std::int32_t>::max(),
		{}}
{}

CVar::CVar(
	CVarStringTag,
	StringView name,
	CVarFlags flags,
	StringView default_value,
	CVarStringValues values)
try {
	CValidator::validate_name(name);

	if (!values.is_empty())
	{
		const auto values_end_iter = values.end();

		const auto found_value_iter = std::find_if(
			values.begin(),
			values_end_iter,
			[default_value](const StringView& value)
			{
				return default_value == value;
			});

		if (found_value_iter == values_end_iter)
		{
			BSTONE_THROW_STATIC_SOURCE("Default value out of range.");
		}
	}

	type_ = CVarType::string;
	name_ = name;
	flags_ = flags;

	int32_min_value_ = std::numeric_limits<std::int32_t>::min();
	int32_max_value_ = std::numeric_limits<std::int32_t>::max();
	int32_value_ = int32_default_value_;

	string_default_value_ = default_value;
	string_values_ = values;
	string_value_ = string_default_value_;
	set_int32_from_string();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVar::CVar(CVarBoolTag, StringView name, CVarFlags flags, bool default_value)
	:
	CVar{CVarInt32Tag{}, name, flags, default_value, 0, 1}
{}

CVar::CVar(CVarStringTag, StringView name, CVarFlags flags, StringView default_value)
	:
	CVar{CVarStringTag{}, name, flags, default_value, CVarStringValues{}}
{}

CVar::CVar(CVar&& rhs) noexcept
{
	swap(rhs);
}

CVar& CVar::operator=(CVar&& rhs) noexcept
{
	swap(rhs);
	return *this;
}

CVarType CVar::get_type() const noexcept
{
	return type_;
}

StringView CVar::get_name() const noexcept
{
	return name_;
}

CVarFlags CVar::get_flags() const noexcept
{
	return flags_;
}

bool CVar::get_bool() const noexcept
{
	return get_int32() != 0;
}

void CVar::set_bool(bool value)
{
	set_int32(value);
}

std::int32_t CVar::get_int32() const noexcept
{
	return int32_value_;
}

void CVar::set_int32(std::int32_t value)
try {
	switch (type_)
	{
		case CVarType::int32:
		case CVarType::string:
			break;

		default:
			fail_unknown_type();
	}

	if (int32_values_.is_empty())
	{
		value = clamp(value, int32_min_value_, int32_max_value_);
	}
	else
	{
		const auto int32_values_end_iter = int32_values_.end();
		const auto value_iter = std::find(int32_values_.begin(), int32_values_end_iter, value);

		if (value_iter == int32_values_end_iter)
		{
			value = int32_default_value_;
		}
	}

	if (int32_value_ == value)
	{
		return;
	}

	int32_value_ = value;
	set_string_from_int32();

	if (type_ == CVarType::string)
	{
		ensure_string();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVarInt32Values CVar::get_int32_values() const noexcept
{
	return int32_values_;
}

StringView CVar::get_string() const noexcept
{
	return string_value_.get();
}

void CVar::set_string(StringView value)
{
	if (string_value_.get() == value)
	{
		return;
	}

	string_value_ = value;
	ensure_string();
	set_int32_from_string();
}

CVarStringValues CVar::get_string_values() const noexcept
{
	return string_values_;
}

void CVar::swap(CVar& rhs)
{
	bstone::swop(type_, rhs.type_);
	name_.swap(rhs.name_);

	bstone::swop(int32_default_value_, rhs.int32_default_value_);
	bstone::swop(int32_min_value_, rhs.int32_min_value_);
	bstone::swop(int32_max_value_, rhs.int32_max_value_);
	bstone::swop(int32_value_, rhs.int32_value_);

	string_default_value_.swap(rhs.string_default_value_);
	string_values_.swap(rhs.string_values_);
	string_value_.swap(rhs.string_value_);
}

[[noreturn]] void CVar::fail_unknown_type()
{
	BSTONE_THROW_STATIC_SOURCE("Unknown type.");
}

CVar::CVar(
	CVarInt32Tag,
	StringView name,
	CVarFlags flags,
	std::int32_t default_value,
	std::int32_t min_value,
	std::int32_t max_value,
	CVarInt32Values values)
try {
	CValidator::validate_name(name);
	const auto value_count = values.get_size();

	if (value_count > 0)
	{
		auto last_value = *values.begin();
		auto found_default_value = (last_value == default_value);

		for (auto i = 1; i < value_count; ++i)
		{
			const auto& value = values.begin()[i];

			if (value <= last_value)
			{
				BSTONE_THROW_STATIC_SOURCE("Unordered or duplicate values.");
			}

			found_default_value |= (value == default_value);
		}

		if (!found_default_value)
		{
			BSTONE_THROW_STATIC_SOURCE("Default int32 value out of range.");
		}
	}
	else
	{
		if (min_value > max_value)
		{
			BSTONE_THROW_STATIC_SOURCE("Min int32 value out of range.");
		}

		if (default_value < min_value || default_value > max_value)
		{
			BSTONE_THROW_STATIC_SOURCE("Default int32 value out of range.");
		}
	}

	type_ = CVarType::int32;
	name_ = name;
	flags_ = flags;

	int32_default_value_ = default_value;
	int32_min_value_ = min_value;
	int32_max_value_ = max_value;
	int32_values_ = values;
	int32_value_ = int32_default_value_;
	set_string_from_int32();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void CVar::set_string_from_int32()
try
{
	char chars[max_int32_chars];
	const auto char_count = to_chars(int32_value_, std::begin(chars), std::end(chars)) - chars;
	string_value_ = StringView{chars, char_count};
}
catch (...)
{
	string_value_ = string_default_value_;
}

void CVar::set_int32_from_string()
try
{
	const auto string = string_value_.get();
	from_chars(string.cbegin(), string.cend(), int32_value_, 0);
}
catch (...)
{
	int32_value_ = int32_default_value_;
}

bool CVar::has_string(StringView string)
{
	const auto values_end_iter = string_values_.end();

	const auto found_value_iter = std::find_if(
		string_values_.begin(),
		values_end_iter,
		[string](const StringView& value)
		{
			return string == value;
		});

	return found_value_iter != values_end_iter;
}

void CVar::ensure_string()
{
	if (string_values_.is_empty())
	{
		return;
	}

	if (has_string(string_value_.get()))
	{
		return;
	}

	string_value_ = string_default_value_;
}

} // namespace bstone
