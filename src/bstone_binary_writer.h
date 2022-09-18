/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_BINARY_WRITER_INCLUDED
#define BSTONE_BINARY_WRITER_INCLUDED

#include "bstone_stream.h"

namespace bstone
{

class BinaryWriter final
{
public:
	BinaryWriter(Stream* stream = nullptr);
	BinaryWriter(BinaryWriter&& rhs);
	BinaryWriter(const BinaryWriter& that) = delete;
	BinaryWriter& operator=(const BinaryWriter& that) = delete;

	bool open(Stream* stream);
	// Closes the writer but stream.
	void close();
	bool is_open() const;
	// Writes a signed 8-bit integer value.
	bool write_s8(std::int8_t value);
	// Writes an unsigned 8-bit integer value.
	bool write_u8(std::uint8_t value);
	// Writes a signed 16-bit integer value.
	bool write_s16(std::int16_t value);
	// Writes an unsigned 16-bit integer value.
	bool write_u16(std::uint16_t value);
	// Writes a signed 32-bit integer value.
	bool write_s32(std::int32_t value);
	// Writes an unsigned 32-bit integer value.
	bool write_u32(std::uint32_t value);
	// Writes a signed 64-bit integer value.
	bool write_s64(std::int64_t value);
	// Writes an unsigned 64-bit integer value.
	bool write_u64(std::uint64_t value);
	// Writes a 32-bit float-point value.
	bool write_r32(float value);
	// Writes a 64-bit float-point value.
	bool write_r64(double value);
	bool write(const void* buffer, int count);
	// Writes a string prepended with signed 32-bit (little-endian) length.
	bool write(const std::string& string);

	// Skips a number of octets forward if count is positive or
	// backward otherwise.
	// Returns false on error.
	bool skip(int count);
	// Returns a current position.
	int get_position() const;
	// Sets a current position to a specified one.
	bool set_position(int position);

	template<typename T>
	bool write(const T& value)
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->write(&value, sizeof(T));
	}

	template<typename T, std::size_t N>
	bool write(const T (&value)[N])
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->write(value, N * sizeof(T));
	}

private:
	Stream* stream_{};
}; // BinaryWriter

} // bstone

#endif // BSTONE_BINARY_WRITER_INCLUDED
