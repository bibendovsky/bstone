/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_ENDIAN_INCLUDED)
#define BSTONE_ENDIAN_INCLUDED

#include <type_traits>
#include "bstone_platform.h"
#include "bstone_int.h"
#include "bstone_span.h"

#if !defined(BSTONE_LITTLE_ENDIAN)
#	define BSTONE_LITTLE_ENDIAN 1
#endif

#if !defined(BSTONE_BIG_ENDIAN)
#	define BSTONE_BIG_ENDIAN 2
#endif

#if BSTONE_LITTLE_ENDIAN <= 0 || BSTONE_BIG_ENDIAN <= 0 || BSTONE_LITTLE_ENDIAN == BSTONE_BIG_ENDIAN
#	error Invalid endian value.
#endif

#if !defined(BSTONE_ENDIAN)

#	if BSTONE_WIN32 && !BSTONE_MINGW

//
// Visual C++
//
#		if _MSC_VER
#			if (defined(_M_IX86) && _M_IX86 == 600) || (defined(_M_AMD64) && _M_AMD64 == 100)
#				define BSTONE_ENDIAN BSTONE_LITTLE_ENDIAN
#			endif
#		endif

#	else

	//
	// __BYTE_ORDER__
	//
#	if !defined(BSTONE_ENDIAN)
#		if defined(__BYTE_ORDER__)
#			if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#				define BSTONE_ENDIAN BSTONE_BIG_ENDIAN
#			elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#				define BSTONE_ENDIAN BSTONE_LITTLE_ENDIAN
#			else
#				error Unsupported GNU compiler byte order.
#			endif
#		endif
#	endif

//
// __BIG_ENDIAN__ / __LITTLE_ENDIAN__
//
#	if !defined(BSTONE_ENDIAN)
#		if defined(__BIG_ENDIAN__)
#			define BSTONE_ENDIAN BSTONE_BIG_ENDIAN
#		elif defined(__LITTLE_ENDIAN__)
#			define BSTONE_ENDIAN BSTONE_LITTLE_ENDIAN
#		endif
#	endif

#endif

#endif

#if !defined(BSTONE_ENDIAN)
#	error Undefined byte order.
#endif

namespace bstone {
namespace endian {

// ==========================================================================

enum class Type
{
	none = 0,

	little = BSTONE_LITTLE_ENDIAN,
	big = BSTONE_BIG_ENDIAN,

	native =
#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
	little
#endif
#if BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
	big
#endif
	,
}; // Type

// ==========================================================================

namespace detail {

template<typename T>
struct IsSupportedType
{
	static constexpr auto value = std::is_integral<T>::value || std::is_enum<T>::value;
};

struct Bytes1Tag {};
struct Bytes2Tag {};
struct Bytes4Tag {};
struct Bytes8Tag {};
struct IntegralTag {};
struct EnumTag {};

template<typename T>
inline constexpr T swap_bytes(T value, Bytes1Tag) noexcept
{
	return value;
}

template<typename T>
inline constexpr T swap_bytes(T value, Bytes2Tag) noexcept
{
	const auto u16_value = static_cast<UInt16>(value);

	return static_cast<T>(static_cast<UInt16>(
		(u16_value >> 8) |
		(u16_value << 8)));
}

template<typename T>
inline constexpr T swap_bytes(T value, Bytes4Tag) noexcept
{
	const auto u32_value = static_cast<UInt32>(value);

	return static_cast<T>(static_cast<UInt32>(
		(u32_value >> 24) |
		((u32_value >> 8) & 0x00'00'FF'00U) |
		((u32_value << 8) & 0x00'FF'00'00U) |
		(u32_value << 24)));
}

template<typename T>
inline constexpr T swap_bytes(T value, Bytes8Tag) noexcept
{
	const auto u64_value = static_cast<UInt64>(value);

	return static_cast<T>(static_cast<UInt64>(
		(u64_value >> 56) |
		((u64_value >> 40) & 0x00'00'00'00'00'00'FF'00ULL) |
		((u64_value >> 24) & 0x00'00'00'00'00'FF'00'00ULL) |
		((u64_value >> 8) & 0x00'00'00'00'FF'00'00'00ULL) |
		((u64_value << 8) & 0x00'00'00'FF'00'00'00'00ULL) |
		((u64_value << 24) & 0x00'00'FF'00'00'00'00'00ULL) |
		((u64_value << 40) & 0x00'FF'00'00'00'00'00'00ULL) |
		(u64_value << 56)));
}

template<typename T>
inline constexpr T swap_bytes(T value, IntegralTag) noexcept
{
	using Tag = std::conditional_t<
		sizeof(T) == 1,
		Bytes1Tag,
		std::conditional_t<
			sizeof(T) == 2,
			Bytes2Tag,
			std::conditional_t<
				sizeof(T) == 4,
				Bytes4Tag,
				std::conditional_t<
					sizeof(T) == 8,
					Bytes8Tag,
					void>>>>;

	return swap_bytes(value, Tag{});
}

template<typename T>
inline constexpr T swap_bytes(T value, EnumTag) noexcept
{
	return swap_bytes(static_cast<std::underlying_type_t<T>>(value), IntegralTag{});
}

} // namespace detail

// ==========================================================================

template<typename T>
inline constexpr T swap_bytes(T value) noexcept
{
	using Tag = std::conditional_t<
		std::is_integral<std::decay_t<T>>::value,
		detail::IntegralTag,
		std::conditional_t<
			std::is_enum<std::decay_t<T>>::value,
			detail::EnumTag,
			void>>;

	return detail::swap_bytes(value, Tag{});
}

template<typename T, std::enable_if_t<sizeof(T) == 1, int> = 0>
inline constexpr void swap_bytes(Span<T> bytes) noexcept
{
	const auto size = bytes.get_size();
	const auto half_size = size / 2;

	for (auto i = decltype(half_size){}; i < half_size; ++i)
	{
		const auto temp = bytes[i];
		bytes[i] = bytes[size - 1 - i];
		bytes[size - 1 - i] = temp;
	}
}

template<typename T, std::enable_if_t<sizeof(T) == 1, int> = 0>
inline constexpr void swap_bytes(Span<T> bytes, Type type) noexcept
{
	if (type != Type::native)
	{
		swap_bytes(bytes);
	}
}

// ==========================================================================

template<typename T>
inline constexpr T to_little(T value) noexcept
{
#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
	return value;
#endif // BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
#if BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
	return swap_bytes(value);
#endif // BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
}

template<typename T>
inline constexpr T to_big(T value) noexcept
{
#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
	return swap_bytes(value);
#endif // BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
#if BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
	return value;
#endif // BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
}

} // namespace endian
} // namespace bstone

#endif // BSTONE_ENDIAN_INCLUDED
