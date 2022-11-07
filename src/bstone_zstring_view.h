/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated string view.

#ifndef BSTONE_ZSTRING_VIEW_INCLUDED
#define BSTONE_ZSTRING_VIEW_INCLUDED

#include <cassert>
#include "bstone_string_view.h"

namespace bstone {

namespace detail {

[[noreturn]] void zstring_view_fail_non_null_terminated();

} // namespace detail

template<typename TChar>
class ZStringViewT
{
public:
	ZStringViewT() noexcept = default;

	constexpr ZStringViewT(std::nullptr_t) = delete;
	constexpr ZStringViewT(const ZStringViewT&) noexcept = default;
	constexpr ZStringViewT& operator=(const ZStringViewT&) noexcept = default;

	constexpr explicit ZStringViewT(const TChar* chars)
		:
		string_view_{chars}
	{
		if ((*string_view_.cend()) != TChar{})
		{
			detail::zstring_view_fail_non_null_terminated();
		}
	}

	constexpr auto to_string_view() const noexcept
	{
		return string_view_;
	}

	constexpr const TChar* get_data() const noexcept
	{
		return string_view_.get_data();
	}

	constexpr Int get_size() const noexcept
	{
		return string_view_.get_size();
	}

	constexpr bool is_empty() const noexcept
	{
		return string_view_.is_empty();
	}

	constexpr const TChar* begin() const noexcept
	{
		return string_view_.begin();
	}

	constexpr const TChar* end() const noexcept
	{
		return string_view_.end();
	}

	constexpr const TChar* cbegin() const noexcept
	{
		return string_view_.cbegin();
	}

	constexpr const TChar* cend() const noexcept
	{
		return string_view_.cend();
	}

	constexpr const TChar& operator[](Int index) const
	{
		assert(index >= 0 && index <= get_size());
		return get_data()[index];
	}

	constexpr int compare(ZStringViewT rhs) const noexcept
	{
		return string_view_.compare(rhs.to_string_view());
	}

	constexpr void swap(ZStringViewT& rhs) noexcept
	{
		string_view_.swap(rhs.string_view_);
	}

private:
	StringViewT<TChar> string_view_{};
};

// ==========================================================================

template<typename TChar>
constexpr inline bool operator==(ZStringViewT<TChar> a, ZStringViewT<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
constexpr inline bool operator!=(ZStringViewT<TChar> a, ZStringViewT<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

using ZStringView = ZStringViewT<char>;
using U16ZStringView = ZStringViewT<char16_t>;
using U32ZStringView = ZStringViewT<char32_t>;

} // namespace bstone

#endif // !BSTONE_ZSTRING_VIEW_INCLUDED
