/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Read-only contiguous sequence of characters.

#ifndef BSTONE_STRING_VIEW_INCLUDED
#define BSTONE_STRING_VIEW_INCLUDED

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <utility>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_utility.h"

namespace bstone {

template<typename TChar>
class BasicStringView
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported type.");

public:
	using Char = TChar;

public:
	constexpr BasicStringView() noexcept;
	constexpr BasicStringView(const BasicStringView&) noexcept = default;
	constexpr BasicStringView(const Char* string) noexcept;

	// BasicStringView(const Char*, std::intptr_t)
	// BasicStringView(TIterBegin, TIterEnd)
	template<typename T1, typename T2>
	constexpr BasicStringView(T1 t1, T2 t2);

	BasicStringView(std::nullptr_t) = delete;

	constexpr BasicStringView& operator=(const BasicStringView&) noexcept = default;

	constexpr const Char* begin() const noexcept;
	constexpr const Char* cbegin() const noexcept;

	constexpr const Char* end() const noexcept;
	constexpr const Char* cend() const noexcept;

	constexpr const Char& operator[](std::intptr_t index) const noexcept;
	constexpr const Char& get_front() const noexcept;
	constexpr const Char& get_back() const noexcept;
	constexpr const Char* get_data() const noexcept;
	constexpr bool has_data() const noexcept;

	constexpr std::intptr_t get_size() const noexcept;
	constexpr bool is_empty() const noexcept;

	constexpr void remove_prefix(std::intptr_t count) noexcept;
	constexpr void remove_suffix(std::intptr_t count) noexcept;

	constexpr void swap(BasicStringView& rhs) noexcept;

	constexpr BasicStringView get_subview(std::intptr_t index) const noexcept;
	constexpr BasicStringView get_subview(std::intptr_t index, std::intptr_t count) const noexcept;

	constexpr int compare(BasicStringView rhs) const noexcept;

	constexpr bool starts_with(Char ch) const noexcept;
	constexpr bool starts_with(BasicStringView view) const noexcept;

	constexpr bool ends_with(Char ch) const noexcept;
	constexpr bool ends_with(BasicStringView view) const noexcept;

	constexpr bool contains(Char ch) const noexcept;
	constexpr bool contains(BasicStringView view) const noexcept;

	constexpr std::intptr_t index_of(Char ch) const noexcept;
	constexpr std::intptr_t index_of(BasicStringView view) const noexcept;

	constexpr std::intptr_t last_index_of(Char ch) const noexcept;
	constexpr std::intptr_t last_index_of(BasicStringView view) const noexcept;

	constexpr std::intptr_t index_of_any(Char ch) const noexcept;
	constexpr std::intptr_t index_of_any(BasicStringView view) const noexcept;

	constexpr std::intptr_t last_index_of_any(Char ch) const noexcept;
	constexpr std::intptr_t last_index_of_any(BasicStringView view) const noexcept;

	constexpr std::intptr_t index_not_of_any(Char ch) const noexcept;
	constexpr std::intptr_t index_not_of_any(BasicStringView view) const noexcept;

	constexpr std::intptr_t last_index_not_of_any(Char view) const noexcept;
	constexpr std::intptr_t last_index_not_of_any(BasicStringView view) const noexcept;

private:
	struct CtorCharsWithCountTag {};
	struct CtorIteratorsTag {};

	template<typename UChar, typename T1, typename T2>
	class MakeCtorTag
	{
	private:
		static constexpr auto is_t1_pointer_to_char =
			std::is_pointer<T1>::value &&
			std::is_same<std::remove_cv_t<std::remove_pointer_t<T1>>, UChar>::value;

		static constexpr auto is_t2_intptr_t =
			std::is_convertible<T2, std::intptr_t>::value;

		static constexpr auto are_chars_with_count = is_t1_pointer_to_char && is_t2_intptr_t;

	public:
		using Type = std::conditional_t<are_chars_with_count, CtorCharsWithCountTag, CtorIteratorsTag>;
	};

private:
	const TChar* data_{};
	std::intptr_t size_{};

private:
	constexpr BasicStringView(const Char* chars, std::intptr_t count, CtorCharsWithCountTag);

	template<typename TIterBegin, typename TIterEnd>
	constexpr BasicStringView(TIterBegin first, TIterEnd last, CtorIteratorsTag);

	constexpr bool has_subview(std::intptr_t index, BasicStringView subview) const noexcept;
};

// ==========================================================================

template<typename TChar>
constexpr BasicStringView<TChar>::BasicStringView() noexcept {}

template<typename TChar>
constexpr BasicStringView<TChar>::BasicStringView(const Char* string) noexcept
	:
	BasicStringView{string, char_traits::get_size(string), CtorCharsWithCountTag{}}
{}

template<typename TChar>
template<typename T1, typename T2>
constexpr BasicStringView<TChar>::BasicStringView(T1 t1, T2 t2)
	:
	BasicStringView{t1, t2, typename MakeCtorTag<Char, T1, T2>::Type{}}
{}

