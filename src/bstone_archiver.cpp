/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	const char* message) noexcept
	:
	Exception{"ARCHIVER", message}
{
}


namespace
{


class ArchiverImpl final :
	public Archiver
{
public:
	ArchiverImpl();

	~ArchiverImpl() override = default;


	void initialize(
		StreamPtr stream) override;

	void uninitialize() noexcept override;

	bool is_initialized() const noexcept override;


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


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	template<typename T>
	T read_integer(
		const bool is_checksum = false)
	try
	{
		if (!is_initialized_)
		{
			fail("Not initialized.");
		}

		if (!is_stream_readable_)
		{
			fail("Stream is not readable.");
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));

		T value;

		const auto read_result = stream_->read(&value, value_size);

		if (read_result != value_size)
		{
			fail("Failed to read an integer value.");
		}

		if (!is_checksum)
		{
			crc32_.update(&value, value_size);
		}

		const auto result = (Endian::should_be_swapped<T>() ? Endian::little(value) : value);

		return result;
	}
	catch (...)
	{
		fail_nested(__func__);
	}

	template<typename T>
	void write_integer(
		const T integer_value,
		const bool is_checksum = false)
	try
	{
		if (!is_initialized_)
		{
			fail("Not initialized.");
		}

		if (!is_stream_writable_)
		{
			fail("Stream is not writable.");
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
			fail("Failed to write an integer value.");
		}
	}
	catch (...)
	{
		fail_nested(__func__);
	}

