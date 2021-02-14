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


//
// SHA-1 implementation based on RFC 3174 sample code
// http://www.ietf.org/rfc/rfc3174.txt
//


#include "bstone_sha1.h"
#include <cassert>


namespace bstone
{


namespace
{


template<int TBits>
constexpr std::uint32_t sha1_circular_shift(
	const std::uint32_t word)
{
	return (word << TBits) | (word >> (32 - TBits));
}


} // namespace


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1
//

const Sha1::Digest32 Sha1::initial_digest_32 =
{
	0x67452301,
	0xEFCDAB89,
	0x98BADCFE,
	0x10325476,
	0xC3D2E1F0,
}; // initial_digest_32


Sha1::Sha1()
	:
	block_{},
	digest_{},
	digest32_{initial_digest_32},
	length_low_{},
	length_high_{},
	block_index_{},
	is_finished_{},
	is_invalid_{}
{
}

Sha1::Sha1(
	const char (&sha1_string)[(hash_size * 2) + 1])
{
	ctor(sha1_string, hash_size * 2);
}

Sha1::Sha1(
	const std::string& sha1_string)
{
	ctor(sha1_string.c_str(), static_cast<int>(sha1_string.size()));
}

Sha1::Sha1(
	const Digest& digest)
	:
	block_{},
	digest_{digest},
	digest32_{},
	length_low_{},
	length_high_{},
	block_index_{},
	is_finished_{true},
	is_invalid_{}
{
}

Sha1::Sha1(
	const Sha1& rhs)
	:
	block_{rhs.block_},
	digest_{rhs.digest_},
	digest32_{rhs.digest32_},
	length_low_{rhs.length_low_},
	length_high_{rhs.length_high_},
	block_index_{rhs.block_index_},
	is_finished_{rhs.is_finished_},
	is_invalid_{rhs.is_invalid_}
{
}

Sha1::Sha1(
	Sha1&& rhs)
	:
	block_(std::move(rhs.block_)),
	digest_(std::move(rhs.digest_)),
	digest32_(std::move(rhs.digest32_)),
	length_low_(std::move(rhs.length_low_)),
	length_high_(std::move(rhs.length_high_)),
	block_index_(std::move(rhs.block_index_)),
	is_finished_(std::move(rhs.is_finished_)),
	is_invalid_(std::move(rhs.is_invalid_))
{
}

Sha1& Sha1::operator=(
	const Sha1& rhs)
{
	if (std::addressof(rhs) != this)
	{
		block_ = rhs.block_;
		digest_ = rhs.digest_;
		digest32_ = rhs.digest32_;
		length_low_ = rhs.length_low_;
		length_high_ = rhs.length_high_;
		block_index_ = rhs.block_index_;
		is_finished_ = rhs.is_finished_;
		is_invalid_ = rhs.is_invalid_;
	}

	return *this;
}

void Sha1::reset()
{
	block_.fill({});
	digest_.fill({});
	digest32_ = initial_digest_32;

	length_low_ = {};
	length_high_ = {};
	block_index_ = {};

	is_finished_ = {};
	is_invalid_ = {};
}

void Sha1::process(
	const void* message,
	const int message_size)
{
	if (!message || message_size < 0)
	{
		assert(!"Invalid parameters.");
		return;
	}

	if (is_finished_ || is_invalid_)
	{
		assert(!"Invalid state.");
		return;
	}

	auto octets = static_cast<const std::uint8_t*>(message);

	for (int i = 0; i < message_size; ++i)
	{
		block_[block_index_] = octets[i];
		block_index_ += 1;

		length_low_ += 8;

		if (length_low_ == 0)
		{
			length_high_ += 1;

			if (length_high_ == 0)
			{
				// Message is too long.
				is_invalid_ = true;

				return;
			}
		}

		if (block_index_ == 64)
		{
			process_block();
		}
	}
}

void Sha1::finish()
{
	if (is_finished_ || is_invalid_)
	{
		return;
	}

	is_finished_ = true;

	pad_message();

	block_.fill(0);

	length_low_ = 0;
	length_high_ = 0;

	for (int i = 0; i < hash_size; ++i)
	{
		digest_[i] = static_cast<std::uint8_t>(digest32_[i >> 2] >> 8 * (3 - (i & 0x3)));
	}
}

bool Sha1::is_finished() const
{
	return is_finished_;
}

bool Sha1::is_valid() const
{
	return !is_invalid_;
}

const Sha1::Digest& Sha1::get_digest() const
{
	return digest_;
}

std::string Sha1::to_string() const
{
	if (!is_finished_ || is_invalid_)
	{
		return {};
	}

	const auto& digest = get_digest();

	std::string digest_string;
	digest_string.reserve(hash_size * 2);

	for (auto digest_octet : digest)
	{
		const char nibbles[2] =
		{
			static_cast<char>((digest_octet / 16) & 0xF),
			static_cast<char>(digest_octet & 0xF),
		}; // nibbles

		for (int i = 0; i < 2; ++i)
		{
			const auto nibble = nibbles[i];

			char nibble_char;

			if (nibble <= 9)
			{
				nibble_char = '0' + nibble;
			}
			else
			{
				nibble_char = 'a' + (nibble - 10);
			}

			digest_string += nibble_char;
		}
	}

	return digest_string;
}

void Sha1::ctor(
	const char* const sha1_string,
	const int sha1_string_length)
{
	is_finished_ = false;
	is_invalid_ = true;

	if (!sha1_string || sha1_string_length != (hash_size * 2))
	{
		return;
	}

	for (int i = 0; i < hash_size; ++i)
	{
		int nibbles[2];

		for (int j = 0; j < 2; ++j)
		{
			auto nibble_char = sha1_string[(i * 2) + j];

			switch (nibble_char)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				nibbles[j] = nibble_char - '0';
				break;

			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				nibbles[j] = 10 + (nibble_char - 'a');
				break;

			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				nibbles[j] = 10 + (nibble_char - 'A');
				break;

			default:
				return;
			}
		}

		const auto digest_octet = static_cast<std::uint8_t>((nibbles[0] * 16) + nibbles[1]);

		digest_[i] = digest_octet;
	}

