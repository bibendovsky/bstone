/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// CRC-32/ISO-HDLC

#ifndef BSTONE_CRC32_INCLUDED
#define BSTONE_CRC32_INCLUDED

#include <cstdint>

namespace bstone {

class Crc32
{
public:
	std::uint32_t get_value() const noexcept;

	void reset() noexcept;
	void update(const void* data, std::intptr_t size);

private:
	std::uint32_t value_{};
};

} // namespace bstone

#endif // BSTONE_CRC32_INCLUDED
