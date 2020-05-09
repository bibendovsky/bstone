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
// Low-pass filter.
//


#include <cstdint>

#include <string>
#include <type_traits>

#include "bstone_exception.h"


namespace bstone
{


class LowPassFilterTException :
	public Exception
{
public:
	explicit LowPassFilterTException(
		const char* const message)
		:
		Exception{std::string{"[AUDIO_LPF] "} + message}
	{
	}
}; // LowPassFilterTException


template<typename TSample>
class LowPassFilterT
{
private:
	static_assert(
		std::is_integral<TSample>::value ||
			std::is_floating_point<TSample>::value,
		"Expected integral or floating-point type."
	);


public:
	LowPassFilterT() noexcept
		:
		alpha_{},
		one_minus_alpha_{},
		previous_sample_{}
	{
	}

	LowPassFilterT(
		const int src_sample_rate,
		const int dst_sample_rate)
		:
		alpha_{},
		one_minus_alpha_{},
		previous_sample_{}
	{
		initialize(src_sample_rate, dst_sample_rate);
	}

	TSample filter(
		const TSample sample) noexcept
	{
		const auto result = static_cast<TSample>(
			(alpha_ * sample) + (one_minus_alpha_ * previous_sample_));

		previous_sample_ = sample;

		return result;
	}

	void initialize(
		const int src_sample_rate,
		const int dst_sample_rate)
	{
		if (src_sample_rate <= 0)
		{
			throw LowPassFilterTException{"Source sample rate out of range."};
		}

		if (dst_sample_rate <= 0)
		{
			throw LowPassFilterTException{"Destination sample rate out of range."};
		}

		if (src_sample_rate == dst_sample_rate)
		{
			throw LowPassFilterTException{"Equal sample rates."};
		}


		//
		// Calculate parameters of low-pass filter for a cutoff frequency `f`.
		//
		// For sample rate `r`:
		// dt = 1 / r
		// rc = 1 / 2 * pi * f
		// alpha = dt / (rc + dt)
		//

		const auto pi = 3.1415926535897931;
		const auto dt = 1.0 / dst_sample_rate;

		// Our cutoff frequency is half of source rate,
		// because it's a maximum frequency allowed by Nyquist.
		const auto rc = 1.0 / (pi * src_sample_rate);

		alpha_ = dt / (rc + dt);
		one_minus_alpha_ = 1.0 - alpha_;
	}

	void reset() noexcept
	{
		previous_sample_ = {};
	}


private:
	double alpha_;
	double one_minus_alpha_;

	TSample previous_sample_;
}; // LowPassFilterT


using LowPassFilterInt16 = LowPassFilterT<std::int16_t>;


} // bstone
