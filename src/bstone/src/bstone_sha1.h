/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SHA-1 implementation based on RFC 3174 sample code.
// https://www.ietf.org/rfc/rfc3174.txt

#ifndef BSTONE_SHA1_INCLUDED
#define BSTONE_SHA1_INCLUDED

#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include <cstdint>
#include <algorithm>
#include <array>
#include <bit>

namespace bstone {

constexpr int sha1_digest_size = 20;
constexpr int sha1_digest_char_count = sha1_digest_size * 2;

// =====================================

class Sha1Digest
{
public:
	constexpr Sha1Digest() = default;

	constexpr Sha1Digest(
		std::uint8_t v_0,
		std::uint8_t v_1,
		std::uint8_t v_2,
		std::uint8_t v_3,
		std::uint8_t v_4,
		std::uint8_t v_5,
		std::uint8_t v_6,
		std::uint8_t v_7,
		std::uint8_t v_8,
		std::uint8_t v_9,
		std::uint8_t v10,
		std::uint8_t v11,
		std::uint8_t v12,
		std::uint8_t v13,
		std::uint8_t v14,
		std::uint8_t v15,
		std::uint8_t v16,
		std::uint8_t v17,
		std::uint8_t v18,
		std::uint8_t v19) noexcept
		:
		digest_{v_0, v_1, v_2, v_3, v_4, v_5, v_6, v_7, v_8, v_9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19}
	{}

	constexpr int get_size() const noexcept
	{
		return sha1_digest_size;
	}

	constexpr const std::uint8_t* get_data() const noexcept
	{
		return digest_;
	}

	constexpr std::uint8_t* get_data() noexcept
	{
		return digest_;
	}

	constexpr const std::uint8_t& operator[](int index) const
	{
		BSTONE_ASSERT(index >= 0 && index < sha1_digest_size);
		return digest_[index];
	}

	constexpr std::uint8_t& operator[](int index)
	{
		BSTONE_ASSERT(index >= 0 && index < sha1_digest_size);
		return digest_[index];
	}

private:
	using Digest = std::uint8_t[sha1_digest_size];

	Digest digest_{};
};

inline constexpr bool operator==(const Sha1Digest& a, const Sha1Digest& b)
{
	return std::equal(a.get_data(), a.get_data() + a.get_size(), b.get_data());
}

inline constexpr bool operator!=(const Sha1Digest& a, const Sha1Digest& b)
{
	return !(a == b);
}

// =====================================

class Sha1
{
public:
	void process(const void* data, int size);
	void finish();
	const Sha1Digest& get_digest() const;

private:
	using Block = std::array<std::uint8_t, 64>;
	using Digest32 = std::array<std::uint32_t, sha1_digest_size / 4>;

	Block block_{}; // 512-bit message block.
	Sha1Digest digest_{}; // Message digest.
	Digest32 digest32_{make_initial_digest_32()}; // Message digest as words.
	std::uint64_t length_{}; // Message length in bits.
	int block_index_{}; // Index into message block array.
	bool is_finished_{}; // Is the digest computed?

	static Digest32 make_initial_digest_32();

	template<int TBitCount>
	requires (TBitCount >= 0 && TBitCount <= 32)
	static std::uint32_t circular_shift(std::uint32_t word) noexcept
	{
		return std::rotl(word, TBitCount);
	}

	void pad_message();
	void process_block();
};

// =====================================

constexpr Sha1Digest make_sha1_digest(const char* chars, int char_count)
{
	if (char_count != sha1_digest_char_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid SHA1 string length.");
	}
	Sha1Digest result{};
	hex_chars_to_bytes(chars, chars + char_count, result.get_data(), result.get_data() + result.get_size());
	return result;
}

constexpr Sha1Digest make_sha1_digest(const char* chars)
{
	return make_sha1_digest(chars, static_cast<int>(char_traits::get_size(chars)));
}

} // namespace bstone

#endif // BSTONE_SHA1_INCLUDED
