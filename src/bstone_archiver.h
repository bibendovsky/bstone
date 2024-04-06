/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ARCHIVER_INCLUDED
#define BSTONE_ARCHIVER_INCLUDED

#include <cstdint>

#include <algorithm>
#include <type_traits>

#include "bstone_assert.h"
#include "bstone_crc32.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_stream.h"

namespace bstone {

class Archiver
{
public:
	void open(Stream& stream);
	void close() noexcept;
	bool is_open() const noexcept;

	bool read_bool();
	std::int8_t read_int8();
	std::uint8_t read_uint8();
	std::int16_t read_int16();
	std::uint16_t read_uint16();
	std::int32_t read_int32();
	std::uint32_t read_uint32();

	void read_char_array(char* items_char, std::intptr_t item_count);
	void read_int8_array(std::int8_t* items_int8, std::intptr_t item_count);
	void read_uint8_array(std::uint8_t* items_uint8, std::intptr_t item_count);
	void read_uint16_array(std::uint16_t* items_uint16, std::intptr_t item_count);

	std::intptr_t read_string(std::intptr_t max_string_length, char* string);

	void read_checksum();

	void write_bool(bool value_bool);
	void write_int8(std::int8_t value_int8);
	void write_uint8(std::uint8_t value_uint8);
	void write_int16(std::int16_t value_int16);
	void write_uint16(std::uint16_t value_uint16);
	void write_int32(std::int32_t value_int32);
	void write_uint32(std::uint32_t value_uint32);

	void write_char_array(const char* items_char, std::intptr_t item_count);
	void write_int8_array(const std::int8_t* items_int8, std::intptr_t item_count);
	void write_uint8_array(const std::uint8_t* items_uint8, std::intptr_t item_count);
	void write_uint16_array(const std::uint16_t* items_uint16, std::intptr_t item_count);

	void write_string(const char* string, std::intptr_t string_length);

	void write_checksum();

private:
	struct EndianByteSizeTag {};
	struct EndianNonByteSizeTag {};

private:
	Crc32 crc32_{};
	Stream* stream_{};

private:
	template<typename T>
	T read_integer(bool is_checksum = false);

	template<typename T>
	void write_integer(T integer_value, bool is_checksum = false);

	template<typename T>
	void read_integer_array(T* items, std::intptr_t item_count);

	template<typename T>
	void write_integer_array_internal(const T* items, std::intptr_t item_count, EndianByteSizeTag);

	template<typename T>
	void write_integer_array_internal(const T* items, std::intptr_t item_count, EndianNonByteSizeTag);

	template<typename T>
	void write_integer_array(const T* items, std::intptr_t item_count);
};

// --------------------------------------------------------------------------

template<typename T>
T Archiver::read_integer(bool is_checksum)
try {
	BSTONE_ASSERT(is_open());

	constexpr auto value_size = static_cast<std::intptr_t>(sizeof(T));
	auto value = T{};

	stream_->read_exactly(&value, value_size);

	if (!is_checksum)
	{
		crc32_.update(&value, value_size);
	}

	return endian::to_little(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename T>
void Archiver::write_integer(T integer_value, bool is_checksum)
try {
	BSTONE_ASSERT(is_open());

	constexpr auto value_size = static_cast<std::intptr_t>(sizeof(T));

	if (!is_checksum)
	{
		crc32_.update(&integer_value, value_size);
	}

	const auto value = endian::to_little(integer_value);
	stream_->write_exactly(&value, value_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename T>
void Archiver::read_integer_array(T* items, std::intptr_t item_count)
try {
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(items != nullptr);
	BSTONE_ASSERT(item_count > 0);

	constexpr auto value_size = static_cast<std::intptr_t>(sizeof(T));
	const auto items_size = value_size * item_count;
	stream_->read_exactly(items, items_size);

#if BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
	for (auto i_item = std::intptr_t{}; i_item < item_count; ++i_item)
	{
		items[i_item] = endian::swap_bytes(items[i_item]);
	}
#endif

	crc32_.update(items, items_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename T>
void Archiver::write_integer_array_internal(const T* items, std::intptr_t item_count, EndianByteSizeTag)
{
	crc32_.update(items, item_count);
	stream_->write_exactly(items, item_count);
}

template<typename T>
void Archiver::write_integer_array_internal(const T* items, std::intptr_t item_count, EndianNonByteSizeTag)
{
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	const auto items_size = item_size * item_count;

	crc32_.update(items, items_size);

	constexpr auto max_buffer_size = 1024;
	constexpr auto items_per_buffer = max_buffer_size / item_size;
	T buffer[items_per_buffer];
	auto item_offset = std::intptr_t{};

	while (item_count > 0)
	{
		const auto to_write_count = std::min(item_count, items_per_buffer);

		std::transform(
			&items[item_offset],
			&items[item_offset + to_write_count],
			buffer,
			endian::to_little<T>);

		const auto to_write_size = item_size * to_write_count;
		stream_->write_exactly(buffer, to_write_size);

		item_count -= to_write_count;
		item_offset += to_write_count;
	}
}

template<typename T>
void Archiver::write_integer_array(const T* items, std::intptr_t item_count)
try {
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(items != nullptr);
	BSTONE_ASSERT(item_count > 0);

#if BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
	using Tag = std::conditional_t<sizeof(T) == 1, EndianByteSizeTag, EndianNonByteSizeTag>;
	write_integer_array_internal(items, item_count, Tag{});
#else
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	const auto items_size = item_size * item_count;
	crc32_.update(items, items_size);
	stream_->write_exactly(items, items_size);
#endif
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

#endif // BSTONE_ARCHIVER_INCLUDED
