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

// Notes:
// - The name, string's default value and string's values MUST remain valid whole CVAR's lifetime.
class CVar
{
public:
	CVar(
		CVarInt32Tag,
		StringView name,
		Int32 default_value,
		Int32 min_value,
		Int32 max_value);

	CVar(CVarInt32Tag, StringView name, Int32 default_value);

	CVar(CVarBoolTag, StringView name, bool default_value);

	CVar(CVarStringTag, StringView name, StringView default_value);

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

	StringView get_string() const noexcept;
	void set_string(StringView value);

	void swap(CVar& rhs);

private:
	using StringValues = std::vector<StringView>;

private:
	static constexpr auto max_int32_chars = 11;

private:
	CVarType type_{};
	StringView name_{};

	Int32 int32_default_value_{};
	Int32 int32_min_value_{};
	Int32 int32_max_value_{};
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
	void set_string_from_int32();
	void set_int32_from_string();
	bool has_string(StringView string);
	void ensure_string();
};

} // namespace bstone

#endif // !BSTONE_CVAR_INCLUDED
