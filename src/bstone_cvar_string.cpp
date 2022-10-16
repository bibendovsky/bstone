/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <algorithm>
#include <memory>
#include "bstone_cvar_string.h"

namespace bstone {

CVarString::CVarString() noexcept = default;

CVarString::CVarString(StringView string_view)
{
	set(string_view);
}

CVarString::CVarString(const CVarString& rhs)
	:
	CVarString{rhs.get()}
{}

CVarString::CVarString(CVarString&& rhs) noexcept
{
	swap(rhs);
}

CVarString& CVarString::operator=(StringView string_view)
{
	set(string_view);
	return *this;
}

CVarString& CVarString::operator=(const CVarString& rhs)
{
	if (std::addressof(rhs) != this)
	{
		set(rhs.get());
	}

	return *this;
}

CVarString& CVarString::operator=(CVarString&& rhs) noexcept
{
	swap(rhs);
	return *this;
}

StringView CVarString::get() const noexcept
{
	return StringView{storage_.get<const char*>(), size_};
}

void CVarString::set(StringView string_view)
{
	const auto new_size = string_view.get_size();

	if (new_size > capacity_)
	{
		const auto new_capacity = std::max(new_size, initial_capacity);
		auto new_storage = make_raw_uptr(new_capacity);
		storage_.swap(new_storage);
		capacity_ = new_capacity;
	}

	std::uninitialized_copy_n(string_view.get_data(), new_size, storage_.get<char*>());
	size_ = new_size;
}

void CVarString::swap(CVarString& rhs) noexcept
{
	storage_.swap(rhs.storage_);
	std::swap(capacity_, rhs.capacity_);
	std::swap(size_, rhs.size_);
}

} // namespace bstone
