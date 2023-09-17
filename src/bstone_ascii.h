/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// ASCII utils.

#if !defined(BSTONE_ASCII_INCLUDED)
#define BSTONE_ASCII_INCLUDED

namespace bstone {
namespace ascii {

template<typename TChar>
inline constexpr bool is_binary(TChar ch)
{
	return ch == '0' || ch == '1';
}

template<typename TChar>
inline constexpr bool is_octal(TChar ch)
{
	return ch >= '0' && ch <= '7';
}

template<typename TChar>
inline constexpr bool is_decimal(TChar ch)
{
	return ch >= '0' && ch <= '9';
}

template<typename TChar>
inline constexpr bool is_hex(TChar ch)
{
	return bstone::ascii::is_decimal(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

template<typename TChar>
inline constexpr bool is_lower(TChar ch)
{
	return ch >= 'a' && ch <= 'z';
}

template<typename TChar>
inline constexpr bool is_upper(TChar ch)
{
	return ch >= 'A' && ch <= 'Z';
}

// ==========================================================================

template<typename TChar>
inline constexpr TChar to_lower(TChar ch)
{
	return bstone::ascii::is_upper(ch) ? static_cast<TChar>('a' + ch - 'A') : ch;
}

template<typename TChar>
inline constexpr TChar to_upper(TChar ch)
{
	return bstone::ascii::is_lower(ch) ? static_cast<TChar>('A' + ch - 'a') : ch;
}

// ==========================================================================

template<typename TIter>
inline constexpr void to_lower(TIter chars_first, TIter chars_last)
{
	for (; chars_first != chars_last; ++chars_first)
	{
		auto& ch = *chars_first;
		ch = bstone::ascii::to_lower(ch);
	}
}

template<typename TIter>
inline constexpr void to_upper(TIter chars_first, TIter chars_last)
{
	for (; chars_first != chars_last; ++chars_first)
	{
		auto& ch = *chars_first;
		ch = bstone::ascii::to_upper(ch);
	}
}

} // namespace ascii
} // namespace bstone

#endif // BSTONE_ASCII_INCLUDED
