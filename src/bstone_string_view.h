/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_VIEW_INCLUDED
#define BSTONE_STRING_VIEW_INCLUDED

#include <cassert>
#include <cstddef>
#include <algorithm>
#include <type_traits>
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_utility.h"

namespace bstone {

template<typename TChar>
class StringViewT
{
public:
	StringViewT() = default;
	StringViewT(const StringViewT&) = default;
	StringViewT& operator=(const StringViewT&) = default;

	StringViewT(std::nullptr_t) = delete;

	constexpr StringViewT(const TChar* chars, IntP size)
		:
		chars_{chars},
		size_{size}
	{}

	constexpr StringViewT(const TChar* chars)
		:
		chars_{chars},
		size_{char_traits::get_size(chars)}
	{}

	constexpr const TChar* get_data() const noexcept
	{
		return chars_;
	}

	constexpr IntP get_size() const noexcept
	{
		return size_;
	}

	constexpr bool is_empty() const noexcept
	{
		return get_size() == 0;
	}

	constexpr const TChar* begin() const noexcept
	{
		return get_data();
	}

	constexpr const TChar* end() const noexcept
	{
		return begin() + get_size();
	}

	constexpr const TChar* cbegin() const noexcept
	{
		return begin();
	}

	constexpr const TChar* cend() const noexcept
	{
		return end();
	}

	constexpr const TChar& operator[](IntP index) const
	{
		assert(index >= 0 && index < get_size());
		return get_data()[index];
	}

	constexpr const TChar& get_front() const
	{
		return operator[](0);
	}

	constexpr const TChar& get_back() const
	{
		return operator[](get_size() - 1);
	}

	constexpr int compare(StringViewT rhs) const noexcept
	{
		return char_traits::compare(get_data(), get_size(), rhs.get_data(), rhs.get_size());
	}

	constexpr bool starts_with(StringViewT substring) const noexcept
	{
		const auto substring_size = substring.get_size();

		if (substring_size > get_size())
		{
			return false;
		}

		const auto string = StringViewT{get_data(), substring_size};
		return string.compare(substring) == 0;
	}

	constexpr bool starts_with(TChar ch) const noexcept
	{
		return !is_empty() && get_front() == ch;
	}

	constexpr bool starts_with(const TChar* chars) const
	{
		return starts_with(StringViewT{chars});
	}

	constexpr bool ends_with(StringViewT substring) const noexcept
	{
		const auto substring_size = substring.get_size();

		if (substring_size > get_size())
		{
			return false;
		}

		const auto string = StringViewT{get_data() + get_size() - substring_size, substring_size};
		return string.compare(substring) == 0;
	}

	constexpr bool ends_with(TChar ch) const noexcept
	{
		return !is_empty() && get_back() == ch;
	}

	constexpr bool ends_with(const TChar* chars) const
	{
		return ends_with(StringViewT{chars});
	}

	constexpr StringViewT get_subview(IntP index, IntP size) const
	{
		const auto this_size = get_size();

		if (index < 0 || index >= this_size)
		{
			BSTONE_THROW_STATIC_SOURCE("View index out of range.");
		}

		assert(size >= 0);

		return StringViewT{get_data() + index, std::min(this_size - index, size)};
	}

	constexpr StringViewT get_subview(IntP index) const
	{
		return get_subview(index, get_size());
	}

	constexpr void swap(StringViewT& rhs) noexcept
	{
		bstone::swop(chars_, rhs.chars_);
		bstone::swop(size_, rhs.size_);
	}

private:
	const TChar* chars_{};
	IntP size_{};
};

// ==========================================================================

template<typename TChar>
inline constexpr bool operator==(StringViewT<TChar> a, StringViewT<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
inline constexpr bool operator!=(StringViewT<TChar> a, StringViewT<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

using StringView = StringViewT<char>;
using U16StringView = StringViewT<char16_t>;
using U32StringView = StringViewT<char32_t>;

} // namespace bstone

#endif // !BSTONE_STRING_VIEW_INCLUDED
