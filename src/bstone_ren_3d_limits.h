/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// 3D renderer limits.
//


#ifndef BSTONE_REN_3D_LIMITS_INCLUDED
#define BSTONE_REN_3D_LIMITS_INCLUDED


namespace bstone
{


struct Ren3dLimits
{
	static constexpr int min_anisotropy_off = 1;
	static constexpr int min_anisotropy_on = 2;
	static constexpr int max_anisotropy = 16;

	static constexpr int min_aa_off = 1;
	static constexpr int min_aa_on = 2;
	static constexpr int max_aa = 32;


	static constexpr int max_mipmap_count = 31;
}; // Ren3dLimits


} // bstone


#endif // !BSTONE_REN_3D_LIMITS_INCLUDED
