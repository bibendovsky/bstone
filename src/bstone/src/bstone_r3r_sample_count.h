/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Sample count
//
// A set of sample counts is a bit mask where the bit N stands for the sample count 1 << N.

#ifndef BSTONE_R3R_SAMPLE_COUNT_INCLUDED
#define BSTONE_R3R_SAMPLE_COUNT_INCLUDED

#include "bstone_r3r_limits.h"

namespace bstone {

struct R3rSampleCount
{
	// The anti-aliasing limit is inclusive, hence the bit of the limit itself belongs to the mask.
	static constexpr unsigned int supported_bitmask =
		static_cast<unsigned int>(R3rLimits::max_aa) |
		static_cast<unsigned int>(R3rLimits::max_aa - 1);

	// Drops the sample counts the renderers don't expose.
	static constexpr unsigned int clamp_bitmask(unsigned int bitmask)
	{
		return bitmask & supported_bitmask;
	}

	// Returns the greatest supported sample count of the mask not exceeding the degree,
	// or one when there is none.
	static constexpr int choose(unsigned int bitmask, int degree)
	{
		const unsigned int clamped_bitmask = clamp_bitmask(bitmask);
		for (int sample_count = R3rLimits::max_aa; sample_count > 1; sample_count /= 2)
		{
			if (sample_count <= degree && (clamped_bitmask & static_cast<unsigned int>(sample_count)) != 0)
			{
				return sample_count;
			}
		}
		return 1;
	}

	// Returns the greatest supported sample count of the mask, or one for an empty mask.
	static constexpr int get_max(unsigned int bitmask)
	{
		return choose(bitmask, R3rLimits::max_aa);
	}
};

} // namespace bstone

#endif // BSTONE_R3R_SAMPLE_COUNT_INCLUDED
