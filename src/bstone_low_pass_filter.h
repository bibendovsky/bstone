/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
	LowPassFilter(int filter_order, int cut_off_frequency, int sampling_frequency);

	void initialize(int filter_order, int cut_off_frequency, int sampling_frequency);

	double process_sample(double sample) noexcept;
	void reset_samples() noexcept;

private:
	using Weights = std::vector<double>;
	using Samples = std::vector<double>;

	int length_{};
	int half_length_{};
	int left_length_{};
	Weights weights_{};
	Samples samples_{};

	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);

	static double get_pi() noexcept;

	void initialize_weights(int filter_order, int cut_off_frequency, int sampling_frequency);
	void apply_hann_weights() noexcept;
}; // LowPassFilter

} // bstone
