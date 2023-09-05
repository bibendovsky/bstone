/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Low-pass filter.
//

#include "bstone_low_pass_filter.h"
#include <cmath>
#include <algorithm>

namespace bstone
{

LowPassFilter::LowPassFilter() noexcept = default;

LowPassFilter::LowPassFilter(int filter_order, int cut_off_frequency, int sampling_frequency)
try {
	initialize(filter_order, cut_off_frequency, sampling_frequency);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void LowPassFilter::initialize(int filter_order, int cut_off_frequency, int sampling_frequency)
try {
	if (filter_order < 1)
	{
		BSTONE_THROW_STATIC_SOURCE("Filter order out of range.");
	}

	if (cut_off_frequency <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Cut off frequency out of range.");
	}

	if (sampling_frequency <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Sampling frequency out of range.");
	}

	if (cut_off_frequency > (sampling_frequency / 2))
	{
		BSTONE_THROW_STATIC_SOURCE("Cut off frequency out of range.");
	}

	initialize_weights(filter_order, cut_off_frequency, sampling_frequency);
	apply_hann_weights();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

double LowPassFilter::process_sample(double sample) noexcept
{
	// Shift samples to the right.
	//
	for (auto i = length_ - 2; i >= 0; --i)
	{
		samples_[i + 1] = samples_[i];
	}

	// Set new sample.
	//
	samples_.front() = sample;

	// Apply weights.
	//
	auto result = 0.0;

	for (auto i = 0; i < half_length_; ++i)
	{
		result += (samples_[i] + samples_[length_ - 1 - i]) * weights_[i];
	}

	if (left_length_ != half_length_)
	{
		result += samples_[half_length_] * weights_.back();
	}

	return result;
}

void LowPassFilter::reset_samples() noexcept
{
	std::uninitialized_fill(samples_.begin(), samples_.end(), 0.0);
}

double LowPassFilter::get_pi() noexcept
{
	return 3.141'592'653'589'793'238'462'643'383'280;
}

void LowPassFilter::initialize_weights(int filter_order, int cut_off_frequency, int sampling_frequency)
{
	// Filter's length.
	length_ = filter_order + 1;
	// Filter's half length.
	half_length_ = length_ / 2;
	// Filter's half length with a peak.
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

void LowPassFilter::apply_hann_weights() noexcept
{
	const auto order = length_ - 1;
	const auto pi_2_div_order = (2.0 * get_pi()) / order;

	for (auto i = 0; i < left_length_; ++i)
	{
		const auto w = 0.5 * (1.0 - std::cos(i * pi_2_div_order));

		weights_[i] *= w;
	}
}

} // bstone
