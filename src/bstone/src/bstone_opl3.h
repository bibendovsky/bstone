/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	// DosBox
	dbopl,
	// Nuked
	nuked,
}; // Opl3Type

// ==========================================================================

//
// OPL3 emulator interface.
//
class Opl3
{
public:
	Opl3() noexcept;
	virtual ~Opl3();

	virtual Opl3Type get_type() const noexcept = 0;

	// Initializes the emulator with a specified output sample rate.
	virtual void initialize(int sample_rate) = 0;

	// Uninitializes the emulator.
	virtual void uninitialize() = 0;

	// Returns true if the wrapper initialized or false otherwise.
	virtual bool is_initialized() const noexcept = 0;

	// Returns an output sample rate.
	virtual int get_sample_rate() const noexcept = 0;

	// Writes a value into a register.
	virtual void write(int port, int value) = 0;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	virtual bool generate(int count, std::int16_t* buffer) = 0;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	virtual bool generate(int count, float* buffer) = 0;

	// Initializes the emulator with defined earlier sample rate.
	virtual bool reset() = 0;

	// Returns a minimum output sample rate.
	// (Emulator dependant value)
	virtual int get_min_sample_rate() const noexcept = 0;
}; // Opl3

// ==========================================================================

using Opl3UPtr = std::unique_ptr<Opl3>;

Opl3UPtr make_opl3(Opl3Type opl3_type);

} // bstone

#endif // !BSTONE_OPL3_INCLUDED
