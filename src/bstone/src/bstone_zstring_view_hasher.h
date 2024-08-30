/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ZSTRING_VIEW_HASHER_INCLUDED
#define BSTONE_ZSTRING_VIEW_HASHER_INCLUDED

#include "bstone_char_hasher.h"
#include "bstone_zstring_view.h"

namespace bstone {

struct ZStringViewHasher
{
	template<typename TChar>
	constexpr std::size_t operator()(BasicZStringView<TChar> zstring_view) const
	{
		return CharHasher{}(zstring_view.cbegin(), zstring_view.cend());
	}
};

} // namespace bstone

#endif // BSTONE_ZSTRING_VIEW_HASHER_INCLUDED
