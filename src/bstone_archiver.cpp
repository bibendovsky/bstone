/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_archiver.h"

#include <string>
#include <vector>

#include "bstone_crc32.h"
#include "bstone_endian.h"
#include "bstone_stream.h"
#include "bstone_un_value.h"


namespace bstone
{


ArchiverException::ArchiverException(
	const char* const message)
	:
	Exception{message}
{
}

ArchiverException::~ArchiverException() = default;


namespace
{


class ArchiverExceptionImpl final :
	public ArchiverException
{
public:
	ArchiverExceptionImpl(
		const char* const message);

	~ArchiverExceptionImpl() override;
}; // ArchiverException


class ArchiverImpl final :
	public Archiver
{
public:
	ArchiverImpl();

	~ArchiverImpl() override = default;


	void initialize(
		StreamPtr stream) override;

	void uninitialize() override;

	bool is_initialized() const override;


	void throw_exception(
		const char* const message) const override;


	bool read_bool() override;

	char read_char() override;


	std::int8_t read_int8() override;

	std::uint8_t read_uint8() override;


	std::int16_t read_int16() override;

	std::uint16_t read_uint16() override;


	std::int32_t read_int32() override;

	std::uint32_t read_uint32() override;


	void read_char_array(
		char* items_char,
		const int item_count) override;

	void read_int8_array(
		std::int8_t* items_int8,
		const int item_count) override;

	void read_uint8_array(
		std::uint8_t* items_uint8,
		const int item_count) override;


	void read_int16_array(
		std::int16_t* items_int16,
		const int item_count) override;

	void read_uint16_array(
		std::uint16_t* items_uint16,
		const int item_count) override;


	void read_string(
		const int max_string_length,
		char* const string,
		int& string_length) override;


	void read_checksum() override;


	void write_bool(
		const bool value_bool) override;

	void write_char(
		const char value_char) override;

	void write_int8(
		const std::int8_t value_int8) override;

	void write_uint8(
		const std::uint8_t value_uint8) override;


	void write_int16(
		const std::int16_t value_int16) override;

	void write_uint16(
		const std::uint16_t value_uint16) override;


	void write_int32(
		const std::int32_t value_int32) override;

	void write_uint32(
		const std::uint32_t value_uint32) override;


	void write_char_array(
		const char* const items_char,
		const int item_count) override;

	void write_int8_array(
		const std::int8_t* const items_int8,
		const int item_count) override;

	void write_uint8_array(
		const std::uint8_t* const items_uint8,
		const int item_count) override;


	void write_int16_array(
		const std::int16_t* const items_int16,
		const int item_count) override;

	void write_uint16_array(
		const std::uint16_t* const items_uint16,
		const int item_count) override;


	void write_string(
		const char* const string,
		const int string_length) override;


	void write_checksum() override;


private:
	using Buffer = std::vector<UnValue<char>>;


	bool is_initialized_;
	bool is_stream_readable_;
	bool is_stream_writable_;
	Crc32 crc32_;
	StreamPtr stream_;
	Buffer buffer_;


	template<typename T>
	T read_integer(
		const bool is_checksum = false)
	{
		if (!is_initialized_)
		{
			throw ArchiverExceptionImpl{"Not initialized."};
		}

		if (!is_stream_readable_)
		{
			throw ArchiverExceptionImpl{"Stream is not readable."};
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));

		T value;

		const auto read_result = stream_->read(&value, value_size);

		if (read_result != value_size)
		{
			throw ArchiverExceptionImpl{"Failed to read an integer value."};
		}

		if (!is_checksum)
		{
			crc32_.update(&value, value_size);
		}

		const auto result = (Endian::should_be_swapped<T>() ? Endian::little(value) : value);

		return result;
	}

	template<typename T>
	void write_integer(
		const T integer_value,
		const bool is_checksum = false)
	{
		if (!is_initialized_)
		{
			throw ArchiverExceptionImpl{"Not initialized."};
		}

		if (!is_stream_writable_)
		{
			throw ArchiverExceptionImpl{"Stream is not writable."};
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));

		if (!is_checksum)
		{
			crc32_.update(&integer_value, value_size);
		}

		const auto value = (Endian::should_be_swapped<T>() ? Endian::little(integer_value) : integer_value);

		const auto write_result = stream_->write(&value, value_size);

		if (!write_result)
		{
			throw ArchiverExceptionImpl{"Failed to write an integer value."};
		}
	}


