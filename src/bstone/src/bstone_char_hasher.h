/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CHAR_HASHER_INCLUDED
#define BSTONE_CHAR_HASHER_INCLUDED

#include <cstddef>

namespace bstone {

struct CharHasher
{
	template<typename TIter>
	constexpr std::size_t operator()(TIter chars_begin, TIter chars_end) const
	{
		static_assert(sizeof(decltype(*TIter{})) <= sizeof(std::size_t), "Unsupported type.");

		auto hash = std::size_t{5381};

		while (!(chars_begin == chars_end))
		{
			hash *= 33;
			hash += static_cast<std::size_t>(*chars_begin);
			++chars_begin;
		}

		return hash;
	}
};

} // namespace bstone

#endif // BSTONE_CHAR_HASHER_INCLUDED
