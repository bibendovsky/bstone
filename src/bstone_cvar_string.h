/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVAR_STRING_INCLUDED
#define BSTONE_CVAR_STRING_INCLUDED

#include "bstone_int.h"
#include "bstone_raw_uptr.h"
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
	static const auto initial_capacity = Int{32};

private:
	RawUPtr storage_{};
	Int capacity_{};
	Int size_{};
};

} // namespace bstone

#endif // !BSTONE_CVAR_STRING_INCLUDED