	template<typename T>
	void read_integer_array(
		T* items,
		const int item_count)
	{
		if (!is_initialized_)
		{
			throw ArchiverExceptionImpl{"Not initialized."};
		}

		if (!items)
		{
			throw ArchiverExceptionImpl{"Null items."};
		}

		if (item_count <= 0)
		{
			throw ArchiverExceptionImpl{"Item count out of range."};
		}

		if (!is_stream_readable_)
		{
			throw ArchiverExceptionImpl{"Stream is not readable."};
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));
		const auto items_size = value_size * item_count;

		const auto read_result = stream_->read(items, items_size);

		if (read_result != items_size)
		{
			throw ArchiverExceptionImpl{"Failed to read an array of integer values."};
		}

		if (Endian::should_be_swapped<T>())
		{
			for (int i_item = 0; i_item < item_count; ++i_item)
			{
				Endian::little_i(items[i_item]);
			}
		}

		crc32_.update(items, items_size);
	}

	template<typename T>
	void write_integer_array(
		const T* const items,
		const int item_count)
	{
		if (!is_initialized_)
		{
			throw ArchiverExceptionImpl{"Not initialized."};
		}

		if (!items)
		{
			throw ArchiverExceptionImpl{"Null items."};
		}

		if (item_count <= 0)
		{
			throw ArchiverExceptionImpl{"Item count out of range."};
		}

		if (!is_stream_writable_)
		{
			throw ArchiverExceptionImpl{"Stream is not writable."};
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));
		const auto items_size = value_size * item_count;

		crc32_.update(items, items_size);

		if (Endian::should_be_swapped<T>())
		{
			if (buffer_.size() < static_cast<std::size_t>(items_size))
			{
				buffer_.resize(items_size);
			}

			auto dst_items = reinterpret_cast<T*>(buffer_.data());

			for (int i_item = 0; i_item < item_count; ++i_item)
			{
				dst_items[i_item] = Endian::little(items[i_item]);
			}

			const auto write_result = stream_->write(dst_items, items_size);

			if (!write_result)
			{
				throw ArchiverExceptionImpl{"Failed to write an array of integer values."};
			}
		}
		else
		{
			const auto write_result = stream_->write(items, items_size);

			if (!write_result)
			{
				throw ArchiverExceptionImpl{"Failed to write an array of integer values."};
			}
		}
	}
}; // ArchiverImpl


ArchiverImpl::ArchiverImpl()
	:
	is_initialized_{},
	is_stream_readable_{},
	is_stream_writable_{},
	crc32_{},
	stream_{},
	buffer_{}
{
}

void ArchiverImpl::initialize(
	StreamPtr stream)
{
	if (is_initialized_)
	{
		throw ArchiverExceptionImpl{"Already initialized."};
	}

	if (!stream)
	{
		throw ArchiverExceptionImpl{"Null stream."};
	}

	if (!stream->is_open())
	{
		throw ArchiverExceptionImpl{"Stream is not open."};
	}


	is_initialized_ = true;
	is_stream_readable_ = stream->is_readable();
	is_stream_writable_ = stream->is_writable();
	crc32_.reset();
	stream_ = stream;
	buffer_.clear();
}

void ArchiverImpl::uninitialize()
{
	is_initialized_ = false;
	is_stream_readable_ = false;
	is_stream_writable_ = false;
	crc32_.reset();
	stream_ = nullptr;
	buffer_.clear();
}

bool ArchiverImpl::is_initialized() const
{
	return is_initialized_;
}

void ArchiverImpl::throw_exception(
	const char* const message) const
{
	throw ArchiverExceptionImpl{message};
}

bool ArchiverImpl::read_bool()
{
	const auto value_uint8 = read_integer<std::uint8_t>();

	return value_uint8 != 0;
}

char ArchiverImpl::read_char()
{
	const auto value_char = read_integer<char>();

	return value_char;
}

std::int8_t ArchiverImpl::read_int8()
{
	const auto value_int8 = read_integer<std::int8_t>();

	return value_int8;
}

std::uint8_t ArchiverImpl::read_uint8()
{
	const auto value_uint8 = read_integer<std::uint8_t>();

	return value_uint8;
}

std::int16_t ArchiverImpl::read_int16()
{
	const auto value_int16 = read_integer<std::int16_t>();

	return value_int16;
}

std::uint16_t ArchiverImpl::read_uint16()
{
	const auto value_uint16 = read_integer<std::uint16_t>();

	return value_uint16;
}

std::int32_t ArchiverImpl::read_int32()
{
	const auto value_int32 = read_integer<std::int32_t>();

	return value_int32;
}

std::uint32_t ArchiverImpl::read_uint32()
{
	const auto value_uint32 = read_integer<std::uint32_t>();

	return value_uint32;
}

