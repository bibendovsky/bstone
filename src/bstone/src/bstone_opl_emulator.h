/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OPL emulator interface

#ifndef BSTONE_OPL_EMULATOR_INCLUDED
#define BSTONE_OPL_EMULATOR_INCLUDED

#include <memory>

namespace bstone {

enum class OplEmulatorType
{
	none,
	// DOSBox OPL
	dbopl,
	// Nuked OPL3
	nuked_opl3,
};

struct OplEmulatorInitParam
{
	int sample_rate;
};

class OplEmulator
{
public:
	inline static constexpr int fixed_sample_rate = 49716;

	OplEmulator() = default;
	virtual ~OplEmulator() = default;

	virtual OplEmulatorType get_type() const = 0;
	virtual bool initialize(const OplEmulatorInitParam& param) = 0;
	virtual void terminate() = 0;
	virtual bool is_initialized() const = 0;
	virtual int get_sample_rate() const = 0;
	virtual int get_min_sample_rate() const = 0;
	virtual int get_channel_count() const = 0;
	virtual void write_immediate(int address, int value) = 0;
	virtual void write_deferred(int address, int value) = 0;
	virtual void generate_frames(float* samples, int frame_count) = 0;
};

// =====================================

using OplEmulatorUPtr = std::unique_ptr<OplEmulator>;

OplEmulatorUPtr make_opl_emulator(OplEmulatorType opl_emulator_type);

} // namespace bstone

#endif // BSTONE_OPL_EMULATOR_INCLUDED
