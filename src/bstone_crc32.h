/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// CRC-32/ISO-HDLC

#if !defined(BSTONE_CRC32_INCLUDED)
#define BSTONE_CRC32_INCLUDED

#include "bstone_int.h"

namespace bstone {

class Crc32
{
public:
	UInt32 get_value() const noexcept;

	void reset() noexcept;
	void update(const void* data, IntP size);

private:
	UInt32 value_{};
};

} // namespace bstone

#endif // BSTONE_CRC32_INCLUDED
