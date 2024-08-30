/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated string.

#ifndef BSTONE_STRING_INCLUDED
#define BSTONE_STRING_INCLUDED

#include <cstdint>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_memory_resource.h"
#include "bstone_string_view.h"
#include "bstone_utility.h"

namespace bstone {

struct StringFillTag {};
struct StringCapacityTag {};

// ==========================================================================

template<typename TChar>
class BasicString
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported character type.");

public:
	using Char = TChar;
	using View = BasicStringView<Char>;

public:
	BasicString();
	explicit BasicString(MemoryResource& memory_resource);
	BasicString(std::intptr_t capacity, StringCapacityTag);
	BasicString(std::intptr_t capacity, StringCapacityTag, MemoryResource& memory_resource);
	BasicString(Char ch, std::intptr_t c_count, StringFillTag);
	BasicString(Char ch, std::intptr_t c_count, MemoryResource& memory_resource);
	BasicString(const Char* string);
	BasicString(const Char* string, MemoryResource& memory_resource);
	BasicString(const Char* chars, std::intptr_t c_count);
	BasicString(const Char* chars, std::intptr_t c_count, MemoryResource& memory_resource);
	explicit BasicString(View view);
	BasicString(View view, MemoryResource& memory_resource);
	BasicString(std::initializer_list<Char> list);
	BasicString(std::initializer_list<Char> list, MemoryResource& memory_resource);
	BasicString(const BasicString& rhs);
	BasicString(BasicString&& rhs) noexcept;

	BasicString& operator=(const BasicString& rhs);
	BasicString& operator=(BasicString&& rhs) noexcept;

	MemoryResource& get_memory_resource() const noexcept;

	const Char* get_data() const noexcept;
	Char* get_data() noexcept;

	std::intptr_t get_capacity() const noexcept;
	std::intptr_t get_size() const noexcept;
	bool is_empty() const noexcept;

	const Char* begin() const noexcept;
	Char* begin() noexcept;

	const Char* end() const noexcept;
	Char* end() noexcept;

	const Char* cbegin() const noexcept;
	const Char* cend() const noexcept;

	const Char& operator[](std::intptr_t index) const noexcept;
	Char& operator[](std::intptr_t index) noexcept;

	const Char& get_front() const noexcept;
	Char& get_front() noexcept;

	const Char& get_back() const noexcept;
	Char& get_back() noexcept;

	void reserve(std::intptr_t capacity);
	void resize(std::intptr_t size);
	void resize(std::intptr_t size, Char ch);
	void clear();

	void swap(BasicString& rhs) noexcept;

	BasicString& assign(Char ch, std::intptr_t c_count);
	BasicString& assign(const Char* string);
	BasicString& assign(const Char* chars, std::intptr_t c_count);
	BasicString& assign(const BasicString& string);
	BasicString& assign(const BasicString& string, std::intptr_t s_index);
	BasicString& assign(const BasicString& string, std::intptr_t s_index, std::intptr_t s_count);
	BasicString& assign(BasicString&& rhs) noexcept;
	BasicString& assign(View view);

	template<typename TIter>
	BasicString& assign(TIter iter_begin, TIter iter_end);

	BasicString& assign(std::initializer_list<Char> list);

	BasicString& append(Char ch);
	BasicString& append(Char ch, std::intptr_t c_count);
	BasicString& append(const Char* string);
	BasicString& append(const Char* chars, std::intptr_t c_count);
	BasicString& append(const BasicString& string);
	BasicString& append(const BasicString& string, std::intptr_t s_index);
	BasicString& append(const BasicString& string, std::intptr_t s_index, std::intptr_t s_count);
	BasicString& append(View view);

	template<typename TIter>
	BasicString& append(TIter iter_begin, TIter iter_end);

	BasicString& append(std::initializer_list<Char> list);

	BasicString& insert(std::intptr_t index, Char ch);
	BasicString& insert(std::intptr_t index, Char ch, std::intptr_t c_count);
	BasicString& insert(std::intptr_t index, const Char* string);
	BasicString& insert(std::intptr_t index, const Char* chars, std::intptr_t c_count);
	BasicString& insert(std::intptr_t index, const BasicString& string);
	BasicString& insert(std::intptr_t index, const BasicString& string, std::intptr_t s_index);
	BasicString& insert(
		std::intptr_t index, const BasicString& string, std::intptr_t s_index, std::intptr_t s_count);
	BasicString& insert(std::intptr_t index, View view);

	template<typename TSrcIter>
	Char* insert(const Char* dst_iter, TSrcIter src_iter_begin, TSrcIter src_iter_end);

	Char* insert(const Char* dst_iter, std::initializer_list<Char> src_chars);

	BasicString& erase(std::intptr_t index);
	BasicString& erase(std::intptr_t index, std::intptr_t count);
	Char* erase(const Char* iter);
	Char* erase(const Char* iter_begin, const Char* iter_end);

	void push_back(Char ch);
	Char pop_back();

	BasicString get_substring(std::intptr_t index) const;
	BasicString get_substring(std::intptr_t index, std::intptr_t count) const;

	View get_view() const noexcept;
	View get_view(std::intptr_t index) const noexcept;
	View get_view(std::intptr_t index, std::intptr_t count) const noexcept;

	operator View() const noexcept
	{
		return get_view();
	}

	int compare(const Char* string) const noexcept;
	int compare(const Char* chars, std::intptr_t c_count) const noexcept;
	int compare(const BasicString& string) const noexcept;
	int compare(View view) const noexcept;

