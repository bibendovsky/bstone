/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_VIEW_HASHER_INCLUDED
#define BSTONE_STRING_VIEW_HASHER_INCLUDED

#include <string_view>
#include "bstone_char_hasher.h"

namespace bstone {

struct StringViewHasher
{
	template<typename TChar>
	constexpr std::size_t operator()(std::basic_string_view<TChar> string_view) const
	{
		return CharHasher{}(string_view.cbegin(), string_view.cend());
	}
};

} // namespace bstone

#endif // !BSTONE_STRING_VIEW_HASHER_INCLUDED
