/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_char_conv.h"
#include "bstone_exception.h"

namespace bstone {
namespace char_conv {

namespace {

class NibbleToHexCharException : public Exception
{
public:
	explicit NibbleToHexCharException(const char* message) noexcept
		:
		Exception{"BSTONE_NIBBLE_TO_HEX_CHAR", message}
	{}

	~NibbleToHexCharException() override = default;
};

class HexCharToByteException : public Exception
{
public:
	explicit HexCharToByteException(const char* message) noexcept
		:
		Exception{"BSTONE_HEX_CHAR_TO_BYTE", message}
	{}

	~HexCharToByteException() override = default;
};

class HexCharsToBytesException : public Exception
{
public:
	explicit HexCharsToBytesException(const char* message) noexcept
		:
		Exception{"BSTONE_HEX_CHARS_TO_BYTES", message}
	{}

	~HexCharsToBytesException() override = default;
};

class BytesToHexCharsException : public Exception
{
public:
	explicit BytesToHexCharsException(const char* message) noexcept
		:
		Exception{"BSTONE_BYTES_TO_HEX_CHARS", message}
	{}

	~BytesToHexCharsException() override = default;
};

class ToCharsException : public Exception
{
public:
	explicit ToCharsException(const char* message) noexcept
		:
		Exception{"BSTONE_TO_CHARS", message}
	{}

	~ToCharsException() override = default;
};

class FromCharsException : public Exception
{
public:
	explicit FromCharsException(const char* message) noexcept
		:
		Exception{"BSTONE_FROM_CHARS", message}
	{}

	~FromCharsException() override = default;
};

} // namespace

// ==========================================================================

namespace detail {

[[noreturn]] void fail_nibble_to_hex_char(const char* message)
{
	throw NibbleToHexCharException{message};
}

[[noreturn]] void fail_hex_char_to_byte(const char* message)
{
	throw HexCharToByteException{message};
}

[[noreturn]] void fail_hex_chars_to_bytes(const char* message)
{
	throw HexCharsToBytesException{message};
}

[[noreturn]] void fail_bytes_to_hex_chars(const char* message)
{
	throw BytesToHexCharsException{message};
}

[[noreturn]] void fail_to_chars(const char* message)
{
	throw ToCharsException{message};
}

[[noreturn]] void fail_from_chars(const char* message)
{
	throw FromCharsException{message};
}

} // namespace detail

} // namespace char_conv
} // namespace bstone
