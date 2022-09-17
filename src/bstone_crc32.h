/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);
}; // Crc32


} // bstone


#endif // !BSTONE_CRC32_INCLUDED
