/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#ifndef BSTONE_GFXV_INCLUDED
#define BSTONE_GFXV_INCLUDED


extern int16_t TELEPORT_LUMP_START;
extern int16_t TELEPORT_LUMP_END;

extern int16_t README_LUMP_START;
extern int16_t README_LUMP_END;

extern int16_t CONTROLS_LUMP_START;
extern int16_t CONTROLS_LUMP_END;

extern int16_t LATCHPICS_LUMP_START;
extern int16_t LATCHPICS_LUMP_END;


//
// Amount of each data item
//
extern int16_t NUMCHUNKS;
extern int16_t NUMFONT;
extern int16_t NUMFONTM;
extern int16_t NUMPICS;
extern int16_t NUMPICM;
extern int16_t NUMSPRITES;
extern int16_t NUMTILE8;
extern int16_t NUMTILE8M;
extern int16_t NUMTILE16;
extern int16_t NUMTILE16M;
extern int16_t NUMTILE32;
extern int16_t NUMTILE32M;
extern int16_t NUMEXTERNS;

//
// File offsets for data items
//
extern int16_t STRUCTPIC;

extern int16_t STARTFONT;
extern int16_t STARTFONTM;
extern int16_t STARTPICS;
extern int16_t STARTPICM;
extern int16_t STARTSPRITES;
extern int16_t STARTTILE8;
extern int16_t STARTTILE8M;
extern int16_t STARTTILE16;
extern int16_t STARTTILE16M;
extern int16_t STARTTILE32;
extern int16_t STARTTILE32M;
extern int16_t STARTEXTERNS;


