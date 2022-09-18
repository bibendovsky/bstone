/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_BINARY_READER_INCLUDED
#define BSTONE_BINARY_READER_INCLUDED

#include "bstone_stream.h"

namespace bstone
{

class BinaryReader
{
public:
	BinaryReader(Stream* stream = nullptr);
	BinaryReader(BinaryReader&& rhs);
	BinaryReader(const BinaryReader& rhs) = delete;
	BinaryReader& operator=(const BinaryReader& rhs) = delete;

	bool open(Stream* stream);
	// Closes the reader but stream.
	void close();
	bool is_open() const;
	// Reads a signed 8-bit integer value.
	std::int8_t read_s8();
	// Reads an unsigned 8-bit integer value.
	std::uint8_t read_u8();
	// Reads a signed 16-bit integer value.
	std::int16_t read_s16();
	// Reads an unsigned 16-bit integer value.
	std::uint16_t read_u16();
	// Reads a signed 32-bit integer value.
	std::int32_t read_s32();
	// Reads an unsigned 32-bit integer value.
	std::uint32_t read_u32();
	// Reads a signed 64-bit integer value.
	std::int64_t read_s64();
	// Reads an unsigned 64-bit integer value.
	std::uint64_t read_u64();
	// Reads a 32-bit float-point value.
	float read_r32();
	// Reads a 64-bit float-point value.
	double read_r64();
	// Reads a string prepended with signed 32-bit (little-endian) length.
	// Returns empty string on error or when max length reached.
	std::string read_string(int max_length = -1);

	bool read(void* buffer, int count);

	template<typename T>
	bool read(T& value)
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->read(&value, sizeof(T)) == sizeof(T);
	}

	template<typename T, std::size_t N>
	bool read(T (&value)[N])
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->read(value, N * sizeof(T)) == N * sizeof(T);
	}

	// Skips a number of octets forward if count is positive or
	// backward otherwise.
	// Returns false on error.
	bool skip(int count);

	// Returns a current position.
	int get_position() const;

	// Sets a current position to a specified one.
	bool set_position(int position);

private:
	Stream* stream_{};

	template<typename T>
	T read()
	{
		if (!is_open())
		{
			return 0;
		}

		T value;

		if (stream_->read(&value, sizeof(T)) != sizeof(T))
		{
			return 0;
		}

		return value;
	}
}; // BinaryReader

} // bstone

#endif // BSTONE_BINARY_READER_INCLUDED
