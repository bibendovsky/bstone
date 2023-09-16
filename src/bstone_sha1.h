/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SHA-1 implementation based on RFC 3174 sample code.
// https://www.ietf.org/rfc/rfc3174.txt

#if !defined(BSTONE_SHA1_INCLUDED)
#define BSTONE_SHA1_INCLUDED

#include <cassert>

#include <type_traits>

#include "bstone_array.h"
#include "bstone_int.h"
#include "bstone_span.h"

namespace bstone {

constexpr auto sha1_digest_size = IntP{20};
constexpr auto sha1_digest_char_count = sha1_digest_size * 2;

// ==========================================================================

using Sha1Digest = Array<UInt8, sha1_digest_size>;

// ==========================================================================

class Sha1
{
public:
	void process(const UInt8* bytes, IntP count);
	void process(Span<const UInt8> items_span);

	void finish();

	const Sha1Digest& get_digest() const noexcept;

private:
	using Block = Array<UInt8, 64>;
	using Digest32 = Array<UInt32, sha1_digest_size / 4>;

private:
	Block block_{}; // 512-bit message block.
	Sha1Digest digest_{}; // Message digest.
	Digest32 digest32_{make_initial_digest_32()}; // Message digest as words.
	UInt64 length_{}; // Message length in bits.
	IntP block_index_{}; // Index into message block array.
	bool is_finished_{}; // Is the digest computed?

private:
	static Digest32 make_initial_digest_32() noexcept;

	template<int TBitCount>
	static UInt32 circular_shift(UInt32 word) noexcept
	{
		static_assert(TBitCount >= 0 && TBitCount <= 32, "Invalid bit count.");
		return word << TBitCount | word >> (32 - TBitCount);
	}

	void pad_message();
	void process_block();
};

} // namespace bstone

#endif // BSTONE_SHA1_INCLUDED
