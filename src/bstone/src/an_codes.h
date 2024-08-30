/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_AN_CODES_INCLUDED
#define BSTONE_AN_CODES_INCLUDED


// --------------------------------------------------------------------------
//
// ANIM CODES - DOCS
//
// --------------------------------------------------------------------------
//
//   FI - Fade In the current frame (Last Frame grabbed)
//   FO - Fade Out the current frame (Last Frame grabbed)
//   FB - Fade In with rate (a numeral value should follow in the script)
//        ** MUST be a divisor of 64
//   FE - Fade Out with rate (a numeral value should follow in the script)
//        ** MUST be a divisor of 64
//   SD - Play sounds (a numeral value should follow in the script)
//   GR - Graphic Page (full screen)
//
//   PA - Pause/Delay 'xxxxxx' number of VBLs
//
//
//
//
//

// --------------------------------------------------------------------------
//
// MACROS
//
// --------------------------------------------------------------------------


#define MV_CNVT_CODE(c1, c2) ((std::uint16_t)((c1) | (c2 << 8)))

#define AN_PAUSE MV_CNVT_CODE('P', 'A')
#define AN_SOUND MV_CNVT_CODE('S', 'D')
#define AN_MUSIC MV_CNVT_CODE('M', 'U')
#define AN_PAGE MV_CNVT_CODE('G', 'R')
#define AN_FADE_IN_FRAME MV_CNVT_CODE('F', 'I')
#define AN_FADE_OUT_FRAME MV_CNVT_CODE('F', 'O')
#define AN_FADE_IN MV_CNVT_CODE('F', 'B')
#define AN_FADE_OUT MV_CNVT_CODE('F', 'E')
#define AN_PALETTE MV_CNVT_CODE('P', 'L')

#define AN_PRELOAD_BEGIN MV_CNVT_CODE('L', 'B')
#define AN_PRELOAD_END MV_CNVT_CODE('L', 'E')

#define AN_END_OF_ANIM MV_CNVT_CODE('X', 'X')


#endif // BSTONE_AN_CODES_INCLUDED
