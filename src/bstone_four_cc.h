/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// FourCC stored as integer in little-endian order (i.e. '0123' => 0x33323130).

#ifndef BSTONE_FOUR_CC_INCLUDED
#define BSTONE_FOUR_CC_INCLUDED

#include <cstdint>

#include "bstone_assert.h"

namespace bstone {

class FourCc
{
public:
	constexpr FourCc() noexcept = default;

	constexpr explicit FourCc(std::uint32_t value) noexcept
		:
		value_{value}
	{}

	template<typename TChar>
	constexpr FourCc(TChar ch_0, TChar ch_1, TChar ch_2, TChar ch_3) noexcept
		:
		value_{
			static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch_0)) |
			(static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch_1)) << 8) |
			(static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch_2)) << 16) |
			(static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch_3)) << 24)}
	{}

	constexpr std::uint32_t get_value() const noexcept
	{
		return value_;
	}

	template<typename TChar = char>
	constexpr char operator[](std::intptr_t index) const noexcept
	{
		BSTONE_ASSERT(index >= 0 && index < 4);

		return static_cast<TChar>(static_cast<std::uint8_t>(value_ >> (8 * index)));
	}

private:
	std::uint32_t value_{};
};

// ==========================================================================

inline constexpr bool operator==(const FourCc& lhs, const FourCc& rhs) noexcept
{
	return lhs.get_value() == rhs.get_value();
}

inline constexpr bool operator!=(const FourCc& lhs, const FourCc& rhs) noexcept
{
	return !(lhs == rhs);
}

} // namespace bstone

#endif // BSTONE_FOUR_CC_INCLUDED
