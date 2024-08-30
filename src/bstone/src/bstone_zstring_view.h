/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated string view.

#ifndef BSTONE_ZSTRING_VIEW_INCLUDED
#define BSTONE_ZSTRING_VIEW_INCLUDED

#include <cstddef>
#include <cstdint>

#include <type_traits>

#include "bstone_assert.h"
#include "bstone_string_view.h"

namespace bstone {

template<typename TChar>
class BasicZStringView
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported type.");

public:
	using Char = TChar;

public:
	constexpr BasicZStringView() noexcept;
	constexpr BasicZStringView(const BasicZStringView&) noexcept = default;
	constexpr BasicZStringView(const Char* string) noexcept;
	BasicZStringView(std::nullptr_t) = delete;

	constexpr BasicZStringView& operator=(const BasicZStringView&) noexcept = default;

	constexpr const Char* begin() const noexcept;
	constexpr const Char* cbegin() const noexcept;

	constexpr const Char* end() const noexcept;
	constexpr const Char* cend() const noexcept;

	constexpr operator BasicStringView<TChar>() const noexcept;
	constexpr const Char& operator[](std::intptr_t index) const noexcept;
	constexpr const Char& get_front() const noexcept;
	constexpr const Char& get_back() const noexcept;
	constexpr const Char* get_data() const noexcept;
	constexpr bool has_data() const noexcept;

	constexpr std::intptr_t get_size() const noexcept;
	constexpr bool is_empty() const noexcept;

	constexpr void remove_prefix(std::intptr_t count) noexcept;
	constexpr void swap(BasicZStringView& rhs) noexcept;

	constexpr BasicZStringView get_subview(std::intptr_t index) const noexcept;

	constexpr int compare(BasicZStringView rhs) const noexcept;

	constexpr bool starts_with(BasicZStringView view) const noexcept;
	constexpr bool starts_with(Char ch) const noexcept;

	constexpr bool ends_with(BasicZStringView view) const noexcept;
	constexpr bool ends_with(Char ch) const noexcept;

	constexpr bool contains(BasicZStringView view) const noexcept;
	constexpr bool contains(Char ch) const noexcept;

	constexpr std::intptr_t index_of(BasicZStringView view) const noexcept;
	constexpr std::intptr_t index_of(Char ch) const noexcept;

	constexpr std::intptr_t last_index_of(BasicZStringView view) const noexcept;
	constexpr std::intptr_t last_index_of(Char ch) const noexcept;

	constexpr std::intptr_t index_of_any(BasicZStringView view) const noexcept;
	constexpr std::intptr_t last_index_of_any(BasicZStringView view) const noexcept;

private:
	using View = BasicStringView<Char>;

private:
	View view_{};

private:
	constexpr BasicZStringView(const Char* chars, std::intptr_t c_count) noexcept;
};

// ==========================================================================

template<typename TChar>
constexpr BasicZStringView<TChar>::BasicZStringView() noexcept {}

template<typename TChar>
constexpr BasicZStringView<TChar>::BasicZStringView(const Char* string) noexcept
	:
	view_{string}
{
	BSTONE_ASSERT(view_.get_data()[view_.get_size()] == Char{});
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::begin() const noexcept -> const Char*
{
	return view_.begin();
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::cbegin() const noexcept -> const Char*
{
	return view_.cbegin();
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::end() const noexcept -> const Char*
{
	return view_.end();
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::cend() const noexcept -> const Char*
{
	return view_.cend();
}

template<typename TChar>
constexpr BasicZStringView<TChar>::operator BasicStringView<TChar>() const noexcept
{
	return BasicStringView<TChar>{get_data(), get_size()};
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::operator[](std::intptr_t index) const noexcept -> const Char&
{
	return view_[index];
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::get_front() const noexcept -> const Char&
{
	return view_.get_front();
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::get_back() const noexcept -> const Char&
{
	return view_.get_back();
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::get_data() const noexcept -> const Char*
{
	return view_.get_data();
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::has_data() const noexcept
{
	return view_.has_data();
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::get_size() const noexcept
{
	return view_.get_size();
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::is_empty() const noexcept
{
	return view_.is_empty();
}

template<typename TChar>
constexpr void BasicZStringView<TChar>::remove_prefix(std::intptr_t count) noexcept
{
	view_.remove_prefix(count);
}

template<typename TChar>
constexpr void BasicZStringView<TChar>::swap(BasicZStringView& rhs) noexcept
{
	view_.swap(rhs.view_);
}

template<typename TChar>
constexpr auto BasicZStringView<TChar>::get_subview(std::intptr_t index) const noexcept -> BasicZStringView
{
	const auto subview = view_.get_subview(index);
	return BasicZStringView{subview.get_data(), subview.get_size()};
}

template<typename TChar>
constexpr int BasicZStringView<TChar>::compare(BasicZStringView rhs) const noexcept
{
	return view_.compare(rhs);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::starts_with(BasicZStringView view) const noexcept
{
	return view_.starts_with(view);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::starts_with(Char ch) const noexcept
{
	return view_.starts_with(ch);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::ends_with(BasicZStringView view) const noexcept
{
	return view_.ends_with(view);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::ends_with(Char ch) const noexcept
{
	return view_.ends_with(ch);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::contains(BasicZStringView view) const noexcept
{
	return view_.contains(view);
}

template<typename TChar>
constexpr bool BasicZStringView<TChar>::contains(Char ch) const noexcept
{
	return view_.contains(ch);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::index_of(BasicZStringView view) const noexcept
{
	return view_.index_of(view);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::index_of(Char ch) const noexcept
{
	return view_.index_of(ch);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::last_index_of(BasicZStringView view) const noexcept
{
	return view_.last_index_of(view);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::last_index_of(Char ch) const noexcept
{
	return view_.last_index_of(ch);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::index_of_any(BasicZStringView view) const noexcept
{
	return view_.index_of_any(view);
}

template<typename TChar>
constexpr std::intptr_t BasicZStringView<TChar>::last_index_of_any(BasicZStringView view) const noexcept
{
	return view_.last_index_of_any(view);
}

template<typename TChar>
constexpr BasicZStringView<TChar>::BasicZStringView(const Char* chars, std::intptr_t c_count) noexcept
	:
	view_{chars, c_count}
{
	BSTONE_ASSERT(chars[c_count] == Char{});
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator==(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return a.compare(b) == 0;
}

template<typename TChar>
inline constexpr bool operator!=(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return !(a == b);
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator<(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return a.compare(b) < 0;
}

template<typename TChar>
inline constexpr bool operator<=(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return a.compare(b) <= 0;
}

// ==========================================================================

template<typename TChar>
inline constexpr bool operator>(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return a.compare(b) > 0;
}

template<typename TChar>
inline constexpr bool operator>=(BasicZStringView<TChar> a, BasicZStringView<TChar> b) noexcept
{
	return a.compare(b) >= 0;
}

// ==========================================================================

using ZStringView = BasicZStringView<char>;
using U16ZStringView = BasicZStringView<char16_t>;
using U32ZStringView = BasicZStringView<char32_t>;

} // namespace bstone

#endif // BSTONE_ZSTRING_VIEW_INCLUDED
