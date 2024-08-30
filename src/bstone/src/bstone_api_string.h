/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null-terminated API string primitive.

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <memory>
#include <type_traits>

#include "bstone_char_traits.h"
#include "bstone_memory_resource.h"
#include "bstone_utility.h"

namespace bstone {

template<typename TChar>
class ApiString
{
	static_assert(
		std::is_integral<TChar>::value && !std::is_same<TChar, bool>::value,
		"Unsupported type.");

public:
	using Char = TChar;

public:
	static constexpr auto char_size = static_cast<std::intptr_t>(sizeof(Char));

public:
	explicit ApiString(MemoryResource& memory_resource);
	ApiString(std::intptr_t capacity, MemoryResource& memory_resource);
	ApiString(const Char* string, MemoryResource& memory_resource);

	MemoryResource& get_memory_resource() const noexcept;

	std::intptr_t get_size() const noexcept;
	void set_size(std::intptr_t size);

	std::intptr_t get_capacity() const noexcept;

	const Char* get_data() const noexcept;
	Char* get_data() noexcept;

	void swap(ApiString& rhs) noexcept;

private:
	using Deleter = MemoryResourceUPtrDeleter<Char[]>;
	using Storage = std::unique_ptr<Char[], Deleter>;

private:
	Storage storage_;
	std::intptr_t capacity_{};
	std::intptr_t size_{};

private:
	static Storage make_storage(
		const Char* string,
		std::intptr_t string_size,
		MemoryResource& memory_resource);
};

// ==========================================================================

template<typename TChar>
ApiString<TChar>::ApiString(MemoryResource& memory_resource)
	:
	storage_{nullptr, memory_resource}
{}

template<typename TChar>
ApiString<TChar>::ApiString(std::intptr_t capacity, MemoryResource& memory_resource)
	:
	ApiString{memory_resource}
{
	const auto byte_count = (capacity + 1) * char_size;
	auto storage = Storage{
		static_cast<Char*>(memory_resource.allocate(byte_count)),
		Deleter{memory_resource}};
	storage[0] = Char{};
	capacity_ = capacity;
	storage_.swap(storage);
}

template<typename TChar>
ApiString<TChar>::ApiString(const Char* string, MemoryResource& memory_resource)
	:
	ApiString{memory_resource}
{
	if (string == nullptr)
	{
		return;
	}

	const auto string_size = char_traits::get_size(string);
	auto storage = make_storage(string, string_size, memory_resource);
	capacity_ = string_size;
	size_ = string_size;
	storage_.swap(storage);
}

template<typename TChar>
MemoryResource& ApiString<TChar>::get_memory_resource() const noexcept
{
	return storage_.get_deleter().get_memory_resource();
}

template<typename TChar>
std::intptr_t ApiString<TChar>::get_size() const noexcept
{
	return size_;
}

template<typename TChar>
void ApiString<TChar>::set_size(std::intptr_t size)
{
	size_ = size;
	storage_[size] = Char{};
}

template<typename TChar>
std::intptr_t ApiString<TChar>::get_capacity() const noexcept
{
	return capacity_;
}

template<typename TChar>
auto ApiString<TChar>::get_data() const noexcept -> const Char*
{
	return storage_.get();
}

template<typename TChar>
auto ApiString<TChar>::get_data() noexcept -> Char*
{
	return storage_.get();
}

template<typename TChar>
void ApiString<TChar>::swap(ApiString& rhs) noexcept
{
	swop(size_, rhs.size_);
	swop(capacity_, rhs.capacity_);
	storage_.swap(rhs.storage_);
}

template<typename TChar>
auto ApiString<TChar>::make_storage(
	const Char* string,
	std::intptr_t string_size,
	MemoryResource& memory_resource) -> Storage
{
	const auto byte_count = (string_size + 1) * char_size;
	auto storage = Storage{
		static_cast<Char*>(memory_resource.allocate(byte_count)),
		Deleter{memory_resource}};
	std::copy_n(string, string_size, static_cast<Char*>(storage.get()));
	storage[string_size] = Char{};
	return storage;
}

} // namespace bstone