void ArchiverImpl::read_char_array(
	char* items,
	const int item_count)
{
	read_integer_array(items, item_count);
}

void ArchiverImpl::read_int8_array(
	std::int8_t* items_int8,
	const int item_count)
{
	read_integer_array(items_int8, item_count);
}

void ArchiverImpl::read_uint8_array(
	std::uint8_t* items_uint8,
	const int item_count)
{
	read_integer_array(items_uint8, item_count);
}

void ArchiverImpl::read_int16_array(
	std::int16_t* items_int16,
	const int item_count)
{
	read_integer_array(items_int16, item_count);
}

void ArchiverImpl::read_uint16_array(
	std::uint16_t* items_uint16,
	const int item_count)
{
	read_integer_array(items_uint16, item_count);
}

void ArchiverImpl::read_string(
	const int max_string_length,
	char* const string,
	int& string_length)
{
	if (max_string_length <= 0)
	{
		throw ArchiverExceptionImpl{"Maximum string length out of range."};
	}

	if (!string)
	{
		throw ArchiverExceptionImpl{"Null string."};
	}

	const auto archived_string_length = read_integer<std::int32_t>();

	if (archived_string_length < 0 || archived_string_length > max_string_length)
	{
		throw ArchiverExceptionImpl{"Archived string length out of range."};
	}

	const auto read_result = stream_->read(string, archived_string_length);

	if (read_result != archived_string_length)
	{
		throw ArchiverExceptionImpl{"Failed to read string data."};
	}

	string_length = archived_string_length;
}

void ArchiverImpl::read_checksum()
{
	const auto checksum = read_integer<std::uint32_t>(true);

	if (checksum != crc32_.get_value())
	{
		throw ArchiverExceptionImpl{"Checksum mismatch."};
	}
}

void ArchiverImpl::write_bool(
	const bool value_bool)
{
	write_integer<std::uint8_t>(value_bool);
}

void ArchiverImpl::write_char(
	const char value_char)
{
	write_integer(value_char);
}

void ArchiverImpl::write_int8(
	const std::int8_t value_int8)
{
	write_integer(value_int8);
}

void ArchiverImpl::write_uint8(
	const std::uint8_t value_uint8)
{
	write_integer(value_uint8);
}

void ArchiverImpl::write_int16(
	const std::int16_t value_int16)
{
	write_integer(value_int16);
}

void ArchiverImpl::write_uint16(
	const std::uint16_t value_uint16)
{
	write_integer(value_uint16);
}

void ArchiverImpl::write_int32(
	const std::int32_t value_int32)
{
	write_integer(value_int32);
}

void ArchiverImpl::write_uint32(
	const std::uint32_t value_uint32)
{
	write_integer(value_uint32);
}

void ArchiverImpl::write_char_array(
	const char* const items_char,
	const int item_count)
{
	write_integer_array(items_char, item_count);
}

void ArchiverImpl::write_int8_array(
	const std::int8_t* const items_int8,
	const int item_count)
{
	write_integer_array(items_int8, item_count);
}

void ArchiverImpl::write_uint8_array(
	const std::uint8_t* const items_uint8,
	const int item_count)
{
	write_integer_array(items_uint8, item_count);
}

void ArchiverImpl::write_int16_array(
	const std::int16_t* const items_int16,
	const int item_count)
{
	write_integer_array(items_int16, item_count);
}

void ArchiverImpl::write_uint16_array(
	const std::uint16_t* const items_uint16,
	const int item_count)
{
	write_integer_array(items_uint16, item_count);
}

void ArchiverImpl::write_string(
	const char* const string,
	const int string_length)
{
	if (!string)
	{
		throw ArchiverExceptionImpl{"Null string."};
	}

	if (string_length < 0)
	{
		throw ArchiverExceptionImpl{"String length out of range."};
	}

	write_integer<std::int32_t>(string_length);

	if (string_length == 0)
	{
		return;
	}

	const auto write_result = stream_->write(string, string_length);

	if (!write_result)
	{
		throw ArchiverExceptionImpl{"Failed to write string data."};
	}
}

void ArchiverImpl::write_checksum()
{
	const auto checksum = crc32_.get_value();

	write_integer<std::int32_t>(checksum, true);
}


ArchiverExceptionImpl::ArchiverExceptionImpl(
	const char* const message)
	:
	ArchiverException{message}
{
}

ArchiverExceptionImpl::~ArchiverExceptionImpl() = default;


} // namespace


ArchiverUPtr make_archiver()
{
	return std::make_unique<ArchiverImpl>();
}


} // bstone
