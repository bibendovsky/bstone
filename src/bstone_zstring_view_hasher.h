/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
	constexpr std::size_t operator()(ZStringViewT<TChar> zstring_view) const
	{
		return CharHasher{}(make_span(zstring_view.get_data(), zstring_view.get_size()));
	}
};

} // namespace bstone

#endif // BSTONE_ZSTRING_VIEW_HASHER_INCLUDED
