#ifndef _AN_CODES_H_
#define _AN_CODES_H_

//--------------------------------------------------------------------------
//
// 										ANIM CODES - DOCS
//
//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
//
// 										MACROS
//
//--------------------------------------------------------------------------


#define MV_CNVT_CODE(c1,c2)		((unsigned)((c1)|(c2<<8)))

#define AN_PAUSE						MV_CNVT_CODE('P','A')
#define AN_SOUND						MV_CNVT_CODE('S','D')
#define AN_MUSIC						MV_CNVT_CODE('M','U')
#define AN_PAGE 						MV_CNVT_CODE('G','R')
#define AN_FADE_IN_FRAME			MV_CNVT_CODE('F','I')
#define AN_FADE_OUT_FRAME			MV_CNVT_CODE('F','O')
#define AN_FADE_IN					MV_CNVT_CODE('F','B')
#define AN_FADE_OUT					MV_CNVT_CODE('F','E')
#define AN_PALETTE					MV_CNVT_CODE('P','L')

#define AN_PRELOAD_BEGIN		 	MV_CNVT_CODE('L','B')
#define AN_PRELOAD_END				MV_CNVT_CODE('L','E')

#define AN_END_OF_ANIM			 	MV_CNVT_CODE('X','X')

#endif