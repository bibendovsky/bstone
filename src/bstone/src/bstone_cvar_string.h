/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVAR_STRING_INCLUDED
#define BSTONE_CVAR_STRING_INCLUDED

#include <cstdint>

#include <memory>

#include "bstone_string_view.h"

namespace bstone {

class CVarString
{
public:
	CVarString() noexcept;
	explicit CVarString(StringView string_view);
	CVarString(const CVarString& rhs);
	CVarString(CVarString&& rhs) noexcept;
	CVarString& operator=(StringView string_view);
	CVarString& operator=(const CVarString& rhs);
	CVarString& operator=(CVarString&& rhs) noexcept;

	StringView get() const noexcept;
	void set(StringView string_view);

	void swap(CVarString& rhs) noexcept;

private:
	using Storage = std::unique_ptr<char[]>;

private:
	static const std::intptr_t initial_capacity;

private:
	Storage storage_{};
	std::intptr_t capacity_{};
	std::intptr_t size_{};
};

} // namespace bstone

#endif // !BSTONE_CVAR_STRING_INCLUDED
