/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OPL3 emulator interface

#ifndef BSTONE_OPL3_INCLUDED
#define BSTONE_OPL3_INCLUDED

#include <cstdint>
#include <memory>

namespace bstone {

class Opl3;

inline constexpr int opl3_fixed_frequency = 49'716;

enum class Opl3Type
{
	none,
	// DOSBox OPL
	dbopl,
	// Nuked OPL3
	nuked,
};

struct Opl3InitParam
{
	int sample_rate;
};

class Opl3
{
public:
	Opl3() = default;
	virtual ~Opl3() = default;

	virtual Opl3Type get_type() const = 0;

	// Initializes the emulator with a specified output sample rate.
	virtual void initialize(const Opl3InitParam& param) = 0;

	// Uninitializes the emulator.
	virtual void uninitialize() = 0;

	// Returns true if the wrapper initialized or false otherwise.
	virtual bool is_initialized() const = 0;

	// Returns an output sample rate.
	virtual int get_sample_rate() const = 0;

	// Returns channel count.
	virtual int get_channel_count() const = 0;

	// Writes a value into a register.
	virtual void write(int port, int value) = 0;

	// Writes a value into a register with delay.
	virtual void write_buffered(int port, int value) = 0;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	virtual bool generate(int count, float* buffer) = 0;

	// Initializes the emulator with defined earlier sample rate.
	virtual bool reset() = 0;

	// Returns a minimum output sample rate.
	// (Emulator dependant value)
	virtual int get_min_sample_rate() const = 0;
};

// =====================================

using Opl3UPtr = std::unique_ptr<Opl3>;

Opl3UPtr make_opl3(Opl3Type opl3_type);

} // namespace bstone

#endif // BSTONE_OPL3_INCLUDED
