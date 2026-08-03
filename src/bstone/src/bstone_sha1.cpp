/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SHA-1 implementation based on RFC 3174 sample code.
// https://www.ietf.org/rfc/rfc3174.txt

#include "bstone_sha1.h"

#include "bstone_endian.h"
#include <algorithm>
#include "bstone_exception.h"

namespace bstone {

void Sha1::process(const void* data, int size)
{
	if (is_finished_)
	{
		BSTONE_THROW_STATIC_SOURCE("Finished.");
	}
	const int bit_count = size * 8;
	if (0xFFFFFFFFFFFFFFFFUL - length_ < static_cast<unsigned int>(bit_count))
	{
		BSTONE_THROW_STATIC_SOURCE("Message too long.");
	}
	const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(data);
	for (int i = 0; i < size; ++i)
	{
		block_[block_index_] = bytes[i];
		block_index_ += 1;
		if (block_index_ == 64)
		{
			process_block();
		}
	}
	length_ += bit_count;
}

void Sha1::finish()
{
	if (is_finished_)
	{
		return;
	}
	is_finished_ = true;
	pad_message();
	block_.fill(0);
	length_ = 0;
	for (int i = 0; i < sha1_digest_size; ++i)
	{
		digest_[i] = static_cast<std::uint8_t>(digest32_[i / 4] >> (8 * (3 - (i % 4))));
	}
}

const Sha1Digest& Sha1::get_digest() const
{
	return digest_;
}

Sha1::Digest32 Sha1::make_initial_digest_32()
{
	return Digest32{0x67452301U, 0xEFCDAB89U, 0x98BADCFEU, 0x10325476U, 0xC3D2E1F0U};
}

void Sha1::pad_message()
{
	// Check to see if the current message block is too small to hold
	// the initial padding bits and length. If so, we will pad the
	// block, process it, and then continue padding into a second block.
	if (block_index_ > 55)
	{
		block_[block_index_] = 0x80;
		block_index_ += 1;
		std::fill_n(block_.data() + block_index_, 64 - block_index_, std::uint8_t{});
		block_index_ = 64;
		process_block();
	}
	else
	{
		block_[block_index_] = 0x80;
		block_index_ += 1;
	}
	std::fill_n(block_.data() + block_index_, 56 - block_index_, std::uint8_t{});
	block_index_ = 56;
	// Store the message length as the last 8 octets.
	block_[56] = static_cast<std::uint8_t>(length_ >> 56);
	block_[57] = static_cast<std::uint8_t>(length_ >> 48);
	block_[58] = static_cast<std::uint8_t>(length_ >> 40);
	block_[59] = static_cast<std::uint8_t>(length_ >> 32);
	block_[60] = static_cast<std::uint8_t>(length_ >> 24);
	block_[61] = static_cast<std::uint8_t>(length_ >> 16);
	block_[62] = static_cast<std::uint8_t>(length_ >> 8);
	block_[63] = static_cast<std::uint8_t>(length_);
	process_block();
}

void Sha1::process_block()
{
	// Constants defined in SHA-1.
	constexpr std::uint32_t k[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
	// Word buffers.
	std::uint32_t a{};
	std::uint32_t b{};
	std::uint32_t c{};
	std::uint32_t d{};
	std::uint32_t e{};
	// Word sequence.
	std::array<std::uint32_t, 80> w{};
	// Initialize the first 16 words in the array W.
	for (int i = 0; i < 16; ++i)
	{
		w[i] = endian::read_u32_be(&block_[i * 4]);
	}
	for (int i = 16; i < 80; ++i)
	{
		w[i] = circular_shift<1>(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);
	}
	a = digest32_[0];
	b = digest32_[1];
	c = digest32_[2];
	d = digest32_[3];
	e = digest32_[4];
	for (int i = 0; i < 20; ++i)
	{
		const std::uint32_t temp = circular_shift<5>(a) + ((b & c) | (~b & d)) + e + w[i] + k[0];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}
	for (int i = 20; i < 40; ++i)
	{
		const std::uint32_t temp = circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[1];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}
	for (int i = 40; i < 60; ++i)
	{
		const std::uint32_t temp = circular_shift<5>(a) + ((b & c) | (b & d) | (c & d)) + e + w[i] + k[2];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}
	for (int i = 60; i < 80; ++i)
	{
		const std::uint32_t temp = circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[3];
		e = d;
		d = c;
		c = circular_shift<30>(b);
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

} // namespace bstone
