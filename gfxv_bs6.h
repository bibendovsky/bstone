/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//////////////////////////////////////
//
// Graphics .H file for .BS6
// IGRAB-ed on Wed Mar 30 18:12:07 1994
//
//////////////////////////////////////

enum graphicnums {
    TELEPORTBACKPIC = 6,
    TELEPORT1OFFPIC = 7,
    TELEPORT2OFFPIC = 8,
    TELEPORT3OFFPIC = 9,
    TELEPORT4OFFPIC = 10,
    TELEPORT5OFFPIC = 11,
    TELEPORT6OFFPIC = 12,
    TELEPORT7OFFPIC = 13,
    TELEPORT8OFFPIC = 14,
    TELEPORT9OFFPIC = 15,
    TELEPORT10OFFPIC = 16,
    TELEPORT1ONPIC = 17,
    TELEPORT2ONPIC = 18,
    TELEPORT3ONPIC = 19,
    TELEPORT4ONPIC = 20,
    TELEPORT5ONPIC = 21,
    TELEPORT6ONPIC = 22,
    TELEPORT7ONPIC = 23,
    TELEPORT8ONPIC = 24,
    TELEPORT9ONPIC = 25,
    TELEPORT10ONPIC = 26,
    BACKGROUND_SCREENPIC = 28,
    APOGEEPIC = 29,
    PIRACYPIC = 30,
    PC13PIC = 31,
    LOSEPIC = 32,
    AUTOMAPPIC = 33,
    H_ALTPIC = 34,
    H_CTRLPIC = 35,
    H_SPACEPIC = 36,
    H_PAUSEPIC = 37,
    H_ESCPIC = 38,
    H_LTARROWPIC = 39,
    H_UPARROWPIC = 40,
    H_DNARROWPIC = 41,
    H_RTARROWPIC = 42,
    H_ENTERPIC = 43,
    H_QPIC = 44,
    H_WPIC = 45,
    H_EPIC = 46,
    H_IPIC = 47,
    H_HPIC = 48,
    H_1PIC = 49,
    H_2PIC = 50,
    H_3PIC = 51,
    H_4PIC = 52,
    H_5PIC = 53,
    H_F1PIC = 54,
    H_F2PIC = 55,
    H_F3PIC = 56,
    H_F4PIC = 57,
    H_F5PIC = 58,
    H_F6PIC = 59,
    H_F7PIC = 60,
    H_F8PIC = 61,
    H_F9PIC = 62,
    H_F10PIC = 63,
    H_TABPIC = 64,
    H_CPIC = 65,
    H_FPIC = 66,
    H_PPIC = 67,
    H_MPIC = 68,
    H_LPIC = 69,
    H_SHIFTPIC = 70,
    APOGEE_LOGOPIC = 71,
    VISAPIC = 72,
    MCPIC = 73,
    FAXPIC = 74,
    H_TOPWINDOWPIC = 75,
    H_LEFTWINDOWPIC = 76,
    H_RIGHTWINDOWPIC = 77,
    H_BOTTOMINFOPIC = 78,
    C_NOTSELECTEDPIC = 79,
    C_SELECTEDPIC = 80,
    C_NOTSELECTED_HIPIC = 81,
    C_SELECTED_HIPIC = 82,
    C_BABYMODEPIC = 83,
    C_EASYPIC = 84,
    C_NORMALPIC = 85,
    C_HARDPIC = 86,
    C_EPISODE1PIC = 87,
    C_EPISODE2PIC = 88,
    C_EPISODE3PIC = 89,
    C_EPISODE4PIC = 90,
    C_EPISODE5PIC = 91,
    C_EPISODE6PIC = 92,
    BIGGOLDSTERNPIC = 93,
    STARLOGOPIC = 94,
    BLAKEWITHGUNPIC = 95,
    STARINSTITUTEPIC = 96,
    MEDALOFHONORPIC = 97,
    SMALLGOLDSTERNPIC = 98,
    BLAKEWINPIC = 99,
    SHUTTLEEXPPIC = 100,
    PLANETSPIC = 101,
    MOUSEPIC = 102,
    JOYSTICKPIC = 103,
    GRAVISPADPIC = 104,
    TITLEPIC = 105,
    WEAPON1PIC = 106,
    WEAPON2PIC = 107,
    WEAPON3PIC = 108,
    WEAPON4PIC = 109,
    WEAPON5PIC = 110,
    WAITPIC = 111,
    READYPIC = 112,
    N_BLANKPIC = 113,
    N_0PIC = 114,
    N_1PIC = 115,
    N_2PIC = 116,
    N_3PIC = 117,
    N_4PIC = 118,
    N_5PIC = 119,
    N_6PIC = 120,
    N_7PIC = 121,
    N_8PIC = 122,
    N_9PIC = 123,
    N_RPIC = 124,
    N_OPIC = 125,
    N_LPIC = 126,
    N_DASHPIC = 127,
    DIM_LIGHTPIC = 128,
    BRI_LIGHTPIC = 129,
    ECG_HEART_GOOD = 130,
    ECG_HEART_BAD = 131,
    ECG_GRID_PIECE = 132,
    ECG_HEARTBEAT_00 = 134,
    ECG_HEARTBEAT_01 = 135,
    ECG_HEARTBEAT_02 = 136,
    ECG_HEARTBEAT_03 = 137,
    ECG_HEARTBEAT_04 = 138,
    ECG_HEARTBEAT_05 = 139,
    ECG_HEARTBEAT_06 = 140,
    ECG_HEARTBEAT_07 = 141,
    ECG_HEARTBEAT_08 = 142,
    ECG_HEARTBEAT_09 = 143,
    ECG_HEARTBEAT_10 = 144,
    ECG_HEARTBEAT_11 = 145,
    ECG_HEARTBEAT_12 = 146,
    ECG_HEARTBEAT_13 = 147,
    ECG_HEARTBEAT_14 = 148,
    ECG_HEARTBEAT_15 = 149,
    ECG_HEARTBEAT_16 = 150,
    ECG_HEARTBEAT_17 = 151,
    ECG_HEARTBEAT_18 = 152,
    ECG_HEARTBEAT_19 = 153,
    ECG_HEARTBEAT_20 = 154,
    ECG_HEARTBEAT_21 = 155,
    ECG_HEARTBEAT_22 = 156,
    ECG_HEARTBEAT_23 = 157,
    ECG_HEARTBEAT_24 = 158,
    ECG_HEARTBEAT_25 = 159,
    ECG_HEARTBEAT_26 = 160,
    ECG_HEARTBEAT_27 = 161,
    INFOAREAPIC = 162,
    TOP_STATUSBARPIC = 163,
    STATUSBARPIC = 164,
    PIRACYPALETTE = 167,
    APOGEEPALETTE = 168,
    TITLEPALETTE = 169,
    ORDERSCREEN = 170,
    ERRORSCREEN = 171,
    INFORMANT_HINTS = 181,
    NICE_SCIE_HINTS = 182,
    MEAN_SCIE_HINTS = 183,
    BRIEF_W1 = 184,
    BRIEF_I1 = 185,
    BRIEF_W2 = 186,
    BRIEF_I2 = 187,
    BRIEF_W3 = 188,
    BRIEF_I3 = 189,
    BRIEF_W4 = 190,
    BRIEF_I4 = 191,
    BRIEF_W5 = 192,
    BRIEF_I5 = 193,
    BRIEF_W6 = 194,
    BRIEF_I6 = 195,
    LEVEL_DESCS = 196,
    POWERBALLTEXT = 197,
    TICSTEXT = 198,
    MUSICTEXT = 199,
    RADARTEXT = 200,
    HELPTEXT = 201,
    SAGATEXT = 202,
    LOSETEXT = 203,
    ORDERTEXT = 204,
    CREDITSTEXT = 205,
    MUSTBE386TEXT = 206,
    QUICK_INFO1_TEXT = 207,
    QUICK_INFO2_TEXT = 208,
    BADINFO_TEXT = 209,
    CALJOY1_TEXT = 210,
    CALJOY2_TEXT = 211,
    READTHIS_TEXT = 212,
    ELEVMSG0_TEXT = 213,
    ELEVMSG1_TEXT = 214,
    ELEVMSG4_TEXT = 215,
    ELEVMSG5_TEXT = 216,
    FLOORMSG_TEXT = 217,
    YOUWIN_TEXT = 218,
    CHANGEVIEW_TEXT = 219,
    DIZ_ERR_TEXT = 220,
    BADLEVELSTEXT = 221,
    BADSAVEGAME_TEXT = 222,

/* FIXME Unknown pictures
	TELEPORT11ONPIC,                     // 17
	TELEPORT12ONPIC,                     // 18
	TELEPORT13ONPIC,                     // 19
	TELEPORT14ONPIC,                     // 20
	TELEPORT15ONPIC,                     // 21
	TELEPORT16ONPIC,                     // 22
	TELEPORT17ONPIC,                     // 23
	TELEPORT18ONPIC,                     // 24
	TELEPORT19ONPIC,                     // 25
	TELEPORT20ONPIC,                     // 26
	TELEUPONPIC,                         // 27
	TELEDNONPIC,                         // 28
	TELEUPOFFPIC,                        // 29
	TELEDNOFFPIC,                        // 30
	TELEPORT11OFFPIC,                    // 41
	TELEPORT12OFFPIC,                    // 42
	TELEPORT13OFFPIC,                    // 43
	TELEPORT14OFFPIC,                    // 44
	TELEPORT15OFFPIC,                    // 45
	TELEPORT16OFFPIC,                    // 46
	TELEPORT17OFFPIC,                    // 47
	TELEPORT18OFFPIC,                    // 48
	TELEPORT19OFFPIC,                    // 49
	TELEPORT20OFFPIC,                    // 50
	AUTOMAP_MAG1PIC,                     // 57
	AUTOMAP_MAG2PIC,                     // 58
	AUTOMAP_MAG4PIC,                     // 59
	H_6PIC,                              // 97
	H_TILDEPIC,                          // 98
	STARPORTPIC,                         // 133
	BOSSPIC,                             // 134
	THREEPLANETSPIC,                     // 135
	SOLARSYSTEMPIC,                      // 136
	AOGENDINGPIC,                        // 137
	GFLOGOSPIC,                          // 138
	BLAKEHEADPIC,                        // 139
	PROJECTFOLDERPIC,                    // 140
	WEAPON6PIC,                          // 147
	WEAPON7PIC,                          // 148
	W1_CORNERPIC,                        // 149
	W2_CORNERPIC,                        // 150
	W3_CORNERPIC,                        // 151
	W4_CORNERPIC,                        // 152
	W5_CORNERPIC,                        // 153
	W6_CORNERPIC,                        // 154
	NG_BLANKPIC,                         // 172
	NG_0PIC,                             // 173
	NG_1PIC,                             // 174
	NG_2PIC,                             // 175
	NG_3PIC,                             // 176
	NG_4PIC,                             // 177
	NG_5PIC,                             // 178
	NG_6PIC,                             // 179
	NG_7PIC,                             // 180
	NG_8PIC,                             // 181
	NG_9PIC,                             // 182
	NO_KEYPIC,                           // 191
	RED_KEYPIC,                          // 192
	YEL_KEYPIC,                          // 193
	BLU_KEYPIC,                          // 194
	ENDINGPALETTE,                       // 199
	NO386SCREEN,                         // 203
	T_DEMO0,                             // 207
	T_DEMO1,                             // 208
	T_DEMO2,                             // 209
	T_DEMO3,                             // 210
	T_DEMO4,                             // 211
	T_DEMO5,                             // 212
	DECOY,                               // 229
	DECOY2,                              // 230
	DECOY3,                              // 231
	DECOY4,                              // 232
*/

	ENUMEND
}; // enum graphicnums

//
// Data LUMPs
//
#define TELEPORT_LUMP_START		7
#define TELEPORT_LUMP_END		50

#define README_LUMP_START		60
#define README_LUMP_END			106

#define CONTROLS_LUMP_START		C_NOTSELECTEDPIC
#define CONTROLS_LUMP_END		C_EPISODE6PIC

#define LATCHPICS_LUMP_START		WEAPON1PIC
#define LATCHPICS_LUMP_END		STATUSBARPIC


//
// Amount of each data item
//
#define NUMCHUNKS    223
#define NUMFONT      5
#define NUMFONTM     0
#define NUMPICS      159
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   58
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   6
#define STARTPICS    6
#define STARTPICM    165
#define STARTSPRITES 165
#define STARTTILE8   165
#define STARTTILE8M  166
#define STARTTILE16  166
#define STARTTILE16M 166
#define STARTTILE32  166
#define STARTTILE32M 166
#define STARTEXTERNS 166

//
// Thank you for using IGRAB!
//
