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


#include "bstone_low_pass_filter.h"

#include <cmath>

#include <algorithm>


namespace bstone
{


class LowPassFilterException :
	public Exception
{
public:
	explicit LowPassFilterException(
		const char* const message)
		:
		Exception{std::string{"[AUDIO_LPF] "} + message}
	{
	}
}; // LowPassFilterException


LowPassFilter::LowPassFilter()
	:
	length_{},
	half_length_{},
	left_length_{},
	weights_{},
	samples_{}
{
}

LowPassFilter::LowPassFilter(
	const int filter_order,
	const int cut_off_frequency,
	const int sampling_frequency)
	:
	LowPassFilter{}
{
	initialize(filter_order, cut_off_frequency, sampling_frequency);
}

void LowPassFilter::initialize(
	const int filter_order,
	const int cut_off_frequency,
	const int sampling_frequency)
{
	if (filter_order < 1)
	{
		throw LowPassFilterException{"Filter order out of range."};
	}

	if (cut_off_frequency <= 0)
	{
		throw LowPassFilterException{"Cut off frequency out of range."};
	}

	if (sampling_frequency <= 0)
	{
		throw LowPassFilterException{"Sampling frequency out of range."};
	}

	if (cut_off_frequency > (sampling_frequency / 2))
	{
		throw LowPassFilterException{"Cut off frequency out of range."};
	}

	initialize_weights(filter_order, cut_off_frequency, sampling_frequency);

	apply_hann_weights();
}

double LowPassFilter::process_sample(
	const double sample)
{
	// Shift samples to the right.
	//
	for (int i = length_ - 2; i >= 0; --i)
	{
		samples_[i + 1] = samples_[i];
	}

	// Set new sample.
	//
	samples_.front() = sample;

	// Apply weights.
	//
	auto result = 0.0;

	for (int i = 0; i < half_length_; ++i)
	{
		result += (samples_[i] + samples_[length_ - 1 - i]) * weights_[i];
	}

	if (left_length_ != half_length_)
	{
		result += samples_[half_length_] * weights_.back();
	}

	return result;
}

void LowPassFilter::reset_samples()
{
	std::uninitialized_fill(samples_.begin(), samples_.end(), 0.0);
}

double LowPassFilter::get_pi() noexcept
{
	return 3.141'592'653'589'793'238'462'643'383'280;
}

void LowPassFilter::initialize_weights(
	const int filter_order,
	const int cut_off_frequency,
	const int sampling_frequency)
{
	// Filter length.
	length_ = filter_order + 1;

	// Filter half length.
	half_length_ = length_ / 2;

	// Filter half length with a peak.
	left_length_ = (length_ + 1) / 2;

	weights_.resize(left_length_);
	samples_.resize(length_);

	const auto fp_half_order = filter_order / 2.0;
	const auto ft = static_cast<double>(cut_off_frequency) / static_cast<double>(sampling_frequency);
	const auto pi_2_ft = 2.0 * get_pi() * ft;

	for (int i = 0; i < half_length_; ++i)
	{
		const auto fp_index = i - fp_half_order;
		const auto w = std::sin(pi_2_ft * fp_index) / (get_pi() * fp_index);

		weights_[i] = w;
	}

	if (left_length_ != half_length_)
	{
		weights_.back() = 2.0 * ft;
	}
}

void LowPassFilter::apply_hann_weights()
{
	const auto order = length_ - 1;
	const auto pi_2_div_order = (2.0 * get_pi()) / order;

	for (int i = 0; i < left_length_; ++i)
	{
		const auto w = 0.5 * (1.0 - std::cos(i * pi_2_div_order));

		weights_[i] *= w;
	}
}


} // bstone
