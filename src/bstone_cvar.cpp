/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <algorithm>
#include <exception>
#include <limits>
#include <utility>
#include "bstone_algorithm.h"
#include "bstone_ascii.h"
#include "bstone_char_conv.h"
#include "bstone_cvar.h"
#include "bstone_exception.h"
#include "bstone_span.h"

namespace bstone {

namespace {

class CVarException : public Exception
{
public:
	explicit CVarException(const char* message)
		:
		Exception{"BSTONE_CVAR", message}
	{}

	~CVarException() override = default;
};

} // namespace

CVar::CVar(
	CVarInt32Tag,
	StringView name,
	CVarFlags flags,
	Int32 default_value,
	Int32 min_value,
	Int32 max_value)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		std::numeric_limits<Int32>::min(),
		std::numeric_limits<Int32>::max(),
		{}}
{}

CVar::CVar(CVarInt32Tag, StringView name, CVarFlags flags, Int32 default_value, std::initializer_list<Int32> values)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		std::numeric_limits<Int32>::min(),
		std::numeric_limits<Int32>::max(),
		values}
{}

CVar::CVar(CVarInt32Tag, StringView name, CVarFlags flags, Int32 default_value)
	:
	CVar{
		CVarInt32Tag{},
		name,
		flags,
		default_value,
		std::numeric_limits<Int32>::min(),
		std::numeric_limits<Int32>::max(),
		{}}
{}

CVar::CVar(
	CVarStringTag,
	StringView name,
	CVarFlags flags,
	StringView default_value,
	std::initializer_list<StringView> values)
try
{
	validate_name(name);

	if (values.size() > 0)
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
			fail("Default value out of range.");
		}
	}

	type_ = CVarType::string;
	name_ = name;
	int32_min_value_ = std::numeric_limits<Int32>::min();
	int32_max_value_ = std::numeric_limits<Int32>::max();
	int32_value_ = int32_default_value_;

	string_default_value_ = default_value;
	string_values_ = values;
	string_value_ = string_default_value_;
	set_int32_from_string();
}
catch (...)
{
	fail_nested(__func__);
}

CVar::CVar(CVarBoolTag, StringView name, CVarFlags flags, bool default_value)
	:
	CVar{CVarInt32Tag{}, name, flags, default_value, 0, 1}
{}

CVar::CVar(CVarStringTag, StringView name, CVarFlags flags, StringView default_value)
	:
	CVar{CVarStringTag{}, name, flags, default_value, std::initializer_list<StringView>{}}
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

Int32 CVar::get_int32() const noexcept
{
	return int32_value_;
}

void CVar::set_int32(Int32 value)
try
{
	switch (type_)
	{
		case CVarType::int32:
		case CVarType::string:
			break;

		default:
			fail_unknown_type();
	}

	if (int32_values_.empty())
	{
		value = algorithm::clamp(value, int32_min_value_, int32_max_value_);
	}
	else
	{
		const auto int32_values_end_iter = int32_values_.cend();
		const auto value_iter = std::find(int32_values_.cbegin(), int32_values_end_iter, value);

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
}
catch (...)
{
	fail_nested(__func__);
}

CVarInt32Values CVar::get_int32_values() const noexcept
{
	return CVarInt32Values{int32_values_.data(), static_cast<Int>(int32_values_.size())};
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
	return CVarStringValues{string_values_.data(), static_cast<Int>(string_values_.size())};
}

void CVar::swap(CVar& rhs)
{
	utility::swap(type_, rhs.type_);
	name_.swap(rhs.name_);

	utility::swap(int32_default_value_, rhs.int32_default_value_);
	utility::swap(int32_min_value_, rhs.int32_min_value_);
	utility::swap(int32_max_value_, rhs.int32_max_value_);
	utility::swap(int32_value_, rhs.int32_value_);

	string_default_value_.swap(rhs.string_default_value_);
	string_values_.swap(rhs.string_values_);
	string_value_.swap(rhs.string_value_);
}

[[noreturn]] void CVar::fail(const char* message)
{
	throw CVarException{message};
}

[[noreturn]] void CVar::fail_unknown_type()
{
	fail("Unknown type.");
}

[[noreturn]] void CVar::fail_nested(const char* message)
{
	std::throw_with_nested(CVarException{message});
}

void CVar::validate_name(StringView name)
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

CVar::CVar(
	CVarInt32Tag,
	StringView name,
	CVarFlags flags,
	Int32 default_value,
	Int32 min_value,
	Int32 max_value,
	std::initializer_list<Int32> values)
try
{
	validate_name(name);
	const auto value_count = values.size();

	if (value_count > 0)
	{
		auto last_value = *values.begin();
		auto found_default_value = (last_value == default_value);

		for (auto i = 1; i < value_count; ++i)
		{
			const auto& value = values.begin()[i];

			if (value <= last_value)
			{
				fail("Unordered or duplicate values.");
			}

			found_default_value |= (value == default_value);
		}

		if (!found_default_value)
		{
			fail("Default int32 value out of range.");
		}
	}
	else
	{
		if (min_value > max_value)
		{
			fail("Min int32 value out of range.");
		}

		if (default_value < min_value || default_value > max_value)
		{
			fail("Default int32 value out of range.");
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
}
catch (...)
{
	fail_nested(__func__);
}

void CVar::set_string_from_int32()
try
{
	char chars[max_int32_chars];
	const auto char_count = char_conv::to_chars(int32_value_, make_span(chars, max_int32_chars), 10);
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

	int32_value_ = char_conv::from_chars<Int32>(
		make_span(string),
		10,
		char_conv::FromCharsFormat::no_prefix);
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
