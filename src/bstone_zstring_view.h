/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated string view.

#if !defined(BSTONE_ZSTRING_VIEW_INCLUDED)
#define BSTONE_ZSTRING_VIEW_INCLUDED

#include <cassert>
#include <cstddef>
#include "bstone_char_traits.h"
#include "bstone_utility.h"

namespace bstone {

template<typename TChar>
class ZStringViewT
{
public:
	ZStringViewT() = default;
	ZStringViewT(const ZStringViewT&) = default;
	ZStringViewT& operator=(const ZStringViewT&) = default;

	ZStringViewT(std::nullptr_t) = delete;

	constexpr ZStringViewT(const TChar* chars)
		:
		chars_{chars},
		size_{char_traits::get_size(chars)}
	{}

	constexpr const TChar* get_data() const noexcept
	{
		return chars_;
	}

	constexpr Int get_size() const noexcept
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
		return get_data() + get_size();
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
		assert(index >= 0 && index <= get_size());
		return get_data()[index];
	}

	constexpr int compare(ZStringViewT rhs) const noexcept
	{
		return char_traits::compare(get_data(), get_size(), rhs.get_data(), rhs.get_size());
	}

	constexpr void swap(ZStringViewT& rhs) noexcept
	{
		utility::swap(chars_, rhs.chars_);
		utility::swap(size_, rhs.size_);
	}

private:
	const TChar* chars_{};
	Int size_{};
};

// ==========================================================================

template<typename TChar>
inline constexpr bool operator==(ZStringViewT<TChar> a, ZStringViewT<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
inline constexpr bool operator!=(ZStringViewT<TChar> a, ZStringViewT<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

using ZStringView = ZStringViewT<char>;
using U16ZStringView = ZStringViewT<char16_t>;
using U32ZStringView = ZStringViewT<char32_t>;

} // namespace bstone

#endif // BSTONE_ZSTRING_VIEW_INCLUDED
