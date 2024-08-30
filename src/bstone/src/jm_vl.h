/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_JM_VL_INCLUDED
#define BSTONE_JM_VL_INCLUDED


// ==========================================================================
//
//  JM_VL.h     - Headerfile for JM_VL.ASM
//
//
//
//
//
// --------------------------------------------------------------------------
//     Copyright (c) 1993 - JAM Productions Inc, All rights reserved.
// ==========================================================================


#include <cstdint>


// ---------------------------------------------------------------------
// VGA Routines
// ---------------------------------------------------------------------
void JM_VGALinearFill(
	int start,
	int length,
	std::uint8_t fill);



#endif // BSTONE_JM_VL_INCLUDED
