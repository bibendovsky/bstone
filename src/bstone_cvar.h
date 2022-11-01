/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVAR_INCLUDED
#define BSTONE_CVAR_INCLUDED

#include <initializer_list>
#include <vector>
#include "bstone_cvar_string.h"
#include "bstone_int.h"
#include "bstone_span.h"
#include "bstone_string_view.h"

namespace bstone {

enum class CVarType
{
	none = 0,
	int32,
	string,
};

struct CVarInt32Tag {};
struct CVarBoolTag {};
struct CVarStringTag {};

using CVarInt32Values = Span<const Int32>;
using CVarStringValues = Span<const StringView>;

// Notes:
// - The name, string's default value and string's values MUST remain valid whole CVAR's lifetime.
class CVar
{
public:
	// Defines int32 CVAR with a specified range.
	CVar(
		CVarInt32Tag,
		StringView name,
		Int32 default_value,
		Int32 min_value,
		Int32 max_value);

	// Defines an int32 CVAR with a specified allowed values.
	CVar(CVarInt32Tag, StringView name, Int32 default_value, std::initializer_list<Int32> values);

	// Defines an int32 CVAR with a maximum range.
	CVar(CVarInt32Tag, StringView name, Int32 default_value);

	// Defines a boolean CVAR.
	CVar(CVarBoolTag, StringView name, bool default_value);

	// Defines a string CVAR with any string value.
	CVar(CVarStringTag, StringView name, StringView default_value);

	// Defines a string CVAR with a list of allowed values.
	CVar(
		CVarStringTag,
		StringView name,
		StringView default_value,
		std::initializer_list<StringView> values);

	CVar(const CVar& rhs) = delete;
	CVar(CVar&& rhs) noexcept;
	CVar& operator=(const CVar& rhs) = delete;
	CVar& operator=(CVar&& rhs) noexcept;

	StringView get_name() const noexcept;

	bool get_bool() const noexcept;
	void set_bool(bool value);

	Int32 get_int32() const noexcept;
	void set_int32(Int32 value);
	CVarInt32Values get_int32_values() const noexcept;

	StringView get_string() const noexcept;
	void set_string(StringView value);
	CVarStringValues get_string_values() const noexcept;

	void swap(CVar& rhs);

private:
	using Int32Values = std::vector<Int32>;
	using StringValues = std::vector<StringView>;

private:
	static constexpr auto max_int32_chars = 11;

private:
	CVarType type_{};
	StringView name_{};

	Int32 int32_default_value_{};
	Int32 int32_min_value_{};
	Int32 int32_max_value_{};
	Int32Values int32_values_{};
	Int32 int32_value_{};

	StringView string_default_value_{};
	StringValues string_values_{};
	CVarString string_value_{};

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_unknown_type();
	[[noreturn]] static void fail_nested(const char* message);

	static void validate_name(StringView name);

private:
	CVar(
		CVarInt32Tag,
		StringView name,
		Int32 default_value,
		Int32 min_value,
		Int32 max_value,
		std::initializer_list<Int32> values);

	void set_string_from_int32();
	void set_int32_from_string();
	bool has_string(StringView string);
	void ensure_string();
};

} // namespace bstone

#endif // !BSTONE_CVAR_INCLUDED
