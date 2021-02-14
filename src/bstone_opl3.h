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
// OPL3 emulator interface.
//


#ifndef BSTONE_OPL3_INCLUDED
#define BSTONE_OPL3_INCLUDED


#include <cstdint>

#include <memory>


namespace bstone
{


class Opl3;


constexpr auto opl3_fixed_frequency = 49'716;


enum class Opl3Type
{
	none,

	// DosBox DBOPL
	dbopl,
}; // Opl3Type


using Opl3UPtr = std::unique_ptr<Opl3>;


//
// OPL3 emulator interface.
//
class Opl3
{
public:
	Opl3() = default;

	virtual ~Opl3() = default;


	virtual Opl3Type get_type() const noexcept = 0;

	// Initializes the emulator with a specified output sample rate.
	virtual void initialize(
		const int sample_rate) = 0;

	// Uninitializes the emulator.
	virtual void uninitialize() = 0;

	// Returns true if the wrapper initialized or false otherwise.
	virtual bool is_initialized() const noexcept = 0;

	// Returns an output sample rate.
	virtual int get_sample_rate() const noexcept = 0;

	// Writes a value into a register.
	virtual void write(
		const int port,
		const int value) = 0;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	virtual bool generate(
		const int count,
		std::int16_t* const buffer) = 0;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	virtual bool generate(
		const int count,
		float* const buffer) = 0;

	// Initializes the emulator with defined earlier sample rate.
	virtual bool reset() = 0;

	// Returns a minimum output sample rate.
	// (Emulator dependant value)
	virtual int get_min_sample_rate() const noexcept = 0;
}; // Opl3


Opl3UPtr make_opl3(
	const Opl3Type opl3_type);


} // bstone


#endif // !BSTONE_OPL3_INCLUDED
