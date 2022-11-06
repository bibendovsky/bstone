/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_VIEW_INCLUDED
#define BSTONE_STRING_VIEW_INCLUDED

#include <cassert>
#include <type_traits>
#include "bstone_c_str.h"
#include "bstone_int.h"
#include "bstone_span.h"
#include "bstone_utility.h"

namespace bstone {

template<typename TChar>
class StringViewT
{
public:
	StringViewT() = default;

	constexpr StringViewT(std::nullptr_t) = delete;
	constexpr StringViewT(const StringViewT&) noexcept = default;

	constexpr StringViewT(const TChar* chars, Int size) noexcept
		:
		data_{chars},
		size_{size}
	{
		assert(size == 0 || (size > 0 && chars != nullptr));
	}

	constexpr StringViewT(const TChar* chars) noexcept
		:
		StringViewT{chars, c_str::get_size(chars)}
	{}

	constexpr const TChar* get_data() const noexcept
	{
		return data_;
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

	constexpr const TChar& operator[](Int index) const noexcept
	{
		assert(index >= 0 && index < get_size());
		return get_data()[index];
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

	constexpr void swap(StringViewT& rhs) noexcept
	{
		utility::swap(data_, rhs.data_);
	}

private:
	const TChar* data_{};
	Int size_{};
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

template<typename TChar>
struct StringViewHasherT
{
	constexpr std::size_t operator()(StringViewT<TChar> string_view) const noexcept
	{
		using Unsigned = std::conditional_t<
			sizeof(TChar) == 1,
			UInt8,
			std::conditional_t<
				sizeof(TChar) == 2,
				UInt16,
				std::conditional_t<
					sizeof(TChar) == 4,
					UInt32,
					void
				>
			>
		>;

		auto hash = std::size_t{};

		for (const auto ch : string_view)
		{
			hash += static_cast<Unsigned>(ch);
			hash += hash << 10;
			hash ^= hash >> 6;
		}

		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}
};

// ==========================================================================

using StringView = StringViewT<char>;
using StringViewHasher = StringViewHasherT<char>;

using U16StringView = StringViewT<char16_t>;
using U16StringViewHasher = StringViewHasherT<char16_t>;

using U32StringView = StringViewT<char32_t>;
using U32StringViewHasher = StringViewHasherT<char32_t>;

// ==========================================================================

template<typename TChar>
inline constexpr auto make_span(StringViewT<TChar> string_view) noexcept
{
	return Span<const TChar>{string_view.get_data(), string_view.get_size()};
}

} // namespace bstone

#endif // !BSTONE_STRING_VIEW_INCLUDED
