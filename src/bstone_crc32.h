/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_CRC32_INCLUDED
#define BSTONE_CRC32_INCLUDED


#include <cstdint>


namespace bstone
{


class Crc32 final
{
public:
	using Value = std::uint32_t;


	Crc32() noexcept;


	void reset() noexcept;

	Value get_value() const noexcept;

	void update(
		const void* data,
		int size);


	template<
		typename T
	>
	void update(
		const T& value)
	{
		update(&value, static_cast<int>(sizeof(T)));
	}


private:
	Value value_{};
}; // Crc32


} // bstone


#endif // !BSTONE_CRC32_INCLUDED
