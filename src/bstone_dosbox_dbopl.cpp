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


#include "bstone_precompiled.h"

#include <algorithm>

#include "dbopl.h"

#include "bstone_opl3.h"


namespace bstone
{


//
// A wrapper for DOSBox DBOPL.
//
class DosboxDbopl final :
	public Opl3
{
public:
	DosboxDbopl();

	Opl3Type get_type() const noexcept override;

	// Initializes the emulator with a specified output sample rate.
	void initialize(
		const int sample_rate) override;

	// Uninitializes the emulator.
	void uninitialize() override;

	// Returns true if the wrapper initialized or false otherwise.
	bool is_initialized() const noexcept override;

	// Returns an output sample rate.
	int get_sample_rate() const noexcept override;

	// Writes a value into a register.
	void write(
		const int fm_port,
		const int fm_value) override;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	bool generate(
		const int count,
		std::int16_t* buffer) override;

	// Resets the emulator.
	bool reset() override;

	// Returns a minimum output sample rate.
	// (Emulator depandant value)
	int get_min_sample_rate() const noexcept override;


private:
	bool is_initialized_;
	int sample_rate_;
	DBOPL::Handler emulator_;
	MixerChannel channel_;


	// Returns a maximum number of output samples generated at once.
	// (Emulator dependent value)
	static int get_max_samples_count() noexcept;
}; // DosboxDbopl


DosboxDbopl::DosboxDbopl()
	:
	is_initialized_{},
	sample_rate_{},
	emulator_{},
	channel_{}
{
}

Opl3Type DosboxDbopl::get_type() const noexcept
{
	return Opl3Type::dbopl;
}

void DosboxDbopl::initialize(
	const int sample_rate)
{
	uninitialize();

	sample_rate_ = std::max(sample_rate, get_min_sample_rate());

	channel_ = {};

	emulator_ = {};
	emulator_.Init(sample_rate_);

	is_initialized_ = true;
}

void DosboxDbopl::uninitialize()
{
	is_initialized_ = false;
	sample_rate_ = 0;
	emulator_ = {};
	channel_ = {};
}

bool DosboxDbopl::is_initialized() const noexcept
{
	return is_initialized_;
}

int DosboxDbopl::get_sample_rate() const noexcept
{
	return sample_rate_;
}

void DosboxDbopl::write(
	const int fm_port,
	const int fm_value)
{
	if (!is_initialized_)
	{
		return;
	}

	emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

bool DosboxDbopl::generate(
	const int count,
	std::int16_t* buffer)
{
	if (!is_initialized_)
	{
		return false;
	}

	if (count < 1)
	{
		return false;
	}

	if (!buffer)
	{
		return false;
	}

	auto remain_count = count;

	while (remain_count > 0)
	{
		const auto generate_count = std::min(remain_count, get_max_samples_count());

		channel_.set_buffer(buffer);

		emulator_.Generate(&channel_, static_cast<Bitu>(generate_count));

		remain_count -= generate_count;
		buffer += generate_count;
	}

	return true;
}

bool DosboxDbopl::reset()
{
	if (!is_initialized_)
	{
		return false;
	}

	initialize(sample_rate_);

	return true;
}

int DosboxDbopl::get_min_sample_rate() const noexcept
{
	return 8000;
}

int DosboxDbopl::get_max_samples_count() noexcept
{
	return 512;
}


namespace detail
{


Opl3UPtr make_dbopl_opl3()
{
	return std::make_unique<DosboxDbopl>();
}


} // detail


} // bstone
