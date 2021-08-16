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
// Low-pass filter.
//
// Based on information at
// http://www.labbookpages.co.uk/audio/firWindowing.html
//


#include <vector>

#include "bstone_exception.h"


namespace bstone
{


class LowPassFilter final
{
public:
	LowPassFilter() noexcept;

	LowPassFilter(
		int filter_order,
		int cut_off_frequency,
		int sampling_frequency);


	void initialize(
		int filter_order,
		int cut_off_frequency,
		int sampling_frequency);

	double process_sample(
		double sample) noexcept;

	void reset_samples() noexcept;


private:
	using Weights = std::vector<double>;
	using Samples = std::vector<double>;


	int length_{};
	int half_length_{};
	int left_length_{};
	Weights weights_{};
	Samples samples_{};


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static double get_pi() noexcept;

	void initialize_weights(
		int filter_order,
		int cut_off_frequency,
		int sampling_frequency);

	void apply_hann_weights() noexcept;
}; // LowPassFilter


} // bstone
