/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// A wrapper for DOSBox DBOPL.
//


#ifndef BSTONE_DOSBOX_DBOPL_INCLUDED
#define BSTONE_DOSBOX_DBOPL_INCLUDED


#include "dbopl.h"


namespace bstone
{


// A wrapper for DOSBox OPL2 emulator.
class DosboxDbopl final
{
public:
	DosboxDbopl();

	// Initializes the emulator with a specified output sample rate.
	void initialize(
		const int sample_rate);

	// Uninitializes the emulator.
	void uninitialize();

	// Returns true if the wrapper initialized or false otherwise.
	bool is_initialized() const;

	// Returns an output sample rate.
	int get_sample_rate() const;

	// Writes a value into a register.
	void write(
		const int fm_port,
		const int fm_value);

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	bool generate(
		const int count,
		std::int16_t* buffer);

	// Resets the emulator.
	bool reset();

	// Returns a minimum output sample rate.
	// (Emulator depandant value)
	static int get_min_sample_rate();


private:
	bool is_initialized_;
	int sample_rate_;
	DBOPL::Handler emulator_;
	MixerChannel channel_;


	// Returns a maximum number of output samples generated at once.
	// (Emulator dependent value)
	static int get_max_samples_count();
}; // DosboxDbopl


} // bstone


#endif // !BSTONE_DOSBOX_DBOPL_INCLUDED
