/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SHA-1 implementation based on RFC 3174 sample code.
// https://www.ietf.org/rfc/rfc3174.txt

#include "bstone_exception.h"
#include "bstone_sha1.h"

namespace bstone {

void Sha1::process(const void* data, std::intptr_t size)
{
	if (is_finished_)
	{
		BSTONE_THROW_STATIC_SOURCE("Finished.");
	}

	const auto bit_count = size * 8;

	if (0xFFFFFFFFFFFFFFFFUL - length_ < static_cast<std::uint64_t>(bit_count))
	{
		BSTONE_THROW_STATIC_SOURCE("Message too long.");
	}

	const auto bytes = static_cast<const std::uint8_t*>(data);

	for (auto i = decltype(size){}; i < size; ++i)
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

	for (auto i = decltype(sha1_digest_size){}; i < sha1_digest_size; ++i)
	{
		digest_[i] = static_cast<std::uint8_t>(digest32_[i / 4] >> (8 * (3 - (i % 4))));
	}
}

const Sha1Digest& Sha1::get_digest() const noexcept
{
	return digest_;
}

Sha1::Digest32 Sha1::make_initial_digest_32() noexcept
{
	return Digest32
	{
		0x67452301U,
		0xEFCDAB89U,
		0x98BADCFEU,
		0x10325476U,
		0xC3D2E1F0U
	};
}

void Sha1::pad_message()
{
	// Check to see if the current message block is too small to hold
	// the initial padding bits and length. If so, we will pad the
	// block, process it, and then continue padding into a second block.
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

	// Store the message length as the last 8 octets.
	//
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
	constexpr std::uint32_t k[] =
	{
		0x5A827999,
		0x6ED9EBA1,
		0x8F1BBCDC,
		0xCA62C1D6
	};

	// Word buffers.

	std::uint32_t a{};
	std::uint32_t b{};
	std::uint32_t c{};
	std::uint32_t d{};
	std::uint32_t e{};

	// Word sequence.
	Array<std::uint32_t, 80> w{};

	// Initialize the first 16 words in the array W.
	//
	for (auto i = std::intptr_t{}; i < 16; ++i)
	{
		w[i] =
			(static_cast<std::uint32_t>(block_[i * 4 + 0]) << 24) |
			(static_cast<std::uint32_t>(block_[i * 4 + 1]) << 16) |
			(static_cast<std::uint32_t>(block_[i * 4 + 2]) << 8) |
			block_[(i * 4) + 3];
	}

	for (auto i = std::intptr_t{16}; i < 80; ++i)
	{
		w[i] = circular_shift<1>(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);
	}

	a = digest32_[0];
	b = digest32_[1];
	c = digest32_[2];
	d = digest32_[3];
	e = digest32_[4];

	for (auto i = std::intptr_t{}; i < 20; ++i)
	{
		const auto temp = circular_shift<5>(a) + ((b & c) | (~b & d)) + e + w[i] + k[0];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (auto i = std::intptr_t{20}; i < 40; ++i)
	{
		const auto temp = circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[1];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (auto i = std::intptr_t{40}; i < 60; ++i)
	{
		const auto temp = circular_shift<5>(a) + ((b & c) | (b & d) | (c & d)) + e + w[i] + k[2];
		e = d;
		d = c;
		c = circular_shift<30>(b);
		b = a;
		a = temp;
	}

	for (auto i = std::intptr_t{60}; i < 80; ++i)
	{
		const auto temp = circular_shift<5>(a) + (b ^ c ^ d) + e + w[i] + k[3];
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
