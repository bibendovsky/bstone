/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//
// A wrapper for the DOSBox OPL2 emulator.
//


#include "bstone_opl2.h"


namespace bstone {


Opl2::Opl2() :
    is_initialized_(false),
    sample_rate_(0)
{
}

void Opl2::initialize(
    int sample_rate)
{
    uninitialize();

    sample_rate_ = std::max(sample_rate, get_min_sample_rate());

    emulator_ = DBOPL::Handler();
    emulator_.Init(sample_rate_);

    is_initialized_ = true;
}

void Opl2::uninitialize()
{
    is_initialized_ = false;
    sample_rate_ = 0;
    emulator_ = DBOPL::Handler();
}

bool Opl2::is_initialized() const
{
    return is_initialized_;
}

int Opl2::get_sample_rate() const
{
    return sample_rate_;
}

void Opl2::write(
    int fm_port,
    int fm_value)
{
    if (!is_initialized())
        return;

    emulator_.WriteReg(
        static_cast<Bit32u>(fm_port),
        static_cast<Bit8u>(fm_value));
}

bool Opl2::generate(
    int count,
    int16_t* buffer)
{
    if (!is_initialized())
        return false;

    if (count < 1)
        return false;

    if (!buffer)
        return false;

    while (count > 0) {
        int generate_count = std::min(count, get_max_samples_count());

        channel_.set_buffer(buffer);

        emulator_.Generate(
            &channel_,
            static_cast<Bitu>(generate_count));

        count -= generate_count;
        buffer += generate_count;
    }

    return true;
}

bool Opl2::reset()
{
    if (!is_initialized())
        return false;

    emulator_.Init(get_sample_rate());

    return true;
}

// (static)
int Opl2::get_min_sample_rate()
{
    return 8000;
}

// (static)
int Opl2::get_max_samples_count()
{
    return 512;
}


} // namespace bstone