/* FIXME Unknown AOG pictures
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


extern int16_t TELEPORTBACKPIC;
extern int16_t TELEPORT1OFFPIC;
extern int16_t TELEPORT2OFFPIC;
extern int16_t TELEPORT3OFFPIC;
extern int16_t TELEPORT4OFFPIC;
extern int16_t TELEPORT5OFFPIC;
extern int16_t TELEPORT6OFFPIC;
extern int16_t TELEPORT7OFFPIC;
extern int16_t TELEPORT8OFFPIC;
extern int16_t TELEPORT9OFFPIC;
extern int16_t TELEPORT10OFFPIC;
extern int16_t TELEPORT1ONPIC;
extern int16_t TELEPORT2ONPIC;
extern int16_t TELEPORT3ONPIC;
extern int16_t TELEPORT4ONPIC;
extern int16_t TELEPORT5ONPIC;
extern int16_t TELEPORT6ONPIC;
extern int16_t TELEPORT7ONPIC;
extern int16_t TELEPORT8ONPIC;
extern int16_t TELEPORT9ONPIC;
extern int16_t TELEPORT10ONPIC;
extern int16_t TELEPORT_TEXT_BG;
extern int16_t BACKGROUND_SCREENPIC;
extern int16_t APOGEEPIC;
extern int16_t PIRACYPIC;
extern int16_t PC13PIC;
extern int16_t LOSEPIC;
extern int16_t AUTOMAPPIC;
extern int16_t PSPROMO1PIC;
extern int16_t PSPROMO2PIC;
extern int16_t PSPROMO3PIC;
extern int16_t H_ALTPIC;
extern int16_t H_CTRLPIC;
extern int16_t H_SPACEPIC;
extern int16_t H_PAUSEPIC;
extern int16_t H_ESCPIC;
extern int16_t H_LTARROWPIC;
extern int16_t H_UPARROWPIC;
extern int16_t H_DNARROWPIC;
extern int16_t H_RTARROWPIC;
extern int16_t H_ENTERPIC;
extern int16_t H_QPIC;
extern int16_t H_WPIC;
extern int16_t H_EPIC;
extern int16_t H_IPIC;
extern int16_t H_HPIC;
extern int16_t H_1PIC;
extern int16_t H_2PIC;
extern int16_t H_3PIC;
extern int16_t H_4PIC;
extern int16_t H_5PIC;
extern int16_t H_F1PIC;
extern int16_t H_F2PIC;
extern int16_t H_F3PIC;
extern int16_t H_F4PIC;
extern int16_t H_F5PIC;
extern int16_t H_F6PIC;
extern int16_t H_F7PIC;
extern int16_t H_F8PIC;
extern int16_t H_F9PIC;
extern int16_t H_F10PIC;
extern int16_t H_TABPIC;
extern int16_t H_CPIC;
extern int16_t H_FPIC;
extern int16_t H_PPIC;
extern int16_t H_MPIC;
extern int16_t H_LPIC;
extern int16_t H_SHIFTPIC;
extern int16_t APOGEE_LOGOPIC;
extern int16_t VISAPIC;
extern int16_t MCPIC;
extern int16_t FAXPIC;
extern int16_t H_TOPWINDOWPIC;
extern int16_t H_LEFTWINDOWPIC;
extern int16_t H_RIGHTWINDOWPIC;
extern int16_t H_BOTTOMINFOPIC;
extern int16_t C_NOTSELECTEDPIC;
extern int16_t C_SELECTEDPIC;
extern int16_t C_NOTSELECTED_HIPIC;
extern int16_t C_SELECTED_HIPIC;
extern int16_t C_BABYMODEPIC;
extern int16_t C_EASYPIC;
extern int16_t C_NORMALPIC;
extern int16_t C_HARDPIC;
extern int16_t C_EPISODE1PIC;
extern int16_t C_EPISODE2PIC;
extern int16_t C_EPISODE3PIC;
extern int16_t C_EPISODE4PIC;
extern int16_t C_EPISODE5PIC;
extern int16_t C_EPISODE6PIC;
extern int16_t BIGGOLDSTERNPIC;
extern int16_t STARLOGOPIC;
extern int16_t BLAKEWITHGUNPIC;
extern int16_t STARINSTITUTEPIC;
extern int16_t MEDALOFHONORPIC;
extern int16_t SMALLGOLDSTERNPIC;
extern int16_t BLAKEWINPIC;
extern int16_t SHUTTLEEXPPIC;
extern int16_t PLANETSPIC;
extern int16_t MOUSEPIC;
extern int16_t JOYSTICKPIC;
extern int16_t GRAVISPADPIC;
extern int16_t TITLEPIC;
extern int16_t PROMO1PIC;
extern int16_t PROMO2PIC;
extern int16_t WEAPON1PIC;
extern int16_t WEAPON2PIC;
extern int16_t WEAPON3PIC;
extern int16_t WEAPON4PIC;
extern int16_t WEAPON5PIC;
extern int16_t WAITPIC;
extern int16_t READYPIC;
extern int16_t N_BLANKPIC;
extern int16_t N_0PIC;
extern int16_t N_1PIC;
extern int16_t N_2PIC;
extern int16_t N_3PIC;
extern int16_t N_4PIC;
extern int16_t N_5PIC;
extern int16_t N_6PIC;
extern int16_t N_7PIC;
extern int16_t N_8PIC;
extern int16_t N_9PIC;
extern int16_t N_RPIC;
extern int16_t N_OPIC;
extern int16_t N_LPIC;
extern int16_t N_DASHPIC;
extern int16_t DIM_LIGHTPIC;
extern int16_t BRI_LIGHTPIC;
extern int16_t ECG_HEART_GOOD;
extern int16_t ECG_HEART_BAD;
extern int16_t ECG_GRID_PIECE;
extern int16_t AMMONUM_BACKGR;
extern int16_t ECG_HEARTBEAT_00;
extern int16_t ECG_HEARTBEAT_01;
extern int16_t ECG_HEARTBEAT_02;
extern int16_t ECG_HEARTBEAT_03;
extern int16_t ECG_HEARTBEAT_04;
extern int16_t ECG_HEARTBEAT_05;
extern int16_t ECG_HEARTBEAT_06;
extern int16_t ECG_HEARTBEAT_07;
extern int16_t ECG_HEARTBEAT_08;
extern int16_t ECG_HEARTBEAT_09;
extern int16_t ECG_HEARTBEAT_10;
extern int16_t ECG_HEARTBEAT_11;
extern int16_t ECG_HEARTBEAT_12;
extern int16_t ECG_HEARTBEAT_13;
extern int16_t ECG_HEARTBEAT_14;
extern int16_t ECG_HEARTBEAT_15;
extern int16_t ECG_HEARTBEAT_16;
extern int16_t ECG_HEARTBEAT_17;
extern int16_t ECG_HEARTBEAT_18;
extern int16_t ECG_HEARTBEAT_19;
extern int16_t ECG_HEARTBEAT_20;
extern int16_t ECG_HEARTBEAT_21;
extern int16_t ECG_HEARTBEAT_22;
extern int16_t ECG_HEARTBEAT_23;
extern int16_t ECG_HEARTBEAT_24;
extern int16_t ECG_HEARTBEAT_25;
extern int16_t ECG_HEARTBEAT_26;
extern int16_t ECG_HEARTBEAT_27;
extern int16_t INFOAREAPIC;
extern int16_t TOP_STATUSBARPIC;
extern int16_t STATUSBARPIC;
extern int16_t PIRACYPALETTE;
extern int16_t APOGEEPALETTE;
extern int16_t TITLEPALETTE;
extern int16_t ORDERSCREEN;
extern int16_t ERRORSCREEN;
extern int16_t INFORMANT_HINTS;
extern int16_t NICE_SCIE_HINTS;
extern int16_t MEAN_SCIE_HINTS;
extern int16_t BRIEF_W1;
extern int16_t BRIEF_I1;
extern int16_t BRIEF_W2;
extern int16_t BRIEF_I2;
extern int16_t BRIEF_W3;
extern int16_t BRIEF_I3;
extern int16_t BRIEF_W4;
extern int16_t BRIEF_I4;
extern int16_t BRIEF_W5;
extern int16_t BRIEF_I5;
extern int16_t BRIEF_W6;
extern int16_t BRIEF_I6;
extern int16_t LEVEL_DESCS;
extern int16_t POWERBALLTEXT;
extern int16_t TICSTEXT;
extern int16_t MUSICTEXT;
extern int16_t RADARTEXT;
extern int16_t HELPTEXT;
extern int16_t SAGATEXT;
extern int16_t LOSETEXT;
extern int16_t ORDERTEXT;
extern int16_t CREDITSTEXT;
extern int16_t MUSTBE386TEXT;
extern int16_t QUICK_INFO1_TEXT;
extern int16_t QUICK_INFO2_TEXT;
extern int16_t BADINFO_TEXT;
extern int16_t CALJOY1_TEXT;
extern int16_t CALJOY2_TEXT;
extern int16_t READTHIS_TEXT;
extern int16_t ELEVMSG0_TEXT;
extern int16_t ELEVMSG1_TEXT;
extern int16_t ELEVMSG4_TEXT;
extern int16_t ELEVMSG5_TEXT;
extern int16_t FLOORMSG_TEXT;
extern int16_t YOUWIN_TEXT;
extern int16_t CHANGEVIEW_TEXT;
extern int16_t BADCHECKSUMTEXT;
extern int16_t DIZ_ERR_TEXT;
extern int16_t BADLEVELSTEXT;
extern int16_t BADSAVEGAME_TEXT;

extern int16_t TELEPORTBACKTOPPIC;
extern int16_t TELEPORTBACKBOTPIC;
extern int16_t TELEPORT11ONPIC;
extern int16_t TELEPORT12ONPIC;
extern int16_t TELEPORT13ONPIC;
extern int16_t TELEPORT14ONPIC;
extern int16_t TELEPORT15ONPIC;
extern int16_t TELEPORT16ONPIC;
extern int16_t TELEPORT17ONPIC;
extern int16_t TELEPORT18ONPIC;
extern int16_t TELEPORT19ONPIC;
extern int16_t TELEPORT20ONPIC;
extern int16_t TELEUPONPIC;
extern int16_t TELEDNONPIC;
extern int16_t TELEUPOFFPIC;
extern int16_t TELEDNOFFPIC;
extern int16_t TELEPORT11OFFPIC;
extern int16_t TELEPORT12OFFPIC;
extern int16_t TELEPORT13OFFPIC;
extern int16_t TELEPORT14OFFPIC;
extern int16_t TELEPORT15OFFPIC;
extern int16_t TELEPORT16OFFPIC;
extern int16_t TELEPORT17OFFPIC;
extern int16_t TELEPORT18OFFPIC;
extern int16_t TELEPORT19OFFPIC;
extern int16_t TELEPORT20OFFPIC;
extern int16_t AUTOMAP_MAG1PIC;
extern int16_t AUTOMAP_MAG2PIC;
extern int16_t AUTOMAP_MAG4PIC;
extern int16_t H_6PIC;
extern int16_t H_TILDEPIC;
extern int16_t H_PLUSPIC;
extern int16_t H_MINUSPIC;
extern int16_t STARPORTPIC;
extern int16_t BOSSPIC;
extern int16_t THREEPLANETSPIC;
extern int16_t SOLARSYSTEMPIC;
extern int16_t AOGENDINGPIC;
extern int16_t GFLOGOSPIC;
extern int16_t BLAKEHEADPIC;
extern int16_t PROJECTFOLDERPIC;
extern int16_t TITLE1PIC;
extern int16_t TITLE2PIC;
extern int16_t WEAPON6PIC;
extern int16_t WEAPON7PIC;
extern int16_t W1_CORNERPIC;
extern int16_t W2_CORNERPIC;
extern int16_t W3_CORNERPIC;
extern int16_t W4_CORNERPIC;
extern int16_t W5_CORNERPIC;
extern int16_t W6_CORNERPIC;
extern int16_t NG_BLANKPIC;
extern int16_t NG_0PIC;
extern int16_t NG_1PIC;
extern int16_t NG_2PIC;
extern int16_t NG_3PIC;
extern int16_t NG_4PIC;
extern int16_t NG_5PIC;
extern int16_t NG_6PIC;
extern int16_t NG_7PIC;
extern int16_t NG_8PIC;
extern int16_t NG_9PIC;
extern int16_t ONEXZOOMPIC;
extern int16_t TWOXZOOMPIC;
extern int16_t FOURXZOOMPIC;
extern int16_t NO_KEYPIC;
extern int16_t RED_KEYPIC;
extern int16_t YEL_KEYPIC;
extern int16_t BLU_KEYPIC;
extern int16_t ENDINGPALETTE;
extern int16_t NO386SCREEN;
extern int16_t T_DEMO0;
extern int16_t T_DEMO1;
extern int16_t T_DEMO2;
extern int16_t T_DEMO3;
extern int16_t T_DEMO4;
extern int16_t T_DEMO5;
extern int16_t DECOY;
extern int16_t DECOY2;
extern int16_t DECOY3;
extern int16_t DECOY4;


#endif // BSTONE_GFXV_INCLUDED