	is_finished_ = true;
	is_invalid_ = false;
}

void Sha1::pad_message()
{
	// Check to see if the current message block is too small to hold
	// the initial padding bits and length.  If so, we will pad the
	// block, process it, and then continue padding into a second
	// block.
	//

	if (block_index_ > 55)
	{
		block_[block_index_] = 0x80;
		block_index_ += 1;

		while (block_index_ < 64)
		{
			block_[block_index_] = 0;
			block_index_ += 1;
		}

		process_block();

		while (block_index_ < 56)
		{
			block_[block_index_] = 0;
			block_index_ += 1;
		}
	}
	else
	{
		block_[block_index_] = 0x80;
		block_index_ += 1;

		while (block_index_ < 56)
		{
			block_[block_index_] = 0;
			block_index_ += 1;
		}
	}

	// Store the message length as the last 8 octets
	//
	block_[56] = static_cast<std::uint8_t>(length_high_ >> 24);
	block_[57] = static_cast<std::uint8_t>(length_high_ >> 16);
	block_[58] = static_cast<std::uint8_t>(length_high_ >> 8);
	block_[59] = static_cast<std::uint8_t>(length_high_);
	block_[60] = static_cast<std::uint8_t>(length_low_ >> 24);
	block_[61] = static_cast<std::uint8_t>(length_low_ >> 16);
	block_[62] = static_cast<std::uint8_t>(length_low_ >> 8);
	block_[63] = static_cast<std::uint8_t>(length_low_);

	process_block();
}

void Sha1::process_block()
{
	// Constants defined in SHA-1
	static const std::uint32_t k[] =
	{
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	}; // k

	// Word buffers
	std::uint32_t a;
	std::uint32_t b;
	std::uint32_t c;
	std::uint32_t d;
	std::uint32_t e;

	// Word sequence
	std::array<std::uint32_t, 80> w;

	// Initialize the first 16 words in the array W
	//
	for (int i = 0; i < 16; ++i)
	{
		w[i] = block_[i * 4] << 24;
		w[i] |= block_[i * 4 + 1] << 16;
		w[i] |= block_[i * 4 + 2] << 8;
		w[i] |= block_[i * 4 + 3];
	}

	for (int i = 16; i < 80; ++i)
	{
		w[i] = sha1_circular_shift<1>(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);
	}

	a = digest32_[0];
	b = digest32_[1];
	c = digest32_[2];
	d = digest32_[3];
	e = digest32_[4];

	for (int i = 0; i < 20; ++i)
	{
		const auto temp = sha1_circular_shift<5>(a) + ((b & c) | ((~b) & d)) + e + w[i] + k[0];

		e = d;
		d = c;
		c = sha1_circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (int i = 20; i < 40; ++i)
	{
		const auto temp = sha1_circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[1];

		e = d;
		d = c;
		c = sha1_circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (int i = 40; i < 60; ++i)
	{
		const auto temp = sha1_circular_shift<5>(a) + ((b & c) | (b & d) | (c & d)) + e + w[i] + k[2];

		e = d;
		d = c;
		c = sha1_circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (int i = 60; i < 80; ++i)
	{
		const auto temp = sha1_circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[3];

		e = d;
		d = c;
		c = sha1_circular_shift<30>(b);
		b = a;
		a = temp;
	}

	digest32_[0] += a;
	digest32_[1] += b;
	digest32_[2] += c;
	digest32_[3] += d;
	digest32_[4] += e;

	block_index_ = 0;
}

//
// Sha1
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 operators

bool operator==(
	const bstone::Sha1& lhs,
	const bstone::Sha1& rhs)
{
	return
		lhs.is_valid() && rhs.is_valid() &&
		lhs.is_finished() && rhs.is_finished() &&
		(lhs.get_digest() == rhs.get_digest());
}

bool operator!=(
	const bstone::Sha1& lhs,
	const bstone::Sha1& rhs)
{
	return !(lhs == rhs);
}

bool operator==(
	const bstone::Sha1& lhs,
	const std::string& rhs)
{
	return lhs == bstone::Sha1{rhs};
}

bool operator!=(
	const bstone::Sha1& lhs,
	const std::string& rhs)
{
	return !(lhs == rhs);
}

// Sha1 operators
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
