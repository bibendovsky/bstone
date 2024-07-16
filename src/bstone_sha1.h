/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SHA-1 implementation based on RFC 3174 sample code.
// https://www.ietf.org/rfc/rfc3174.txt

#ifndef BSTONE_SHA1_INCLUDED
#define BSTONE_SHA1_INCLUDED

#include <cstdint>

#include "bstone_array.h"
#include "bstone_char_conv.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_span.h"

namespace bstone {

constexpr auto sha1_digest_size = 20;
constexpr auto sha1_digest_char_count = sha1_digest_size * 2;

// ==========================================================================

using Sha1Digest = Array<std::uint8_t, sha1_digest_size>;

// ==========================================================================

class Sha1
{
public:
	void process(const void* data, std::intptr_t size);

	template<typename T>
	void process(Span<T> span);

	void finish();

	const Sha1Digest& get_digest() const noexcept;

private:
	using Block = Array<std::uint8_t, 64>;
	using Digest32 = Array<std::uint32_t, sha1_digest_size / 4>;

private:
	Block block_{}; // 512-bit message block.
	Sha1Digest digest_{}; // Message digest.
	Digest32 digest32_{make_initial_digest_32()}; // Message digest as words.
	std::uint64_t length_{}; // Message length in bits.
	std::intptr_t block_index_{}; // Index into message block array.
	bool is_finished_{}; // Is the digest computed?

private:
	static Digest32 make_initial_digest_32() noexcept;

	template<int TBitCount>
	static std::uint32_t circular_shift(std::uint32_t word) noexcept
	{
		static_assert(TBitCount >= 0 && TBitCount <= 32, "Invalid bit count.");
		return word << TBitCount | word >> (32 - TBitCount);
	}

	void pad_message();
	void process_block();
};

// --------------------------------------------------------------------------

template<typename T>
void Sha1::process(Span<T> span)
{
	process(span.get_data(), span.get_bytes_size());
}

// ==========================================================================

template<typename TChar>
constexpr Sha1Digest make_sha1_digest(const TChar* chars, std::intptr_t char_count)
{
	if (char_count != sha1_digest_char_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid SHA1 string length.");
	}

	auto result = Sha1Digest{};
	hex_chars_to_bytes(chars, chars + char_count, result.begin(), result.end());
	return result;
}

template<typename TChar>
constexpr Sha1Digest make_sha1_digest(const TChar* chars)
{
	return make_sha1_digest(chars, char_traits::get_size(chars));
}

} // namespace bstone

#endif // BSTONE_SHA1_INCLUDED
