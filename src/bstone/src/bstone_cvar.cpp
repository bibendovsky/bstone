/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2022-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_cvalidator.h"
#include "bstone_cvar.h"
#include "bstone_exception.h"
#include <cmath>
#include <algorithm>
#include <charconv>
#include <exception>
#include <iterator>
#include <limits>
#include <utility>

namespace bstone {

CVar::CVar(
	CVarInt32Tag,
	std::string_view name,
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
	std::string_view name,
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

CVar::CVar(CVarInt32Tag, std::string_view name, CVarFlags flags, std::int32_t default_value)
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
	CVarFloat32Tag,
	std::string_view name,
	CVarFlags flags,
	float default_value,
	float min_value,
	float max_value)
try
{
	CValidator::validate_name(name);
	CValidator::validate_float32_category(default_value);
	CValidator::validate_float32_category(min_value);
	CValidator::validate_float32_category(max_value);
	if (min_value > max_value)
	{
		BSTONE_THROW_STATIC_SOURCE("Min float32 value is greater than max one.");
	}
	if (default_value < min_value || default_value > max_value)
	{
		BSTONE_THROW_STATIC_SOURCE("Default float32 value is out of bounds.");
	}
	type_ = CVarType::float32;
	name_ = name;
	flags_ = flags;
	int32_value_ = static_cast<std::int32_t>(default_value);
	float32_default_value_ = default_value;
	float32_min_value_ = min_value;
	float32_max_value_ = max_value;
	float32_value_ = default_value;
	set_string_from_float32();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVar::CVar(
	CVarStringTag,
	std::string_view name,
	CVarFlags flags,
	std::string_view default_value,
	CVarStringValues values)
try
{
	CValidator::validate_name(name);
	if (!values.empty() && !has_string(default_value, values))
	{
		BSTONE_THROW_STATIC_SOURCE("The default string is not a part of a string list.");
	}
	type_ = CVarType::string;
	name_ = name;
	flags_ = flags;
	string_default_value_ = default_value;
	string_values_ = values;
	string_value_ = string_default_value_;
	set_int32_from_string();
	set_float32_from_string();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVar::CVar(CVarBoolTag, std::string_view name, CVarFlags flags, bool default_value)
	:
	CVar{CVarInt32Tag{}, name, flags, default_value, 0, 1}
{}

CVar::CVar(CVarStringTag, std::string_view name, CVarFlags flags, std::string_view default_value)
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

CVarType CVar::get_type() const
{
	return type_;
}

std::string_view CVar::get_name() const
{
	return name_;
}

CVarFlags CVar::get_flags() const
{
	return flags_;
}

bool CVar::get_bool() const
{
	return get_int32() != 0;
}

void CVar::set_bool(bool value)
{
	set_int32(value);
}

std::int32_t CVar::get_int32() const
{
	return int32_value_;
}

void CVar::set_int32(std::int32_t value)
try
{
	if (type_ != CVarType::int32)
	{
		fail_unknown_type();
	}
	if (int32_values_.empty())
	{
		value = std::clamp(value, int32_min_value_, int32_max_value_);
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
	float32_value_ = static_cast<float>(value);
	set_string_from_int32();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVarInt32Values CVar::get_int32_values() const
{
	return int32_values_;
}

float CVar::get_float32() const
{
	return float32_value_;
}

void CVar::set_float32(float value)
try
{
	if (type_ != CVarType::float32)
	{
		fail_unknown_type();
	}
	switch (std::fpclassify(value))
	{
		case FP_NORMAL:
		case FP_ZERO:
			value = std::clamp(value, float32_min_value_, float32_max_value_);
			break;
		default:
			value = float32_default_value_;
			break;
	}
	if (value == float32_value_)
	{
		return;
	}
	int32_value_ = static_cast<std::int32_t>(value);
	float32_value_ = value;
	set_string_from_float32();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string_view CVar::get_string() const
{
	return string_value_.get();
}

void CVar::set_string(std::string_view value)
try
{
	switch (type_)
	{
		case CVarType::int32:
			if (const OptionalInt32 optional_int32 = parse_int32(value);
				optional_int32.has_value())
			{
				set_int32(optional_int32.value());
			}
			else
			{
				set_int32(int32_default_value_);
			}
			break;
		case CVarType::float32:
			if (const OptionalFloat32 optional_float32 = parse_float32(value);
				optional_float32.has_value())
			{
				set_float32(optional_float32.value());
			}
			else
			{
				set_float32(float32_default_value_);
			}
			break;
		case CVarType::string:
			if (string_values_.empty())
			{}
			else if (!has_string(value))
			{
				value = string_default_value_;
			}
			if (string_value_.get() != value)
			{
				string_value_ = value;
				set_int32_from_string();
				set_float32_from_string();
			}
			break;
		default:
			fail_unknown_type();
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

CVarStringValues CVar::get_string_values() const
{
	return string_values_;
}

void CVar::swap(CVar& rhs)
{
	std::swap(type_, rhs.type_);
	name_.swap(rhs.name_);

	std::swap(int32_default_value_, rhs.int32_default_value_);
	std::swap(int32_min_value_, rhs.int32_min_value_);
	std::swap(int32_max_value_, rhs.int32_max_value_);
	std::swap(int32_value_, rhs.int32_value_);

	std::swap(float32_default_value_, rhs.float32_default_value_);
	std::swap(float32_min_value_, rhs.float32_min_value_);
	std::swap(float32_max_value_, rhs.float32_max_value_);
	std::swap(float32_value_, rhs.float32_value_);

	string_default_value_.swap(rhs.string_default_value_);
	std::swap(string_values_, rhs.string_values_);
	string_value_.swap(rhs.string_value_);
}

[[noreturn]] void CVar::fail_unknown_type()
{
	BSTONE_THROW_STATIC_SOURCE("Unknown type.");
}

CVar::CVar(
	CVarInt32Tag,
	std::string_view name,
	CVarFlags flags,
	std::int32_t default_value,
	std::int32_t min_value,
	std::int32_t max_value,
	CVarInt32Values values)
try
{
	CValidator::validate_name(name);
	const std::size_t value_count = values.size();
	if (value_count > 0)
	{
		const std::int32_t last_value = *values.begin();
		bool found_default_value = (last_value == default_value);
		for (std::size_t i = 1; i < value_count; ++i)
		{
			const std::int32_t& value = values.begin()[i];
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
	float32_value_ = static_cast<float>(int32_value_);
	set_string_from_int32();
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

auto CVar::parse_int32(std::string_view string) -> OptionalInt32
{
	std::int32_t value;
	if (const auto [string_end, ec] = std::from_chars(string.data(), string.data() + string.size(), value);
		ec != std::errc{})
	{
		return OptionalInt32{};
	}
	return OptionalInt32{value};
}

auto CVar::parse_float32(std::string_view string) -> OptionalFloat32
{
	float value;
	if (const auto [string_end, ec] = std::from_chars(string.data(), string.data() + string.size(), value);
		ec != std::errc{})
	{
		return OptionalFloat32{};
	}
	return OptionalFloat32{value};
}

void CVar::set_string_from_int32()
{
	char chars[max_int32_chars];
	if (const auto [chars_end, ec] = std::to_chars(std::begin(chars), std::end(chars), int32_value_);
		ec == std::errc{})
	{
		const std::intptr_t char_count = chars_end - chars;
		string_value_ = std::string_view{chars, static_cast<std::size_t>(char_count)};
	}
	else
	{
		string_value_ = string_default_value_;
	}
}

void CVar::set_int32_from_string()
{
	const std::string_view string = string_value_.get();
	if (const auto [string_end, ec] = std::from_chars(string.data(), string.data() + string.size(), int32_value_);
		ec != std::errc{})
	{
		int32_value_ = int32_default_value_;
	}
}

void CVar::set_float32_from_string()
{
	const std::string_view string = string_value_.get();
	if (const auto [string_end, ec] = std::from_chars(string.data(), string.data() + string.size(), float32_value_);
		ec != std::errc{})
	{
		float32_value_ = float32_default_value_;
	}
}

void CVar::set_string_from_float32()
try
{
	char chars[max_float32_chars];
	if (const auto [chars_end, ec] = std::to_chars(std::begin(chars), std::end(chars), float32_value_);
		ec == std::errc{})
	{
		const std::intptr_t char_count = chars_end - chars;
		string_value_ = std::string_view{chars, static_cast<std::size_t>(char_count)};
	}
	else
	{
		string_value_ = string_default_value_;
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool CVar::has_string(std::string_view string, CVarStringValues string_values)
{
	return std::any_of(
		std::cbegin(string_values),
		std::cend(string_values),
		[string](std::string_view value){return string == value;});
}

bool CVar::has_string(std::string_view string) const
{
	return has_string(string, string_values_);
}

void CVar::ensure_string()
{
	if (string_values_.empty())
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
