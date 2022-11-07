/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_VIEW_INCLUDED
#define BSTONE_STRING_VIEW_INCLUDED

#include <cstddef>
#include <algorithm>
#include <type_traits>
#include "bstone_c_str.h"
#include "bstone_utility.h"

namespace bstone {

namespace detail {

[[noreturn]] void string_view_fail(const char* message);

} // namespace detail

template<typename TChar>
class StringViewT
{
public:
	StringViewT() noexcept = default;

	constexpr StringViewT(std::nullptr_t) = delete;
	constexpr StringViewT(const StringViewT&) noexcept = default;
	constexpr StringViewT& operator=(const StringViewT&) noexcept = default;

	constexpr StringViewT(const TChar* chars, Int size)
		:
		span_{chars, size}
	{}

	constexpr explicit StringViewT(const TChar* chars)
		:
		span_{chars, c_str::get_size(chars)}
	{}

	template<typename UChar>
	constexpr explicit StringViewT(Span<UChar> span) noexcept
		:
		span_{span.get_data(), span.get_size()}
	{}

	constexpr StringViewT& operator=(const TChar* chars)
	{
		span_ = c_str::make_span(chars);
		return *this;
	}

	constexpr auto to_span() const noexcept
	{
		return span_;
	}

	constexpr const TChar* get_data() const noexcept
	{
		return span_.get_data();
	}

	constexpr Int get_size() const noexcept
	{
		return span_.get_size();
	}

	constexpr bool is_empty() const noexcept
	{
		return span_.is_empty();
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

	constexpr const TChar& operator[](Int index) const
	{
		return span_[index];
	}

	constexpr int compare(StringViewT rhs) const noexcept
	{
		const auto& lhs = *this;
		const auto lhs_size = lhs.get_size();
		const auto rhs_size = rhs.get_size();
		const auto size = (lhs_size < rhs_size ? lhs_size : rhs_size);

		for (auto i = decltype(size){}; i < size; ++i)
		{
			const auto& lhs_char = lhs[i];
			const auto& rhs_char = rhs[i];

			if (lhs_char < rhs_char)
			{
				return -1;
			}
			else if (lhs_char > rhs_char)
			{
				return 1;
			}
		}

		if (lhs_size < rhs_size)
		{
			return -1;
		}
		else if (lhs_size > rhs_size)
		{
			return 1;
		}
		else
		{
			return 0;
		}
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
		return !is_empty() && span_.get_front() == ch;
	}

	constexpr bool starts_with(const TChar* chars) const noexcept
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
		return !is_empty() && span_.get_back() == ch;
	}

	constexpr bool ends_with(const TChar* chars) const noexcept
	{
		return ends_with(StringViewT{chars});
	}

	constexpr StringViewT get_subview(Int index, Int size) const
	{
		const auto this_size = get_size();

		if (index < 0 || index >= this_size)
		{
			detail::string_view_fail("View index out of range.");
		}

		if (size < 0)
		{
			detail::string_view_fail("View size out of range.");
		}

		return StringViewT{get_data() + index, (std::min)(this_size - index, size)};
	}

	constexpr StringViewT get_subview(Int index) const
	{
		const auto size = get_size();

		if (index < 0 || index >= size)
		{
			detail::string_view_fail("View index out of range.");
		}

		return StringViewT{get_data() + index, size - index};
	}

	constexpr void swap(StringViewT& rhs) noexcept
	{
		span_.swap(rhs.span_);
	}

private:
	Span<const TChar> span_{};
};

// ==========================================================================

template<typename TChar>
constexpr inline bool operator==(StringViewT<TChar> a, StringViewT<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
constexpr inline bool operator!=(StringViewT<TChar> a, StringViewT<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

using StringView = StringViewT<char>;
using U16StringView = StringViewT<char16_t>;
using U32StringView = StringViewT<char32_t>;

} // namespace bstone

#endif // !BSTONE_STRING_VIEW_INCLUDED