	template<typename T>
	void read_integer_array(
		T* items,
		const int item_count)
	try
	{
		if (!is_initialized_)
		{
			fail("Not initialized.");
		}

		if (!items)
		{
			fail("Null items.");
		}

		if (item_count <= 0)
		{
			fail("Item count out of range.");
		}

		if (!is_stream_readable_)
		{
			fail("Stream is not readable.");
		}

		constexpr auto value_size = static_cast<int>(sizeof(T));
		const auto items_size = value_size * item_count;

		const auto read_result = stream_->read(items, items_size);

		if (read_result != items_size)
		{
			fail("Failed to read an array of integer values.");
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
	catch (...)
	{
		fail_nested(__func__);
	}

	template<typename T>
	void write_integer_array(
		const T* const items,
		const int item_count)
	try
	{
		if (!is_initialized_)
		{
			fail("Not initialized.");
		}

		if (!items)
		{
			fail("Null items.");
		}

		if (item_count <= 0)
		{
			fail("Item count out of range.");
		}

		if (!is_stream_writable_)
		{
			fail("Stream is not writable.");
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
				fail("Failed to write an array of integer values.");
			}
		}
		else
		{
			const auto write_result = stream_->write(items, items_size);

			if (!write_result)
			{
				fail("Failed to write an array of integer values.");
			}
		}
	}
	catch (...)
	{
		fail_nested(__func__);
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
try
{
	if (is_initialized_)
	{
		fail("Already initialized.");
	}

	if (!stream)
	{
		fail("Null stream.");
	}

	if (!stream->is_open())
	{
		fail("Stream is not open.");
	}


	is_initialized_ = true;
	is_stream_readable_ = stream->is_readable();
	is_stream_writable_ = stream->is_writable();
	crc32_.reset();
	stream_ = stream;
	buffer_.clear();
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::uninitialize() noexcept
{
	is_initialized_ = false;
	is_stream_readable_ = false;
	is_stream_writable_ = false;
	crc32_.reset();
	stream_ = nullptr;
	buffer_.clear();
}

bool ArchiverImpl::is_initialized() const noexcept
{
	return is_initialized_;
}

bool ArchiverImpl::read_bool()
try
{
	const auto value_uint8 = read_integer<std::uint8_t>();

	return value_uint8 != 0;
}
catch (...)
{
	fail_nested(__func__);
}

char ArchiverImpl::read_char()
try
{
	const auto value_char = read_integer<char>();

	return value_char;
}
catch (...)
{
	fail_nested(__func__);
}

std::int8_t ArchiverImpl::read_int8()
try
{
	const auto value_int8 = read_integer<std::int8_t>();

	return value_int8;
}
catch (...)
{
	fail_nested(__func__);
}

std::uint8_t ArchiverImpl::read_uint8()
try
{
	const auto value_uint8 = read_integer<std::uint8_t>();

	return value_uint8;
}
catch (...)
{
	fail_nested(__func__);
}

std::int16_t ArchiverImpl::read_int16()
try
{
	const auto value_int16 = read_integer<std::int16_t>();

	return value_int16;
}
catch (...)
{
	fail_nested(__func__);
}

std::uint16_t ArchiverImpl::read_uint16()
try
{
	const auto value_uint16 = read_integer<std::uint16_t>();

	return value_uint16;
}
catch (...)
{
	fail_nested(__func__);
}

std::int32_t ArchiverImpl::read_int32()
try
{
	const auto value_int32 = read_integer<std::int32_t>();

	return value_int32;
}
catch (...)
{
	fail_nested(__func__);
}

std::uint32_t ArchiverImpl::read_uint32()
try
{
	const auto value_uint32 = read_integer<std::uint32_t>();

	return value_uint32;
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_char_array(
	char* items,
	const int item_count)
try
{
	read_integer_array(items, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_int8_array(
	std::int8_t* items_int8,
	const int item_count)
try
{
	read_integer_array(items_int8, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_uint8_array(
	std::uint8_t* items_uint8,
	const int item_count)
try
{
	read_integer_array(items_uint8, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_int16_array(
	std::int16_t* items_int16,
	const int item_count)
try
{
	read_integer_array(items_int16, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_uint16_array(
	std::uint16_t* items_uint16,
	const int item_count)
try
{
	read_integer_array(items_uint16, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_string(
	const int max_string_length,
	char* const string,
	int& string_length)
try
{
	if (max_string_length <= 0)
	{
		fail("Maximum string length out of range.");
	}

	if (!string)
	{
		fail("Null string.");
	}

	const auto archived_string_length = read_integer<std::int32_t>();

	if (archived_string_length < 0 || archived_string_length > max_string_length)
	{
		fail("Archived string length out of range.");
	}

	const auto read_result = stream_->read(string, archived_string_length);

	if (read_result != archived_string_length)
	{
		fail("Failed to read string data.");
	}

	string_length = archived_string_length;
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::read_checksum()
try
{
	const auto checksum = read_integer<std::uint32_t>(true);

	if (checksum != crc32_.get_value())
	{
		fail("Checksum mismatch.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_bool(
	const bool value_bool)
try
{
	write_integer<std::uint8_t>(value_bool);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_char(
	const char value_char)
try
{
	write_integer(value_char);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_int8(
	const std::int8_t value_int8)
try
{
	write_integer(value_int8);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_uint8(
	const std::uint8_t value_uint8)
try
{
	write_integer(value_uint8);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_int16(
	const std::int16_t value_int16)
try
{
	write_integer(value_int16);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_uint16(
	const std::uint16_t value_uint16)
try
{
	write_integer(value_uint16);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_int32(
	const std::int32_t value_int32)
try
{
	write_integer(value_int32);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_uint32(
	const std::uint32_t value_uint32)
try
{
	write_integer(value_uint32);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_char_array(
	const char* const items_char,
	const int item_count)
try
{
	write_integer_array(items_char, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_int8_array(
	const std::int8_t* const items_int8,
	const int item_count)
try
{
	write_integer_array(items_int8, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_uint8_array(
	const std::uint8_t* const items_uint8,
	const int item_count)
try
{
	write_integer_array(items_uint8, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_int16_array(
	const std::int16_t* const items_int16,
	const int item_count)
try
{
	write_integer_array(items_int16, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_uint16_array(
	const std::uint16_t* const items_uint16,
	const int item_count)
try
{
	write_integer_array(items_uint16, item_count);
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_string(
	const char* const string,
	const int string_length)
try
{
	if (!string)
	{
		fail("Null string.");
	}

	if (string_length < 0)
	{
		fail("String length out of range.");
	}

	write_integer<std::int32_t>(string_length);

	if (string_length == 0)
	{
		return;
	}

	const auto write_result = stream_->write(string, string_length);

	if (!write_result)
	{
		fail("Failed to write string data.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void ArchiverImpl::write_checksum()
try
{
	const auto checksum = crc32_.get_value();

	write_integer<std::int32_t>(checksum, true);
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void ArchiverImpl::fail(
	const char* message)
{
	throw ArchiverException{message};
}

[[noreturn]]
void ArchiverImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(ArchiverException{message});
}


} // namespace


ArchiverUPtr make_archiver()
{
	return std::make_unique<ArchiverImpl>();
}


[[noreturn]]
void archiver_fail(
	const char* message)
{
	throw ArchiverException{message};
}


} // bstone
