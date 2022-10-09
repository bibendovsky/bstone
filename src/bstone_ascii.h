/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ASCII_INCLUDED
#define BSTONE_ASCII_INCLUDED

#include <utility>

namespace bstone {
namespace ascii {

template<typename TChar>
inline constexpr bool is_binary(TChar ch) noexcept
{
	return ch == '0' || ch == '1';
}

template<typename TChar>
inline constexpr bool is_octal(TChar ch) noexcept
{
	return ch >= '0' && ch <= '7';
}

template<typename TChar>
inline constexpr bool is_decimal(TChar ch) noexcept
{
	return ch >= '0' && ch <= '9';
}

template<typename TChar>
inline constexpr bool is_hex(TChar ch) noexcept
{
	return is_decimal(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

template<typename TChar>
inline constexpr bool is_lower(TChar ch) noexcept
{
	return ch >= 'a' && ch <= 'z';
}

template<typename TChar>
inline constexpr bool is_upper(TChar ch) noexcept
{
	return ch >= 'A' && ch <= 'Z';
}

// ==========================================================================

template<typename TChar>
inline constexpr TChar to_lower(TChar ch) noexcept
{
	return is_upper(ch) ? static_cast<TChar>('a' + ch - 'A') : ch;
}

template<typename TChar>
inline constexpr TChar to_upper(TChar ch) noexcept
{
	return is_lower(ch) ? static_cast<TChar>('A' + ch - 'a') : ch;
}

// ==========================================================================

template<typename TChars>
inline void to_lower_range(TChars&& chars) noexcept
{
	for (auto& ch : std::forward<TChars>(chars))
	{
		ch = to_lower(ch);
	}
}

template<typename TChars>
inline void to_upper_range(TChars&& chars) noexcept
{
	for (auto& ch : std::forward<TChars>(chars))
	{
		ch = to_upper(ch);
	}
}

} // namespace ascii
} // namespace bstone

#endif // !BSTONE_ASCII_INCLUDED