	std::intptr_t index_of(Char ch) const noexcept;
	std::intptr_t index_of(const Char* substring) const noexcept;
	std::intptr_t index_of(const Char* subchars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_of(const BasicString& substring) const noexcept;
	std::intptr_t index_of(View subview) const noexcept;

	std::intptr_t index_of(std::intptr_t index, Char ch) const noexcept;
	std::intptr_t index_of(std::intptr_t index, const Char* substring) const noexcept;
	std::intptr_t index_of(std::intptr_t index, const Char* subchars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_of(std::intptr_t index, const BasicString& substring) const noexcept;
	std::intptr_t index_of(std::intptr_t index, View subview) const noexcept;

	std::intptr_t last_index_of(Char ch) const noexcept;
	std::intptr_t last_index_of(const Char* substring) const noexcept;
	std::intptr_t last_index_of(const Char* subchars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of(const BasicString& substring) const noexcept;
	std::intptr_t last_index_of(View subview) const noexcept;

	std::intptr_t last_index_of(std::intptr_t index, Char ch) const noexcept;
	std::intptr_t last_index_of(std::intptr_t index, const Char* substring) const noexcept;
	std::intptr_t last_index_of(std::intptr_t index, const Char* subchars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of(std::intptr_t index, const BasicString& substring) const noexcept;
	std::intptr_t last_index_of(std::intptr_t index, View subview) const noexcept;

	std::intptr_t index_of_any(Char ch) const noexcept;
	std::intptr_t index_of_any(const Char* string) const noexcept;
	std::intptr_t index_of_any(const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_of_any(const BasicString& string) const noexcept;
	std::intptr_t index_of_any(const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t index_of_any(
		const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept;
	std::intptr_t index_of_any(View string_view) const noexcept;

	std::intptr_t index_of_any(std::intptr_t index, const Char ch) const noexcept;
	std::intptr_t index_of_any(std::intptr_t index, const Char* string) const noexcept;
	std::intptr_t index_of_any(std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_of_any(std::intptr_t index, const BasicString& string) const noexcept;
	std::intptr_t index_of_any(
		std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t index_of_any(
		std::intptr_t index,
		const BasicString& string,
		std::intptr_t s_index,
		std::intptr_t s_count) const noexcept;
	std::intptr_t index_of_any(std::intptr_t index, View view) const noexcept;

	std::intptr_t index_not_of_any(Char ch) const noexcept;
	std::intptr_t index_not_of_any(const Char* string) const noexcept;
	std::intptr_t index_not_of_any(const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_not_of_any(const BasicString& string) const noexcept;
	std::intptr_t index_not_of_any(const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t index_not_of_any(
		const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept;
	std::intptr_t index_not_of_any(View view) const noexcept;

	std::intptr_t index_not_of_any(std::intptr_t index, Char ch) const noexcept;
	std::intptr_t index_not_of_any(std::intptr_t index, const Char* string) const noexcept;
	std::intptr_t index_not_of_any(
		std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t index_not_of_any(std::intptr_t index, const BasicString& string) const noexcept;
	std::intptr_t index_not_of_any(
		std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t index_not_of_any(
		std::intptr_t index,
		const BasicString& string,
		std::intptr_t s_index,
		std::intptr_t s_count) const noexcept;
	std::intptr_t index_not_of_any(std::intptr_t index, View view) const noexcept;

	std::intptr_t last_index_of_any(Char ch) const noexcept;
	std::intptr_t last_index_of_any(const Char* string) const noexcept;
	std::intptr_t last_index_of_any(const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of_any(const BasicString& string) const noexcept;
	std::intptr_t last_index_of_any(const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t last_index_of_any(
		const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of_any(View view) const noexcept;

	std::intptr_t last_index_of_any(std::intptr_t index, Char ch) const noexcept;
	std::intptr_t last_index_of_any(std::intptr_t index, Char* string) const noexcept;
	std::intptr_t last_index_of_any(std::intptr_t index, const Char* string) const noexcept;
	std::intptr_t last_index_of_any(std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of_any(std::intptr_t index, const BasicString& string) const noexcept;
	std::intptr_t last_index_of_any(
		std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t last_index_of_any(
		std::intptr_t index,
		const BasicString& string,
		std::intptr_t s_index,
		std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_of_any(std::intptr_t index, View view) const noexcept;

	std::intptr_t last_index_not_of_any(Char ch) const noexcept;
	std::intptr_t last_index_not_of_any(const Char* string) const noexcept;
	std::intptr_t last_index_not_of_any(const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_not_of_any(const BasicString& string) const noexcept;
	std::intptr_t last_index_not_of_any(const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t last_index_not_of_any(
		const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_not_of_any(View view) const noexcept;

	std::intptr_t last_index_not_of_any(std::intptr_t index, Char ch) const noexcept;
	std::intptr_t last_index_not_of_any(std::intptr_t index, const Char* string) const noexcept;
	std::intptr_t last_index_not_of_any(
		std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_not_of_any(std::intptr_t index, const BasicString& string) const noexcept;
	std::intptr_t last_index_not_of_any(
		std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept;
	std::intptr_t last_index_not_of_any(
		std::intptr_t index,
		const BasicString& string,
		std::intptr_t s_index,
		std::intptr_t s_count) const noexcept;
	std::intptr_t last_index_not_of_any(std::intptr_t index, View view) const noexcept;

	bool contains(Char ch) const noexcept;
	bool contains(const Char* substring) const noexcept;
	bool contains(const Char* subchars, std::intptr_t s_count) const noexcept;
	bool contains(const BasicString& substring) const noexcept;
	bool contains(View subview) const noexcept;

	bool starts_with(Char ch) const noexcept;
	bool starts_with(const Char* substring) const noexcept;
	bool starts_with(const Char* subchars, std::intptr_t s_count) const noexcept;
	bool starts_with(const BasicString& substring) const noexcept;
	bool starts_with(View subview) const noexcept;

	bool ends_with(Char ch) const noexcept;
	bool ends_with(const Char* substring) const noexcept;
	bool ends_with(const Char* subchars, std::intptr_t s_count) const noexcept;
	bool ends_with(const BasicString& substring) const noexcept;
	bool ends_with(View subview) const noexcept;

	static BasicString make_with_capacity(std::intptr_t capacity);
	static BasicString make_with_fill(Char value, std::intptr_t count);

private:
	static constexpr auto default_char = Char{};
	static constexpr auto local_capacity = 15;

private:
	using Storage = MemoryResourceUPtr<Char[]>;
	using Deleter = typename Storage::deleter_type;

	using LocalData = Char[local_capacity + 1];

	template<typename TIter>
	struct GetIteratorTag
	{
		using IteratorCategory = typename std::iterator_traits<TIter>::iterator_category;

		using Value = std::conditional_t<
			std::is_same<IteratorCategory, std::random_access_iterator_tag>::value,
			std::random_access_iterator_tag,
			std::input_iterator_tag>;
	};

	template<typename TIter>
	using GetIteratorTagT = typename GetIteratorTag<TIter>::Value;

	template<typename T>
	using IndexOfGenericFunc = std::intptr_t (View::*)(T) const;

private:
	Storage storage_;
	Char* data_;
	std::intptr_t capacity_{};
	std::intptr_t size_{};
	LocalData local_data_;

private:
	void terminate_with_null() noexcept;
	void set_size_and_terminate_without_this(std::intptr_t size);
	BasicString& set_size_and_terminate(std::intptr_t size);

	void copy_overlapped(std::intptr_t old_index, std::intptr_t count, std::intptr_t new_index);

	template<typename TIter>
	BasicString& assign(TIter iter_begin, TIter iter_end, std::input_iterator_tag);

	template<typename TIter>
	BasicString& assign(TIter iter_begin, TIter iter_end, std::random_access_iterator_tag);

	template<typename TIter>
	BasicString& append(TIter iter_begin, TIter iter_end, std::input_iterator_tag);

	template<typename TIter>
	BasicString& append(TIter iter_begin, TIter iter_end, std::random_access_iterator_tag);

	template<typename TSrcIter>
	Char* insert(
		const Char* dst_iter,
		TSrcIter src_iter_begin,
		TSrcIter src_iter_end,
		std::input_iterator_tag);

	template<typename TSrcIter>
	Char* insert(
		const Char* dst_iter,
		TSrcIter src_iter_begin,
		TSrcIter src_iter_end,
		std::random_access_iterator_tag);

	template<typename TSubValue>
	std::intptr_t index_of_generic(
		std::intptr_t index,
		TSubValue sub_value,
		IndexOfGenericFunc<TSubValue> func) const noexcept;
};

// --------------------------------------------------------------------------

template<typename TChar>
BasicString<TChar>::BasicString()
	:
	BasicString{get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(MemoryResource& memory_resource)
	:
	BasicString{0, StringCapacityTag{}, memory_resource}
{}

template<typename TChar>
BasicString<TChar>::BasicString(std::intptr_t capacity, StringCapacityTag)
	:
	BasicString{capacity, StringCapacityTag{}, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(std::intptr_t capacity, StringCapacityTag, MemoryResource& memory_resource)
	:
	storage_{
		capacity > local_capacity ? memory_resource.template allocate<Char>(capacity + 1) : nullptr,
		Deleter{memory_resource}},
	data_{capacity > local_capacity ? storage_.get() : local_data_},
	capacity_{capacity}
{
	BSTONE_ASSERT(capacity >= 0);

	terminate_with_null();
}

template<typename TChar>
BasicString<TChar>::BasicString(Char ch, std::intptr_t c_count, StringFillTag)
	:
	BasicString{ch, c_count, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(Char ch, std::intptr_t c_count, MemoryResource& memory_resource)
	:
	BasicString{c_count, StringCapacityTag{}, memory_resource}
{
	std::fill_n(get_data(), c_count, ch);
	set_size_and_terminate_without_this(c_count);
}

template<typename TChar>
BasicString<TChar>::BasicString(const Char* string)
	:
	BasicString{string, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(const Char* string, MemoryResource& memory_resource)
	:
	BasicString{View{string}, memory_resource}
{}

template<typename TChar>
BasicString<TChar>::BasicString(const Char* chars, std::intptr_t c_count)
	:
	BasicString{chars, c_count, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(std::initializer_list<Char> list)
	:
	BasicString{list, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(std::initializer_list<Char> list, MemoryResource& memory_resource)
	:
	BasicString{list.begin(), static_cast<std::intptr_t>(list.size()), memory_resource}
{}

template<typename TChar>
BasicString<TChar>::BasicString(const Char* chars, std::intptr_t s_count, MemoryResource& memory_resource)
	:
	BasicString{View{chars, s_count}, memory_resource}
{}

template<typename TChar>
BasicString<TChar>::BasicString(View string_view)
	:
	BasicString{string_view, get_default_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(View view, MemoryResource& memory_resource)
	:
	BasicString{view.get_size(), StringCapacityTag{}, memory_resource}
{
	const auto string_view_size = view.get_size();
	std::copy_n(view.get_data(), string_view_size, get_data());
	set_size_and_terminate_without_this(string_view_size);
}

template<typename TChar>
BasicString<TChar>::BasicString(const BasicString& rhs)
	:
	BasicString{rhs.get_view(), rhs.get_memory_resource()}
{}

template<typename TChar>
BasicString<TChar>::BasicString(BasicString&& rhs) noexcept
	:
	storage_{std::move(rhs.storage_)},
	capacity_{rhs.capacity_},
	size_{rhs.size_}
{
	if (get_capacity() <= local_capacity)
	{
		std::copy_n(rhs.local_data_, get_size() + 1, local_data_);
		data_ = local_data_;
	}
	else
	{
		data_ = storage_.get();
	}
}

template<typename TChar>
auto BasicString<TChar>::operator=(const BasicString& rhs) -> BasicString&
{
	BSTONE_ASSERT(this != std::addressof(rhs));

	auto string_copy = rhs;
	swap(string_copy);
	return *this;
}

template<typename TChar>
auto BasicString<TChar>::operator=(BasicString&& rhs) noexcept -> BasicString&
{
	BSTONE_ASSERT(this != std::addressof(rhs));

	swap(rhs);
	return *this;
}

template<typename TChar>
MemoryResource& BasicString<TChar>::get_memory_resource() const noexcept
{
	return storage_.get_deleter().get_memory_resource();
}

template<typename TChar>
auto BasicString<TChar>::get_data() const noexcept -> const Char*
{
	return data_;
}

template<typename TChar>
auto BasicString<TChar>::get_data() noexcept -> Char*
{
	return data_;
}

template<typename TChar>
std::intptr_t BasicString<TChar>::get_capacity() const noexcept
{
	return capacity_;
}

template<typename TChar>
std::intptr_t BasicString<TChar>::get_size() const noexcept
{
	return size_;
}

template<typename TChar>
bool BasicString<TChar>::is_empty() const noexcept
{
	return get_size() == 0;
}

template<typename TChar>
auto BasicString<TChar>::begin() const noexcept -> const Char*
{
	return get_data();
}

template<typename TChar>
auto BasicString<TChar>::begin() noexcept -> Char*
{
	return bstone::as_mutable(bstone::as_const(*this).begin());
}

template<typename TChar>
auto BasicString<TChar>::end() const noexcept -> const Char*
{
	return begin() + get_size();
}

template<typename TChar>
auto BasicString<TChar>::end() noexcept -> Char*
{
	return bstone::as_mutable(bstone::as_const(*this).end());
}

template<typename TChar>
auto BasicString<TChar>::cbegin() const noexcept -> const Char*
{
	return begin();
}

template<typename TChar>
auto BasicString<TChar>::cend() const noexcept -> const Char*
{
	return end();
}

template<typename TChar>
auto BasicString<TChar>::operator[](std::intptr_t index) const noexcept -> const Char&
{
	BSTONE_ASSERT(index >= 0 && index <= get_size());

	return get_data()[index];
}

template<typename TChar>
auto BasicString<TChar>::operator[](std::intptr_t index) noexcept -> Char&
{
	return bstone::as_mutable(bstone::as_const(*this)[index]);
}

template<typename TChar>
auto BasicString<TChar>::get_front() const noexcept -> const Char&
{
	return (*this)[0];
}

template<typename TChar>
auto BasicString<TChar>::get_front() noexcept -> Char&
{
	return bstone::as_mutable(bstone::as_const(*this).get_front());
}

template<typename TChar>
auto BasicString<TChar>::get_back() const noexcept -> const Char&
{
	return (*this)[get_size() - 1];
}

template<typename TChar>
auto BasicString<TChar>::get_back() noexcept -> Char&
{
	return bstone::as_mutable(bstone::as_const(*this).get_back());
}

template<typename TChar>
void BasicString<TChar>::reserve(std::intptr_t capacity)
{
	BSTONE_ASSERT(capacity >= 0);

	if (capacity <= get_capacity())
	{
		return;
	}

	if (capacity <= local_capacity)
	{
		capacity_ = capacity;
		return;
	}

	auto& memory_resource = get_memory_resource();
	auto new_storage = Storage{memory_resource.template allocate<Char>(capacity + 1), memory_resource};
	std::copy_n(get_data(), get_size() + 1, new_storage.get());
	storage_.swap(new_storage);
	data_ = storage_.get();
	capacity_ = capacity;
}

template<typename TChar>
void BasicString<TChar>::resize(std::intptr_t size)
{
	resize(size, default_char);
}

template<typename TChar>
void BasicString<TChar>::resize(std::intptr_t size, Char ch)
{
	BSTONE_ASSERT(size >= 0);

	if (size == get_size())
	{
		return;
	}

	if (size > get_size())
	{
		reserve(size);
		const auto fill_count = size - get_size();
		std::fill_n(end(), fill_count, ch);
	}

	set_size_and_terminate_without_this(size);
}

template<typename TChar>
void BasicString<TChar>::clear()
{
	set_size_and_terminate_without_this(0);
}

template<typename TChar>
void BasicString<TChar>::swap(BasicString& rhs) noexcept
{
	storage_.swap(rhs.storage_);
	bstone::swop(capacity_, rhs.capacity_);
	bstone::swop(size_, rhs.size_);
	bstone::swop(local_data_, rhs.local_data_);

	data_ = capacity_ <= local_capacity ? local_data_ : storage_.get();
	rhs.data_ = capacity_ <= local_capacity ? rhs.local_data_ : rhs.storage_.get();
}

template<typename TChar>
auto BasicString<TChar>::assign(Char ch, std::intptr_t c_count) -> BasicString&
{
	if (c_count == 0)
	{
		clear();
		return *this;
	}

	reserve(c_count);
	std::fill_n(get_data(), c_count, ch);
	return set_size_and_terminate(c_count);
}

template<typename TChar>
auto BasicString<TChar>::assign(const Char* string) -> BasicString&
{
	return assign(View{string});
}

template<typename TChar>
auto BasicString<TChar>::assign(const Char* chars, std::intptr_t c_count) -> BasicString&
{
	return assign(View{chars, c_count});
}

template<typename TChar>
auto BasicString<TChar>::assign(const BasicString& string) -> BasicString&
{
	return assign(string.get_view());
}

template<typename TChar>
auto BasicString<TChar>::assign(const BasicString& string, std::intptr_t s_index) -> BasicString&
{
	return assign(string.get_view(s_index));
}

template<typename TChar>
auto BasicString<TChar>::assign(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) -> BasicString&
{
	return assign(string.get_view(s_index, s_count));
}

template<typename TChar>
auto BasicString<TChar>::assign(BasicString&& string) noexcept -> BasicString&
{
	swap(string);
	return *this;
}

template<typename TChar>
auto BasicString<TChar>::assign(View view) -> BasicString&
{
	if (view.is_empty())
	{
		clear();
		return *this;
	}

	const auto string_view_size = view.get_size();
	reserve(string_view_size);
	std::copy_n(view.get_data(), string_view_size, get_data());
	return set_size_and_terminate(string_view_size);
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::assign(TIter iter_begin, TIter iter_end) -> BasicString&
{
	using Tag = GetIteratorTagT<TIter>;
	return assign(iter_begin, iter_end, Tag{});
}

template<typename TChar>
auto BasicString<TChar>::assign(std::initializer_list<Char> list) -> BasicString&
{
	return assign(View{list.begin(), static_cast<std::intptr_t>(list.size())});
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(Char ch)
{
	return append(View{&ch, 1});
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(Char ch, std::intptr_t c_count)
{
	BSTONE_ASSERT(c_count >= 0);

	if (c_count == 0)
	{
		return *this;
	}

	const auto new_size = get_size() + c_count;
	reserve(new_size);
	std::fill_n(end(), c_count, ch);
	return set_size_and_terminate(new_size);
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(const Char* string)
{
	return append(View{string});
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(const Char* chars, std::intptr_t c_count)
{
	return append(View{chars, c_count});
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(const BasicString& string)
{
	return append(string.get_view());
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(const BasicString& string, std::intptr_t s_index)
{
	return append(string.get_view(s_index));
}

template<typename TChar>
BasicString<TChar>& BasicString<TChar>::append(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count)
{
	return append(string.get_view(s_index, s_count));
}

template<typename TChar>
auto BasicString<TChar>::append(View view) -> BasicString&
{
	if (view.is_empty())
	{
		return *this;
	}

	const auto string_view_size = view.get_size();
	const auto new_size = get_size() + string_view_size;
	reserve(new_size);
	std::copy_n(view.get_data(), string_view_size, end());
	return set_size_and_terminate(new_size);
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::append(TIter iter_begin, TIter iter_end) -> BasicString&
{
	using Tag = GetIteratorTagT<TIter>;
	return append(iter_begin, iter_end, Tag{});
}

template<typename TChar>
auto BasicString<TChar>::append(std::initializer_list<Char> list) -> BasicString&
{
	return append(View{list.begin(), static_cast<std::intptr_t>(list.size())});
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, Char ch) -> BasicString&
{
	return insert(index, &ch, 1);
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, Char ch, std::intptr_t c_count) -> BasicString&
{
	BSTONE_ASSERT(c_count >= 0);

	if (c_count == 0)
	{
		return *this;
	}

	const auto new_size = get_size() + c_count;

	if (new_size <= get_capacity())
	{
		copy_overlapped(index, c_count, index + c_count);
		std::fill_n(&(*this)[index], c_count, ch);
	}
	else
	{
		auto& memory_resource = get_memory_resource();
		auto new_storage = Storage{memory_resource.template allocate<Char>(new_size + 1), memory_resource};
		std::copy_n(get_data(), index, new_storage.get());
		std::fill_n(&new_storage[index], c_count, ch);
		std::copy_n(&(*this)[index], get_size() - index, &new_storage[index + c_count]);
		storage_.swap(new_storage);
		data_ = storage_.get();
		capacity_ = new_size;
		size_ = new_size;
	}

	return set_size_and_terminate(new_size);
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, const Char* string) -> BasicString&
{
	return insert(index, View{string});
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, const Char* chars, std::intptr_t c_count) -> BasicString&
{
	return insert(index, View{chars, c_count});
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, const BasicString& string) -> BasicString&
{
	return insert(index, string.get_view());
}

template<typename TChar>
auto BasicString<TChar>::insert(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index) -> BasicString&
{
	return insert(index, string.get_view(s_index));
}

template<typename TChar>
auto BasicString<TChar>::insert(
	std::intptr_t index,
	const BasicString& string,
	std::intptr_t s_index,
	std::intptr_t s_count) -> BasicString&
{
	return insert(index, string.get_view(s_index, s_count));
}

template<typename TChar>
auto BasicString<TChar>::insert(std::intptr_t index, View view) -> BasicString&
{
	if (view.is_empty())
	{
		return *this;
	}

	const auto string_view_size = view.get_size();
	const auto new_size = get_size() + string_view_size;

	if (new_size <= get_capacity())
	{
		copy_overlapped(index, string_view_size, index + string_view_size);
		std::copy_n(view.get_data(), string_view_size, &(*this)[index]);
	}
	else
	{
		auto& memory_resource = get_memory_resource();
		auto new_storage = Storage{memory_resource.template allocate<Char>(new_size + 1), memory_resource};
		std::copy_n(get_data(), index, new_storage.get());
		std::copy_n(view.get_data(), string_view_size, &new_storage[index]);
		std::copy_n(&(*this)[index], size_ - index, &new_storage[index + string_view_size]);
		storage_.swap(new_storage);
		data_ = storage_.get();
		capacity_ = new_size;
		size_ = new_size;
	}

	return set_size_and_terminate(new_size);
}

template<typename TChar>
template<typename TSrcIter>
auto BasicString<TChar>::insert(const Char* dst_iter, TSrcIter src_iter_begin, TSrcIter src_iter_end) -> Char*
{
	using Tag = GetIteratorTagT<TSrcIter>;
	return insert(dst_iter, src_iter_begin, src_iter_end, Tag{});
}

template<typename TChar>
auto BasicString<TChar>::insert(const Char* dst_iter, std::initializer_list<Char> src_chars) -> Char*
{
	return insert(dst_iter, src_chars.begin(), src_chars.end(), std::random_access_iterator_tag{});
}

template<typename TChar>
auto BasicString<TChar>::erase(std::intptr_t index) -> BasicString&
{
	return erase(index, get_size() - index);
}

template<typename TChar>
auto BasicString<TChar>::erase(std::intptr_t index, std::intptr_t count) -> BasicString&
{
	BSTONE_ASSERT(index >= 0 && count >= 0);
	BSTONE_ASSERT(index + count <= get_size());

	if (count == 0)
	{
		return *this;
	}

	const auto copy_count = get_size() - index - count;
	std::copy_n(&(*this)[index + count], copy_count, &(*this)[index]);
	return set_size_and_terminate(get_size() - count);
}

template<typename TChar>
auto BasicString<TChar>::erase(const Char* iter) -> Char*
{
	const auto index = iter - begin();
	return erase(index).begin() + index;
}

template<typename TChar>
auto BasicString<TChar>::erase(const Char* iter_begin, const Char* iter_end) -> Char*
{
	const auto index = iter_begin - begin();
	const auto count = iter_end - iter_begin;
	return erase(index, count).begin() + index;
}

template<typename TChar>
void BasicString<TChar>::push_back(Char ch)
{
	append(ch);
}

template<typename TChar>
auto BasicString<TChar>::pop_back() -> Char
{
	const auto ch = get_back();
	set_size_and_terminate_without_this(get_size() - 1);
	return ch;
}

template<typename TChar>
auto BasicString<TChar>::get_substring(std::intptr_t index) const -> BasicString
{
	return get_substring(index, get_size() - index);
}

template<typename TChar>
auto BasicString<TChar>::get_substring(std::intptr_t index, std::intptr_t count) const -> BasicString
{
	BSTONE_ASSERT(index >= 0 && count >= 0);
	BSTONE_ASSERT(index + count <= get_size());

	return BasicString<TChar>{get_data() + index, count, get_memory_resource()};
}

template<typename TChar>
auto BasicString<TChar>::get_view() const noexcept -> View
{
	return View{get_data(), get_size()};
}

template<typename TChar>
auto BasicString<TChar>::get_view(std::intptr_t index) const noexcept -> View
{
	BSTONE_ASSERT(index >= 0 && index <= get_size());

	return View{get_data() + index, get_size() - index};
}

template<typename TChar>
auto BasicString<TChar>::get_view(
	std::intptr_t index, std::intptr_t count) const noexcept -> View
{
	BSTONE_ASSERT(index >= 0 && count >= 0);
	BSTONE_ASSERT(index + count <= get_size());

	return View{get_data() + index, count};
}

template<typename TChar>
int BasicString<TChar>::compare(const Char* string) const noexcept
{
	return compare(View{string});
}

template<typename TChar>
int BasicString<TChar>::compare(const Char* chars, std::intptr_t c_count) const noexcept
{
	return compare(View{chars, c_count});
}

template<typename TChar>
int BasicString<TChar>::compare(const BasicString& string) const noexcept
{
	return compare(string.get_view());
}

template<typename TChar>
int BasicString<TChar>::compare(View view) const noexcept
{
	return get_view().compare(view);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(Char ch) const noexcept
{
	return get_view().index_of(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(const Char* substring) const noexcept
{
	return index_of(View{substring});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(const Char* subchars, std::intptr_t s_count) const noexcept
{
	return index_of(View{subchars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(const BasicString& substring) const noexcept
{
	return index_of(substring.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(View subview) const noexcept
{
	return get_view().index_of(subview);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(std::intptr_t index, Char ch) const noexcept
{
	return index_of_generic(index, ch, &View::index_of);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(std::intptr_t index, const Char* substring) const noexcept
{
	return index_of(index, View{substring});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(
	std::intptr_t index, const Char* subchars, std::intptr_t s_count) const noexcept
{
	return index_of(index, View{subchars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(std::intptr_t index, const BasicString& substring) const noexcept
{
	return index_of(index, substring.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of(std::intptr_t index, View subview) const noexcept
{
	return index_of_generic(index, subview, &View::index_of);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(Char ch) const noexcept
{
	return get_view().last_index_of(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(const Char* substring) const noexcept
{
	return last_index_of(View{substring});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(const Char* subchars, std::intptr_t s_count) const noexcept
{
	return last_index_of(View{subchars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(const BasicString& substring) const noexcept
{
	return last_index_of(substring.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(View subview) const noexcept
{
	return get_view().last_index_of(subview);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(std::intptr_t index, Char ch) const noexcept
{
	return index_of_generic(index, ch, &View::last_index_of);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(std::intptr_t index, const Char* substring) const noexcept
{
	return last_index_of(index, View{substring});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(
	std::intptr_t index, const Char* subchars, std::intptr_t s_count) const noexcept
{
	return last_index_of(index, View{subchars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(std::intptr_t index, const BasicString& substring) const noexcept
{
	return last_index_of(index, substring.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of(
	std::intptr_t index, View subview) const noexcept
{
	return index_of_generic(index, subview, &View::last_index_of);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(Char ch) const noexcept
{
	return index_of(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(const Char* string) const noexcept
{
	return index_of_any(View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(const Char* chars, std::intptr_t s_count) const noexcept
{
	return index_of_any(View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(const BasicString& string) const noexcept
{
	return index_of_any(string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(const BasicString& string, std::intptr_t s_index) const noexcept
{
	return index_of_any(string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return index_of_any(string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(View view) const noexcept
{
	return get_view().index_of_any(view);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(std::intptr_t index, Char ch) const noexcept
{
	return index_of(index, ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(std::intptr_t index, const Char* string) const noexcept
{
	return index_of_any(index, View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(
	std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept
{
	return index_of_any(index, View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(std::intptr_t index, const BasicString& string) const noexcept
{
	return index_of_any(index, string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept
{
	return index_of_any(index, string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return index_of_any(index, string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_of_any(std::intptr_t index, View view) const noexcept
{
	return index_of_generic(index, view, &View::index_of_any);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(Char ch) const noexcept
{
	return get_view().index_not_of_any(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(const Char* string) const noexcept
{
	return index_not_of_any(View(string));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(const Char* chars, std::intptr_t s_count) const noexcept
{
	return index_not_of_any(View(chars, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(const BasicString& string) const noexcept
{
	return index_not_of_any(string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(
	const BasicString& string, std::intptr_t s_index) const noexcept
{
	return index_not_of_any(string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return index_not_of_any(string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(View view) const noexcept
{
	return get_view().index_not_of_any(view);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(std::intptr_t index, Char ch) const noexcept
{
	return index_of_generic(index, ch, &View::index_not_of_any);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(std::intptr_t index, const Char* string) const noexcept
{
	return index_not_of_any(index, View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(
	std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept
{
	return index_not_of_any(index, View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(std::intptr_t index, const BasicString& string) const noexcept
{
	return index_not_of_any(index, string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept
{
	return index_not_of_any(index, string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(
	std::intptr_t index,
	const BasicString& string,
	std::intptr_t s_index,
	std::intptr_t s_count) const noexcept
{
	return index_not_of_any(index, string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::index_not_of_any(std::intptr_t index, View view) const noexcept
{
	return index_of_generic(index, view, &View::index_not_of_any);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(Char ch) const noexcept
{
	return last_index_of(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(const Char* string) const noexcept
{
	return last_index_of_any(View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(const Char* chars, std::intptr_t s_count) const noexcept
{
	return last_index_of_any(View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(const BasicString& string) const noexcept
{
	return last_index_of_any(string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	const BasicString& string, std::intptr_t s_index) const noexcept
{
	return last_index_of_any(string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return last_index_of_any(string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(View view) const noexcept
{
	return get_view().last_index_of_any(view);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(std::intptr_t index, Char ch) const noexcept
{
	return last_index_of(index, ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(std::intptr_t index, const Char* string) const noexcept
{
	return last_index_of_any(index, View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept
{
	return last_index_of_any(index, View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	std::intptr_t index, const BasicString& string) const noexcept
{
	return last_index_of_any(index, string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept
{
	return last_index_of_any(index, string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return last_index_of_any(index, string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_of_any(std::intptr_t index, View view) const noexcept
{
	return index_of_generic(index, view, &View::last_index_of_any);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(Char ch) const noexcept
{
	return get_view().last_index_not_of_any(ch);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(const Char* string) const noexcept
{
	return last_index_not_of_any(View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	const Char* chars, std::intptr_t s_count) const noexcept
{
	return last_index_not_of_any(View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(const BasicString& string) const noexcept
{
	return last_index_not_of_any(string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	const BasicString& string, std::intptr_t s_index) const noexcept
{
	return last_index_not_of_any(string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return last_index_not_of_any(string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(View view) const noexcept
{
	return get_view().last_index_not_of_any(view);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(std::intptr_t index, Char ch) const noexcept
{
	return index_of_generic(index, ch, &View::last_index_not_of_any);
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(std::intptr_t index, const Char* string) const noexcept
{
	return last_index_not_of_any(index, View{string});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	std::intptr_t index, const Char* chars, std::intptr_t s_count) const noexcept
{
	return last_index_not_of_any(index, View{chars, s_count});
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	std::intptr_t index, const BasicString& string) const noexcept
{
	return last_index_not_of_any(index, string.get_view());
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index) const noexcept
{
	return last_index_not_of_any(index, string.get_view(s_index));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(
	std::intptr_t index, const BasicString& string, std::intptr_t s_index, std::intptr_t s_count) const noexcept
{
	return last_index_not_of_any(index, string.get_view(s_index, s_count));
}

template<typename TChar>
std::intptr_t BasicString<TChar>::last_index_not_of_any(std::intptr_t index, View view) const noexcept
{
	return index_of_generic(index, view, &View::last_index_not_of_any);
}

template<typename TChar>
bool BasicString<TChar>::contains(Char ch) const noexcept
{
	return get_view().contains(ch);
}

template<typename TChar>
bool BasicString<TChar>::contains(const Char* substring) const noexcept
{
	return contains(View{substring});
}

template<typename TChar>
bool BasicString<TChar>::contains(const Char* subchars, std::intptr_t s_count) const noexcept
{
	return contains(View{subchars, s_count});
}

template<typename TChar>
bool BasicString<TChar>::contains(const BasicString& substring) const noexcept
{
	return contains(substring.get_view());
}

template<typename TChar>
bool BasicString<TChar>::contains(View subview) const noexcept
{
	return get_view().contains(subview);
}

template<typename TChar>
bool BasicString<TChar>::starts_with(Char ch) const noexcept
{
	return get_view().starts_with(ch);
}

template<typename TChar>
bool BasicString<TChar>::starts_with(const Char* substring) const noexcept
{
	return starts_with(View{substring});
}

template<typename TChar>
bool BasicString<TChar>::starts_with(const Char* subchars, std::intptr_t s_count) const noexcept
{
	return starts_with(View{subchars, s_count});
}

template<typename TChar>
bool BasicString<TChar>::starts_with(const BasicString& substring) const noexcept
{
	return starts_with(substring.get_view());
}

template<typename TChar>
bool BasicString<TChar>::starts_with(View subview) const noexcept
{
	return get_view().starts_with(subview);
}

template<typename TChar>
bool BasicString<TChar>::ends_with(Char ch) const noexcept
{
	return get_view().ends_with(ch);
}

template<typename TChar>
bool BasicString<TChar>::ends_with(const Char* substring) const noexcept
{
	return ends_with(View{substring});
}

template<typename TChar>
bool BasicString<TChar>::ends_with(const Char* subchars, std::intptr_t s_count) const noexcept
{
	return ends_with(View{subchars, s_count});
}

template<typename TChar>
bool BasicString<TChar>::ends_with(const BasicString& substring) const noexcept
{
	return ends_with(substring.get_view());
}

template<typename TChar>
bool BasicString<TChar>::ends_with(View subview) const noexcept
{
	return get_view().ends_with(subview);
}

template<typename TChar>
auto BasicString<TChar>::make_with_capacity(std::intptr_t capacity) -> BasicString
{
	return BasicString<TChar>{capacity, StringCapacityTag{}};
}

template<typename TChar>
auto BasicString<TChar>::make_with_fill(Char value, std::intptr_t count) -> BasicString
{
	return BasicString<TChar>{value, count, StringFillTag{}};
}

template<typename TChar>
void BasicString<TChar>::terminate_with_null() noexcept
{
	*end() = default_char;
}

template<typename TChar>
void BasicString<TChar>::set_size_and_terminate_without_this(std::intptr_t size)
{
	size_ = size;
	terminate_with_null();
}

template<typename TChar>
auto BasicString<TChar>::set_size_and_terminate(std::intptr_t size) -> BasicString&
{
	set_size_and_terminate_without_this(size);
	return *this;
}

template<typename TChar>
void BasicString<TChar>::copy_overlapped(std::intptr_t old_index, std::intptr_t count, std::intptr_t new_index)
{
	const auto old_chars = get_data() + old_index;
	const auto new_chars = get_data() + new_index;

	for (auto i = decltype(count){}; i < count; ++i)
	{
		new_chars[i] = old_chars[i];
	}
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::assign(TIter iter_begin, TIter iter_end, std::input_iterator_tag) -> BasicString&
{
	auto new_string = BasicString{get_memory_resource()};
	new_string.reserve(get_capacity());

	for (; iter_begin != iter_end; ++iter_begin)
	{
		new_string.append(*iter_begin);
	}

	swap(new_string);
	return *this;
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::assign(
	TIter iter_begin, TIter iter_end, std::random_access_iterator_tag) -> BasicString&
{
	const auto size = iter_end - iter_begin;
	reserve(size);
	std::copy_n(&(*iter_begin), size, get_data());
	return set_size_and_terminate(size);
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::append(TIter iter_begin, TIter iter_end, std::input_iterator_tag) -> BasicString&
{
	auto suffix = BasicString{get_memory_resource()};

	for (; iter_begin != iter_end; ++iter_begin)
	{
		suffix.append(*iter_begin);
	}

	const auto new_size = get_size() + suffix.get_size();
	reserve(new_size);
	std::copy_n(suffix.cbegin(), suffix.get_size(), end());
	return set_size_and_terminate(new_size);
}

template<typename TChar>
template<typename TIter>
auto BasicString<TChar>::append(
	TIter iter_begin, TIter iter_end, std::random_access_iterator_tag) -> BasicString&
{
	const auto size = iter_end - iter_begin;
	const auto new_size = get_size() + size;
	reserve(new_size);
	std::copy_n(&(*iter_begin), size, end());
	return set_size_and_terminate(new_size);
}

template<typename TChar>
template<typename TSrcIter>
auto BasicString<TChar>::insert(
	const Char* dst_iter,
	TSrcIter src_iter_begin,
	TSrcIter src_iter_end,
	std::input_iterator_tag) -> Char*
{
	auto substring = BasicString{get_memory_resource()};

	for (; src_iter_begin != src_iter_end; ++src_iter_begin)
	{
		const auto ch = *src_iter_begin;
		substring.push_back(ch);
	}

	const auto index = dst_iter - get_data();
	insert(index, substring);
	return get_data() + index;
}

template<typename TChar>
template<typename TSrcIter>
auto BasicString<TChar>::insert(
	const Char* dst_iter,
	TSrcIter src_iter_begin,
	TSrcIter src_iter_end,
	std::random_access_iterator_tag) -> Char*
{
	const auto index = dst_iter - get_data();
	const auto count = src_iter_end - src_iter_begin;
	const auto string = &(*src_iter_begin);
	insert(index, string, count);
	return get_data() + index;
}

template<typename TChar>
template<typename TSubValue>
std::intptr_t BasicString<TChar>::index_of_generic(
	std::intptr_t index,
	TSubValue sub_value,
	IndexOfGenericFunc<TSubValue> func) const noexcept
{
	const auto found_index = (get_view(index).*func)(sub_value);

	if (found_index < 0)
	{
		return found_index;
	}

	return index + found_index;
}

// ==========================================================================

template<typename TChar>
inline BasicString<TChar> operator+(const BasicString<TChar>& lhs, TChar rhs)
{
	const auto new_size = lhs.get_size() + 1;
	auto string = BasicString<TChar>{lhs.get_memory_resource()};
	string.reserve(new_size);
	return string.append(lhs).append(rhs);
}

template<typename TChar>
inline BasicString<TChar> operator+(TChar lhs, const BasicString<TChar>& rhs)
{
	return rhs + lhs;
}

// --------------------------------------------------------------------------

template<typename TChar>
inline BasicString<TChar> operator+(const BasicString<TChar>& lhs, const TChar* rhs)
{
	const auto rhs_size = char_traits::get_size(rhs);
	const auto new_size = lhs.get_size() + rhs_size;
	auto string = BasicString<TChar>{lhs.get_memory_resource()};
	string.reserve(new_size);
	return string.append(lhs).append(rhs, rhs_size);
}

template<typename TChar>
inline BasicString<TChar> operator+(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs + lhs;
}

// --------------------------------------------------------------------------

template<typename TChar>
inline BasicString<TChar> operator+(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	const auto new_size = lhs.get_size() + rhs.get_size();
	auto string = BasicString<TChar>{lhs.get_memory_resource()};
	string.reserve(new_size);
	return string.append(lhs).append(rhs);
}

// ==========================================================================

template<typename TChar>
inline BasicString<TChar>& operator+=(BasicString<TChar>& lhs, TChar rhs)
{
	return lhs.append(rhs);
}

template<typename TChar>
inline BasicString<TChar>& operator+=(BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.append(rhs);
}

template<typename TChar>
inline BasicString<TChar>& operator+=(BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.append(rhs);
}

// ==========================================================================

template<typename TChar>
inline bool operator==(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.compare(rhs) == 0;
}

template<typename TChar>
inline bool operator==(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs.compare(lhs) == 0;
}

template<typename TChar>
inline bool operator==(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.compare(rhs) == 0;
}

// ==========================================================================

template<typename TChar>
inline bool operator!=(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return !(lhs == rhs);
}

template<typename TChar>
inline bool operator!=(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs != lhs;
}

template<typename TChar>
inline bool operator!=(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return !(lhs == rhs);
}

// ==========================================================================

template<typename TChar>
inline bool operator<(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.compare(rhs) < 0;
}

template<typename TChar>
inline bool operator<(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs.compare(lhs) >= 0;
}

template<typename TChar>
inline bool operator<(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.compare(rhs) < 0;
}

// ==========================================================================

template<typename TChar>
inline bool operator<=(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.compare(rhs) <= 0;
}

template<typename TChar>
inline bool operator<=(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs.compare(lhs) >= 0;
}

template<typename TChar>
inline bool operator<=(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.compare(rhs) <= 0;
}

// ==========================================================================

template<typename TChar>
inline bool operator>(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.compare(rhs) > 0;
}

template<typename TChar>
inline bool operator>(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs.compare(lhs) < 0;
}

template<typename TChar>
inline bool operator>(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.compare(rhs) > 0;
}

// ==========================================================================

template<typename TChar>
inline bool operator>=(const BasicString<TChar>& lhs, const TChar* rhs)
{
	return lhs.compare(rhs) >= 0;
}

template<typename TChar>
inline bool operator>=(const TChar* lhs, const BasicString<TChar>& rhs)
{
	return rhs.compare(lhs) <= 0;
}

template<typename TChar>
inline bool operator>=(const BasicString<TChar>& lhs, const BasicString<TChar>& rhs)
{
	return lhs.compare(rhs) >= 0;
}

// ==========================================================================

using String = BasicString<char>;
using U16String = BasicString<char16_t>;
using U32String = BasicString<char32_t>;

} // namespace bstone

#endif // BSTONE_STRING_INCLUDED
