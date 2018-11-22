/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include <limits>
#include <string>
#include <type_traits>
#include <vector>
#include "bstone_archiver.h"
#include "bstone_crc32.h"
#include "bstone_endian.h"
#include "bstone_stream.h"
#include "bstone_un_value.h"


namespace bstone
{


namespace
{


template<typename T>
struct ShouldSwap
{
	static constexpr bool value =
		Endian::is_big() &&
		std::numeric_limits<typename std::make_unsigned_t<T>>::max() > 0xFF;
}; // ShouldSwap


class ArchiverExceptionImpl final :
	public ArchiverException
{
public:
	ArchiverExceptionImpl() = delete;

	ArchiverExceptionImpl(
		const char* const message);

	ArchiverExceptionImpl(
		const std::string& message);

	~ArchiverExceptionImpl() override = default;

	const char* get_message() const override;


private:
	std::string message_;
}; // ArchiverException


class ArchiverImpl final :
	public Archiver
{
public:
	ArchiverImpl();

	~ArchiverImpl() override = default;


	void create();

	void destroy() override;


	void initialize(
		StreamPtr stream) override;

	void uninitialize() override;

	bool is_initialized() const override;


	char read_char() override;

	signed char read_schar() override;

	unsigned char read_uchar() override;


	short read_sshort() override;

	unsigned short read_ushort() override;


	long read_slong() override;

	unsigned long read_ulong() override;


	void read_char_array(
		char* items,
		const int item_count) override;

	void read_schar_array(
		signed char* items,
		const int item_count) override;

	void read_uchar_array(
		unsigned char* items,
		const int item_count) override;


	void read_sshort_array(
		signed short* items,
		const int item_count) override;

	void read_ushort_array(
		unsigned short* items,
		const int item_count) override;


	void read_string(
		const int max_string_length,
		char* const string,
		int& string_length) override;


	void read_checksum() override;


	void write_char(
		const char value_char) override;

	void write_schar(
		const signed char value_schar) override;

	void write_uchar(
		const unsigned char value_uchar) override;


	void write_sshort(
		const short value_sshort) override;

	void write_ushort(
		const unsigned short value_ushort) override;


	void write_slong(
		const long value_slong) override;

	void write_ulong(
		const unsigned long value_ulong) override;


	void write_char_array(
		const char* const items,
		const int item_count) override;

	void write_schar_array(
		const signed char* const items,
		const int item_count) override;

	void write_uchar_array(
		const unsigned char* const items,
		const int item_count) override;


	void write_sshort_array(
		const short* const items,
		const int item_count) override;

	void write_ushort_array(
		const unsigned short* const items,
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

		const auto result = (ShouldSwap<T>::value ? Endian::little(value) : value);

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

		const auto value = (ShouldSwap<T>::value ? Endian::little(integer_value) : integer_value);

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

		if (ShouldSwap<T>::value)
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

		if (ShouldSwap<T>::value)
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


using ArchiverImplUPtr = std::unique_ptr<ArchiverImpl, ArchiverDeleter>;


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

void ArchiverImpl::create()
{
}

void ArchiverImpl::destroy()
{
	uninitialize();

	delete this;
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

char ArchiverImpl::read_char()
{
	const auto ch = read_integer<char>();

	return ch;
}

signed char ArchiverImpl::read_schar()
{
	const auto schar = read_integer<signed char>();

	return schar;
}

unsigned char ArchiverImpl::read_uchar()
{
	const auto uchar = read_integer<unsigned char>();

	return uchar;
}

short ArchiverImpl::read_sshort()
{
	const auto sshort = read_integer<short>();

	return sshort;
}

unsigned short ArchiverImpl::read_ushort()
{
	const auto ushort = read_integer<unsigned short>();

	return ushort;
}

long ArchiverImpl::read_slong()
{
	const auto slong = read_integer<long>();

	return slong;
}

unsigned long ArchiverImpl::read_ulong()
{
	const auto ulong = read_integer<unsigned long>();

	return ulong;
}

void ArchiverImpl::read_char_array(
	char* items,
	const int item_count)
{
	read_integer_array(items, item_count);
}

void ArchiverImpl::read_schar_array(
	signed char* items,
	const int item_count)
{
	read_integer_array(items, item_count);
}

void ArchiverImpl::read_uchar_array(
	unsigned char* items,
	const int item_count)
{
	read_integer_array(items, item_count);
}

void ArchiverImpl::read_sshort_array(
	signed short* items,
	const int item_count)
{
	read_integer_array(items, item_count);
}

void ArchiverImpl::read_ushort_array(
	unsigned short* items,
	const int item_count)
{
	read_integer_array(items, item_count);
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

	const auto archived_string_length = read_slong();

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
	const auto checksum = read_integer<unsigned long>(true);

	if (checksum != crc32_.get_value())
	{
		throw ArchiverExceptionImpl{"Checksum mismatch."};
	}
}

void ArchiverImpl::write_char(
	const char value_schar)
{
	write_integer(value_schar);
}

void ArchiverImpl::write_schar(
	const signed char value_schar)
{
	write_integer(value_schar);
}

void ArchiverImpl::write_uchar(
	const unsigned char value_uchar)
{
	write_integer(value_uchar);
}

void ArchiverImpl::write_sshort(
	const short value_sshort)
{
	write_integer(value_sshort);
}

void ArchiverImpl::write_ushort(
	const unsigned short value_ushort)
{
	write_integer(value_ushort);
}

void ArchiverImpl::write_slong(
	const long value_slong)
{
	write_integer(value_slong);
}

void ArchiverImpl::write_ulong(
	const unsigned long value_ulong)
{
	write_integer(value_ulong);
}

void ArchiverImpl::write_char_array(
	const char* const items,
	const int item_count)
{
	write_integer_array(items, item_count);
}

void ArchiverImpl::write_schar_array(
	const signed char* const items,
	const int item_count)
{
	write_integer_array(items, item_count);
}

void ArchiverImpl::write_uchar_array(
	const unsigned char* const items,
	const int item_count)
{
	write_integer_array(items, item_count);
}

void ArchiverImpl::write_sshort_array(
	const short* const items,
	const int item_count)
{
	write_integer_array(items, item_count);
}

void ArchiverImpl::write_ushort_array(
	const unsigned short* const items,
	const int item_count)
{
	write_integer_array(items, item_count);
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

	write_slong(string_length);

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

	write_integer<unsigned long>(checksum, true);
}


ArchiverExceptionImpl::ArchiverExceptionImpl(
	const char* const message)
	:
	message_{message ? message : std::string{}}
{
}

ArchiverExceptionImpl::ArchiverExceptionImpl(
	const std::string& message)
	:
	message_{message}
{
}

const char* ArchiverExceptionImpl::get_message() const
{
	return message_.c_str();
}


} // namespace


ArchiverUPtr ArchiverFactory::create()
{
	auto archiver = ArchiverImplUPtr{new ArchiverImpl{}};

	archiver->create();

	return archiver;
}


} // bstone
