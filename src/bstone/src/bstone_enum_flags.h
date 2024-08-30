/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ENUM_FLAGS_INCLUDED
#define BSTONE_ENUM_FLAGS_INCLUDED

#include <type_traits>

//
// Macros:
// - BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(T)
//   Enables bitwise operators for specified enum type T.
//

#define BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(T) \
\
static_assert( \
	std::is_enum<T>::value && !std::is_convertible<T, std::underlying_type_t<T>>::value, \
		"Expected a scoped enumeration type."); \
\
inline constexpr T operator~(T rhs) noexcept \
{ \
	using Value = std::underlying_type_t<T>; \
	return static_cast<T>(~static_cast<Value>(rhs)); \
} \
\
inline constexpr T operator|(T lhs, T rhs) noexcept \
{ \
	using Value = std::underlying_type_t<T>; \
	return static_cast<T>(static_cast<Value>(lhs) | static_cast<Value>(rhs)); \
} \
\
inline constexpr T operator&(T lhs, T rhs) noexcept \
{ \
	using Value = std::underlying_type_t<T>; \
	return static_cast<T>(static_cast<Value>(lhs) & static_cast<Value>(rhs)); \
} \
\
inline constexpr T operator^(T lhs, T rhs) noexcept \
{ \
	using Value = std::underlying_type_t<T>; \
	return static_cast<T>(static_cast<Value>(lhs) ^ static_cast<Value>(rhs)); \
} \
\
inline constexpr T& operator|=(T& lhs, T rhs) noexcept \
{ \
	lhs = lhs | rhs; \
	return lhs; \
} \
\
inline constexpr T& operator&=(T& lhs, T rhs) noexcept \
{ \
	lhs = lhs & rhs; \
	return lhs; \
} \
\
inline constexpr T& operator^=(T& lhs, T rhs) noexcept \
{ \
	lhs = lhs ^ rhs; \
	return lhs; \
} \
\

#endif // !BSTONE_ENUM_FLAGS_INCLUDED
