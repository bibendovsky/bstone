/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Byte order (endianness) manipulation.
//


#ifndef BSTONE_ENDIAN_INCLUDED
#define BSTONE_ENDIAN_INCLUDED


#include <cstdint>

#include "SDL_endian.h"


namespace bstone
{


enum class EndianId
{
	none,
	big,
	little,

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	native = big,
#else // SDL_BYTEORDER == SDL_BIG_ENDIAN
	native = little,
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN
}; // EndianId


namespace detail
{


//
// Notes:
//    - We are assuming CPU architecture without exotic types (i.e. 9-bit char, etc).
//
template<typename T>
struct ShouldBeSwapped
{
	static constexpr auto value_ = (sizeof(T) > 1);
}; // ShouldBeSwapped


class EndianSwap final
{
public:
	static std::uint8_t swap(
		const std::uint8_t value)
	{
		return value;
	}

	static std::int8_t swap(
		const std::int8_t value)
	{
		return value;
	}

	static std::uint16_t swap(
		const std::uint16_t value)
	{
		return (value >> 8) | (value << 8);
	}

	static std::int16_t swap(
		const std::int16_t value)
	{
		return static_cast<std::int16_t>(swap(static_cast<std::uint16_t>(value)));
	}

	static std::uint32_t swap(
		const std::uint32_t value)
	{
		constexpr auto ooxxooxx = std::uint32_t{0x00FF00FF};
		constexpr auto xxooxxoo = std::uint32_t{0xFF00FF00};

		const auto swap16 = (value << 16) | (value >> 16);
		return ((swap16 << 8) & xxooxxoo) | ((swap16 >> 8) & ooxxooxx);
	}

	static std::int32_t swap(
		const std::int32_t value)
	{
		return static_cast<std::int32_t>(swap(static_cast<std::uint32_t>(value)));
	}

	static std::uint64_t swap(
		const std::uint64_t value)
	{
		constexpr auto ooxxooxx = std::uint64_t{0x0000FFFF0000FFFF};
		constexpr auto xxooxxoo = std::uint64_t{0xFFFF0000FFFF0000};

		constexpr auto oxoxoxox = std::uint64_t{0x00FF00FF00FF00FF};
		constexpr auto xoxoxoxo = std::uint64_t{0xFF00FF00FF00FF00};

		const auto swap32 = (value << 32) | (value >> 32);
		const auto swap16 = ((swap32 & ooxxooxx) << 16) | ((swap32 & xxooxxoo) >> 16);
		return ((swap16 & oxoxoxox) << 8) | ((swap16 & xoxoxoxo) >> 8);
	}

	static std::int64_t swap(
		const std::int64_t value)
	{
		return static_cast<std::int64_t>(swap(static_cast<std::uint64_t>(value)));
	}
}; // EndianSwap


template<EndianId TId>
struct Endian final
{
	// Returns swaped bytes on little-endian platform or as-is otherwise.
	template<typename T>
	static T big(
		const T value) = delete;

	// Swaps the bytes inplace on little-endian platform.
	template<typename T>
	static void big_i(
		T& value) = delete;

	// Returns swaped bytes on big-endian platform or as-is otherwise.
	template<typename T>
	static T little(
		const T value) = delete;

	// Swaps the bytes inplace on big-endian platform.
	template<typename T>
	static void little_i(
		T& value) = delete;

	static bool is_big() = delete;

	static bool is_little() = delete;

	static bool should_be_swapped() = delete;
}; // Endian


template<>
struct Endian<EndianId::big> final
{
	template<typename T>
	static T big(
		const T value)
	{
		return value;
	}

	template<typename T>
	static void big_i(
		T&)
	{
	}

	template<typename T>
	static T little(
		const T value)
	{
		return detail::EndianSwap::swap(value);
	}

	template<typename T>
	static void little_i(
		T& value)
	{
		value = detail::EndianSwap::swap(value);
	}

	static constexpr bool is_big()
	{
		return true;
	}

	static constexpr bool is_little()
	{
		return false;
	}

	template<typename T>
	static constexpr bool should_be_swapped()
	{
		return ShouldBeSwapped<T>::value_;
	}
}; // Endian


template<>
struct Endian<EndianId::little> final
{
	template<typename T>
	static T big(
		const T value)
	{
		return detail::EndianSwap::swap(value);
	}

	template<typename T>
	static void big_i(
		T& value)
	{
		value = detail::EndianSwap::swap(value);
	}

	template<typename T>
	static T little(
		const T value)
	{
		return value;
	}

	template<typename T>
	static void little_i(
		T&)
	{
	}

	static constexpr bool is_big()
	{
		return false;
	}

	static constexpr bool is_little()
	{
		return true;
	}

	template<typename T>
	static constexpr bool should_be_swapped()
	{
		return ShouldBeSwapped<T>::value_;
	}
}; // Endian


} // detail


using Endian = detail::Endian<EndianId::native>;


} // bstone


#endif // !BSTONE_ENDIAN_INCLUDED
