/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_RLEW_DECODER_INCLUDED
#define BSTONE_RLEW_DECODER_INCLUDED

#include <cstdint>

namespace bstone {

class RlewDecoder
{
public:
	static std::intptr_t decode(
		std::uint16_t tag,
		const std::uint8_t* src_bytes,
		std::intptr_t src_count,
		std::uint16_t* dst_words,
		std::intptr_t dst_count);
};

} // namespace bstone

#endif // BSTONE_RLEW_DECODER_INCLUDED
