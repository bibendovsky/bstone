/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_rlew_decoder.h"

#include <cassert>

#include <algorithm>

#include "bstone_exception.h"

namespace bstone {

std::intptr_t RlewDecoder::decode(
	std::uint16_t tag,
	const std::uint8_t* src_bytes,
	std::intptr_t src_count,
	std::uint16_t* dst_words,
	std::intptr_t dst_count)
try
{
	assert(src_count >= 0);
	assert(dst_count >= 0);

	if ((src_count % 2) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Odd source byte count.");
	}

	auto src_index = std::intptr_t{};
	auto dst_index = std::intptr_t{};

	const auto read_word = [src_bytes, src_count, &src_index]() -> std::uint16_t
	{
		if (src_count - src_index < 2)
		{
			BSTONE_THROW_STATIC_SOURCE("Source underflow.");
		}

		const auto word_lo = src_bytes[src_index++];
		const auto word_hi = src_bytes[src_index++];
		const auto word = static_cast<std::uint16_t>(word_lo | word_hi << 8);
		return word;
	};

	const auto write_word = [dst_words, dst_count, &dst_index](std::uint16_t word)
	{
		if (dst_count - dst_index < 1)
		{
			BSTONE_THROW_STATIC_SOURCE("Destination overflow.");
		}

		dst_words[dst_index++] = word;
	};

	const auto write_words = [dst_words, dst_count, &dst_index](
		std::uint16_t value,
		std::intptr_t count)
	{
		if (dst_count - dst_index < count)
		{
			BSTONE_THROW_STATIC_SOURCE("Destination overflow.");
		}

		std::fill_n(dst_words + dst_index, count, value);
		dst_index += count;
	};

	const auto decoded_size_in_bytes = read_word();

	if ((decoded_size_in_bytes % 2) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Odd decoded word count.");
	}

	while (src_index < src_count)
	{
		const auto word = read_word();

		if (word != tag)
		{
			write_word(word);
		}
		else
		{
			const auto count = read_word();
			const auto value = read_word();
			write_words(value, count);
		}
	}

	if (dst_index != decoded_size_in_bytes / 2)
	{
		BSTONE_THROW_STATIC_SOURCE("Decoded word count mismatch.");
	}

	return dst_index;
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
