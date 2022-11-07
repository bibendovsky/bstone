/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CHAR_HASHER_INCLUDED
#define BSTONE_CHAR_HASHER_INCLUDED

#include <cstddef>
#include <type_traits>
#include "bstone_span.h"

namespace bstone {

struct CharHasher
{
	template<typename TChar>
	constexpr std::size_t operator()(Span<TChar> chars) const
	{
		using Unsigned = std::conditional_t<
			sizeof(TChar) == 1,
			UInt8,
			std::conditional_t<
				sizeof(TChar) == 2,
				UInt16,
				std::conditional_t<
					sizeof(TChar) == 4,
					UInt32,
					void
				>
			>
		>;

		static_assert(!std::is_same<Unsigned, void>::value, "Unsupported char type.");

		auto hash = std::size_t{};

		for (const auto ch : chars)
		{
			hash += static_cast<Unsigned>(ch);
			hash += hash << 10;
			hash ^= hash >> 6;
		}

		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}
};

} // namespace bstone

#endif // !BSTONE_CHAR_HASHER_INCLUDED
