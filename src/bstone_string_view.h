/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_VIEW_INCLUDED
#define BSTONE_STRING_VIEW_INCLUDED

#include <cassert>
#include <type_traits>
#include <bstone_c_str.h>
#include <bstone_int.h>

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
		assert(chars != nullptr);
		assert(size >= 0);
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

	constexpr int compare(const StringViewT& rhs) const noexcept
	{
		using Tag = std::conditional_t<
			std::is_integral<TChar>::value &&
				(sizeof(TChar) < sizeof(int) ||
					(sizeof(TChar) == sizeof(int) && std::is_signed<TChar>::value)),
			OptimalCompareTag,
			SuboptimalCompareTag>;

		return compare(rhs, Tag{});
	}

private:
	struct OptimalCompareTag {};
	struct SuboptimalCompareTag {};

	const TChar* data_{};
	Int size_{};

	constexpr int compare(const StringViewT& rhs, OptimalCompareTag) const noexcept
	{
		const auto& lhs = *this;
		const auto lhs_size = lhs.get_size();
		const auto rhs_size = rhs.get_size();
		const auto size = (lhs_size < rhs_size ? lhs_size : rhs_size);

		for (auto i = decltype(size){}; i < size; ++i)
		{
			const auto delta = lhs[i] - rhs[i];

			if (delta != 0)
			{
				return delta;
			}
		}

		return static_cast<int>(lhs_size - rhs_size);
	}

	constexpr int compare(const StringViewT& rhs, SuboptimalCompareTag) const noexcept
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

		if (lhs_size == rhs_size)
		{
			return 0;
		}
		if (lhs_size < rhs_size)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
};

// ==========================================================================

template<typename TChar>
constexpr inline bool operator==(const StringViewT<TChar>& a, const StringViewT<TChar>& b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
constexpr inline bool operator!=(const StringViewT<TChar>& a, const StringViewT<TChar>& b) noexcept
{
	return !(a == b);
}

// ==========================================================================

using StringView = StringViewT<char>;
using U16StringView = StringViewT<char16_t>;

} // namespace bstone

#endif // !BSTONE_STRING_VIEW_INCLUDED
