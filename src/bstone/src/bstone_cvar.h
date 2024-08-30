/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVAR_INCLUDED
#define BSTONE_CVAR_INCLUDED

#include <vector>
#include "bstone_cvar_string.h"
#include "bstone_span.h"
#include "bstone_string_view.h"
#include "bstone_enum_flags.h"

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

using CVarInt32Values = Span<const std::int32_t>;
using CVarStringValues = Span<const StringView>;

enum class CVarFlags : unsigned int
{
	none = 0,
	archive = 1U << 0,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(CVarFlags)

// Notes:
// - The name, string's default value and string's values MUST remain valid whole CVAR's lifetime.
class CVar
{
public:
	// Defines int32 CVAR with a specified range.
	CVar(
		CVarInt32Tag,
		StringView name,
		CVarFlags flags,
		std::int32_t default_value,
		std::int32_t min_value,
		std::int32_t max_value);

	// Defines an int32 CVAR with a specified allowed values.
	CVar(
		CVarInt32Tag,
		StringView name,
		CVarFlags flags,
		std::int32_t default_value,
		CVarInt32Values values);

	// Defines an int32 CVAR with a maximum range.
	CVar(CVarInt32Tag, StringView name, CVarFlags flags, std::int32_t default_value);

	// Defines a boolean CVAR.
	CVar(CVarBoolTag, StringView name, CVarFlags flags, bool default_value);

	// Defines a string CVAR with any string value.
	CVar(CVarStringTag, StringView name, CVarFlags flags, StringView default_value);

	// Defines a string CVAR with a list of allowed values.
	CVar(
		CVarStringTag,
		StringView name,
		CVarFlags flags,
		StringView default_value,
		CVarStringValues values);

	CVar(const CVar& rhs) = delete;
	CVar(CVar&& rhs) noexcept;
	CVar& operator=(const CVar& rhs) = delete;
	CVar& operator=(CVar&& rhs) noexcept;

	CVarType get_type() const noexcept;
	StringView get_name() const noexcept;
	CVarFlags get_flags() const noexcept;

	bool get_bool() const noexcept;
	void set_bool(bool value);

	std::int32_t get_int32() const noexcept;
	void set_int32(std::int32_t value);
	CVarInt32Values get_int32_values() const noexcept;

	StringView get_string() const noexcept;
	void set_string(StringView value);
	CVarStringValues get_string_values() const noexcept;

	void swap(CVar& rhs);

private:
	static constexpr auto max_int32_chars = 11;

private:
	CVarType type_{};
	StringView name_{};
	CVarFlags flags_{};

	std::int32_t int32_default_value_{};
	std::int32_t int32_min_value_{};
	std::int32_t int32_max_value_{};
	CVarInt32Values int32_values_{};
	std::int32_t int32_value_{};

	StringView string_default_value_{};
	CVarStringValues string_values_{};
	CVarString string_value_{};

private:
	[[noreturn]] static void fail_unknown_type();

private:
	CVar(
		CVarInt32Tag,
		StringView name,
		CVarFlags flags,
		std::int32_t default_value,
		std::int32_t min_value,
		std::int32_t max_value,
		CVarInt32Values values);

	void set_string_from_int32();
	void set_int32_from_string();
	bool has_string(StringView string);
	void ensure_string();
};

} // namespace bstone

#endif // !BSTONE_CVAR_INCLUDED