template<typename TChar>
constexpr auto BasicStringView<TChar>::begin() const noexcept -> const Char*
{
	return get_data();
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::cbegin() const noexcept -> const Char*
{
	return begin();
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::end() const noexcept -> const Char*
{
	return begin() + get_size();
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::cend() const noexcept -> const Char*
{
	return end();
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::operator[](std::intptr_t index) const noexcept -> const Char&
{
	BSTONE_ASSERT(index >= 0 && index < get_size());

	return get_data()[index];
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::get_front() const noexcept -> const Char&
{
	return (*this)[0];
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::get_back() const noexcept -> const Char&
{
	return (*this)[get_size() - 1];
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::get_data() const noexcept -> const Char*
{
	return data_;
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::has_data() const noexcept
{
	return get_data() != nullptr;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::get_size() const noexcept
{
	return size_;
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::is_empty() const noexcept
{
	return get_size() == 0;
}

template<typename TChar>
constexpr void BasicStringView<TChar>::remove_prefix(std::intptr_t count) noexcept
{
	BSTONE_ASSERT(count >= 0 && count <= get_size());

	data_ += count;
	size_ -= count;
}

template<typename TChar>
constexpr void BasicStringView<TChar>::remove_suffix(std::intptr_t count) noexcept
{
	BSTONE_ASSERT(count >= 0 && count <= get_size());

	size_ -= count;
}

template<typename TChar>
constexpr void BasicStringView<TChar>::swap(BasicStringView& rhs) noexcept
{
	bstone::swop(data_, rhs.data_);
	bstone::swop(size_, rhs.size_);
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::get_subview(std::intptr_t index) const noexcept -> BasicStringView
{
	return get_subview(index, get_size() - index);
}

template<typename TChar>
constexpr auto BasicStringView<TChar>::get_subview(
	std::intptr_t index,
	std::intptr_t count) const noexcept -> BasicStringView
{
	BSTONE_ASSERT(index >= 0 && count >= 0);
	BSTONE_ASSERT(index + count <= get_size());

	return BasicStringView{get_data() + index, count};
}

template<typename TChar>
constexpr int BasicStringView<TChar>::compare(BasicStringView rhs) const noexcept
{
	return char_traits::compare(get_data(), get_size(), rhs.get_data(), rhs.get_size());
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::starts_with(Char ch) const noexcept
{
	return !is_empty() && get_front() == ch;
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::starts_with(BasicStringView view) const noexcept
{
	if (view.get_size() > get_size())
	{
		return false;
	}

	return has_subview(0, view);
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::ends_with(Char ch) const noexcept
{
	return !is_empty() && get_back() == ch;
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::ends_with(BasicStringView view) const noexcept
{
	const auto index = get_size() - view.get_size();

	if (index < 0)
	{
		return false;
	}

	return has_subview(index, view);
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::contains(Char ch) const noexcept
{
	return index_of(ch) >= 0;
}

template<typename TChar>
constexpr bool BasicStringView<TChar>::contains(BasicStringView view) const noexcept
{
	return index_of(view) >= 0;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_of(Char ch) const noexcept
{
	auto index = std::intptr_t{};

	for (const auto haystack_ch : *this)
	{
		if (haystack_ch == ch)
		{
			return index;
		}

		++index;
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_of(BasicStringView view) const noexcept
{
	const auto view_size = view.get_size();
	const auto max_haystack_index = get_size() - view_size;

	if (max_haystack_index < 0)
	{
		return -1;
	}

	if (view_size == 0)
	{
		return 0;
	}

	for (auto i_haystack = std::intptr_t{}; i_haystack <= max_haystack_index; ++i_haystack)
	{
		if (has_subview(i_haystack, view))
		{
			return i_haystack;
		}
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_of(Char ch) const noexcept
{
	const auto& haystack = *this;

	for (auto i_haystack = get_size() - 1; i_haystack >= 0; --i_haystack)
	{
		if (haystack[i_haystack] == ch)
		{
			return i_haystack;
		}
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_of(BasicStringView view) const noexcept
{
	const auto view_size = view.get_size();
	const auto max_haystack_index = get_size() - view_size;

	if (max_haystack_index < 0)
	{
		return -1;
	}

	if (view_size == 0)
	{
		return 0;
	}

	for (auto i_haystack = max_haystack_index; i_haystack >= 0; --i_haystack)
	{
		if (has_subview(i_haystack, view))
		{
			return i_haystack;
		}
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_of_any(Char ch) const noexcept
{
	return index_of(ch);
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_of_any(BasicStringView view) const noexcept
{
	const auto haystack_size = get_size();
	const auto needle_size = view.get_size();

	if (needle_size > haystack_size)
	{
		return -1;
	}

	if (needle_size == 0)
	{
		return 0;
	}

	if (needle_size == 1)
	{
		return index_of(view.get_front());
	}

	auto index = std::intptr_t{};

	for (const auto haystack_ch : *this)
	{
		for (const auto needle_ch : view)
		{
			if (needle_ch == haystack_ch)
			{
				return index;
			}
		}

		++index;
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_of_any(Char ch) const noexcept
{
	return last_index_of(ch);
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_of_any(BasicStringView view) const noexcept
{
	const auto haystack_size = get_size();
	const auto needle_size = view.get_size();

	if (needle_size > haystack_size)
	{
		return -1;
	}

	if (needle_size == 0)
	{
		return 0;
	}

	if (needle_size == 1)
	{
		return last_index_of(view.get_front());
	}

	const auto& haystack = *this;
	const auto& needle = view;

	for (auto i_haystack = haystack_size - 1; i_haystack >= 0; --i_haystack)
	{
		const auto haystack_ch = haystack[i_haystack];

		for (const auto needle_ch : needle)
		{
			if (needle_ch == haystack_ch)
			{
				return i_haystack;
			}
		}
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_not_of_any(Char ch) const noexcept
{
	auto index = std::intptr_t{};

	for (const auto haystack_ch : *this)
	{
		if (haystack_ch != ch)
		{
			return index;
		}

		++index;
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::index_not_of_any(BasicStringView view) const noexcept
{
	const auto haystack_size = get_size();
	const auto needle_size = view.get_size();

	if (needle_size > haystack_size)
	{
		return -1;
	}

	if (needle_size == 0)
	{
		return 0;
	}

	if (needle_size == 1)
	{
		return index_not_of_any(view.get_front());
	}

	const auto& haystack = *this;
	const auto& needle = view;
	auto index = std::intptr_t{};

	for (const auto haystack_ch : haystack)
	{
		auto is_found = true;

		for (const auto needle_ch : needle)
		{
			if (needle_ch == haystack_ch)
			{
				is_found = false;
				break;
			}
		}

		if (is_found)
		{
			return index;
		}

		++index;
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_not_of_any(Char ch) const noexcept
{
	const auto& haystack = *this;

	for (auto i_haystack = get_size() - 1; i_haystack >= 0; --i_haystack)
	{
		if (haystack[i_haystack] != ch)
		{
			return i_haystack;
		}
	}

	return -1;
}

template<typename TChar>
constexpr std::intptr_t BasicStringView<TChar>::last_index_not_of_any(BasicStringView view) const noexcept
{
	const auto haystack_size = get_size();
	const auto needle_size = view.get_size();

	if (needle_size > haystack_size)
	{
		return -1;
	}

	if (needle_size == 0)
	{
		return 0;
	}

	if (needle_size == 1)
	{
		return last_index_not_of_any(view.get_front());
	}

	const auto& haystack = *this;
	const auto& needle = view;

	for (auto i_haystack = haystack_size - 1; i_haystack >= 0; --i_haystack)
	{
		const auto haystack_ch = haystack[i_haystack];
		auto is_found = true;

		for (const auto needle_char : needle)
		{
			if (needle_char == haystack_ch)
			{
				is_found = false;
				break;
			}
		}

		if (is_found)
		{
			return i_haystack;
		}
	}

	return -1;
}

template<typename TChar>
constexpr BasicStringView<TChar>::BasicStringView(const Char* chars, std::intptr_t count, CtorCharsWithCountTag)
	:
	data_{chars},
	size_{count}
{
	BSTONE_ASSERT((get_data() != nullptr && get_size() >= 0) || (get_data() == nullptr && get_size() == 0));
}

template<typename TChar>
template<typename TIterBegin, typename TIterEnd>
constexpr BasicStringView<TChar>::BasicStringView(TIterBegin first, TIterEnd last, CtorIteratorsTag)
	:
	BasicStringView{&(*first), last - first, CtorCharsWithCountTag{}}
{}

template<typename TChar>
constexpr bool BasicStringView<TChar>::has_subview(std::intptr_t index, BasicStringView subview) const noexcept
{
	BSTONE_ASSERT(index >= 0 && subview.get_size() >= 0);
	BSTONE_ASSERT(index + subview.get_size() <= get_size());

	const auto& haystack = *this;

	const auto needle_size = subview.get_size();
	const auto& needle = subview;

	for (auto i = std::intptr_t{}; i < needle_size; ++i)
	{
		if (haystack[index + i] != needle[i])
		{
			return false;
		}
	}

	return true;
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator==(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
inline constexpr bool operator!=(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator<(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return a.compare(b) < 0;
}

template<typename TChar>
inline constexpr bool operator<=(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return a.compare(b) <= 0;
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator>(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return a.compare(b) > 0;
}

template<typename TChar>
inline constexpr bool operator>=(BasicStringView<TChar> a, BasicStringView<TChar> b) noexcept
{
	return a.compare(b) >= 0;
}

// ==========================================================================

using StringView = BasicStringView<char>;
using U16StringView = BasicStringView<char16_t>;
using U32StringView = BasicStringView<char32_t>;
using WStringView = BasicStringView<wchar_t>;

} // namespace bstone

#endif // BSTONE_STRING_VIEW_INCLUDED
