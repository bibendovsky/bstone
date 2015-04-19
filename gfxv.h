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


#ifndef GFXV_INCLUDED
#define GFXV_INCLUDED


extern int TELEPORT_LUMP_START;
extern int TELEPORT_LUMP_END;

extern int README_LUMP_START;
extern int README_LUMP_END;

extern int CONTROLS_LUMP_START;
extern int CONTROLS_LUMP_END;

extern int LATCHPICS_LUMP_START;
extern int LATCHPICS_LUMP_END;


//
// Amount of each data item
//
extern int NUMCHUNKS;
extern int NUMFONT;
extern int NUMFONTM;
extern int NUMPICS;
extern int NUMPICM;
extern int NUMSPRITES;
extern int NUMTILE8;
extern int NUMTILE8M;
extern int NUMTILE16;
extern int NUMTILE16M;
extern int NUMTILE32;
extern int NUMTILE32M;
extern int NUMEXTERNS;

//
// File offsets for data items
//
extern int STRUCTPIC;

extern int STARTFONT;
extern int STARTFONTM;
extern int STARTPICS;
extern int STARTPICM;
extern int STARTSPRITES;
extern int STARTTILE8;
extern int STARTTILE8M;
extern int STARTTILE16;
extern int STARTTILE16M;
extern int STARTTILE32;
extern int STARTTILE32M;
extern int STARTEXTERNS;


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


extern int TELEPORTBACKPIC;
extern int TELEPORT1OFFPIC;
extern int TELEPORT2OFFPIC;
extern int TELEPORT3OFFPIC;
extern int TELEPORT4OFFPIC;
extern int TELEPORT5OFFPIC;
extern int TELEPORT6OFFPIC;
extern int TELEPORT7OFFPIC;
extern int TELEPORT8OFFPIC;
extern int TELEPORT9OFFPIC;
extern int TELEPORT10OFFPIC;
extern int TELEPORT1ONPIC;
extern int TELEPORT2ONPIC;
extern int TELEPORT3ONPIC;
extern int TELEPORT4ONPIC;
extern int TELEPORT5ONPIC;
extern int TELEPORT6ONPIC;
extern int TELEPORT7ONPIC;
extern int TELEPORT8ONPIC;
extern int TELEPORT9ONPIC;
extern int TELEPORT10ONPIC;
extern int TELEPORT_TEXT_BG;
extern int BACKGROUND_SCREENPIC;
extern int APOGEEPIC;
extern int PIRACYPIC;
extern int PC13PIC;
extern int LOSEPIC;
extern int AUTOMAPPIC;
extern int H_ALTPIC;
extern int H_CTRLPIC;
extern int H_SPACEPIC;
extern int H_PAUSEPIC;
extern int H_ESCPIC;
extern int H_LTARROWPIC;
extern int H_UPARROWPIC;
extern int H_DNARROWPIC;
extern int H_RTARROWPIC;
extern int H_ENTERPIC;
extern int H_QPIC;
extern int H_WPIC;
extern int H_EPIC;
extern int H_IPIC;
extern int H_HPIC;
extern int H_1PIC;
extern int H_2PIC;
extern int H_3PIC;
extern int H_4PIC;
extern int H_5PIC;
extern int H_F1PIC;
extern int H_F2PIC;
extern int H_F3PIC;
extern int H_F4PIC;
extern int H_F5PIC;
extern int H_F6PIC;
extern int H_F7PIC;
extern int H_F8PIC;
extern int H_F9PIC;
extern int H_F10PIC;
extern int H_TABPIC;
extern int H_CPIC;
extern int H_FPIC;
extern int H_PPIC;
extern int H_MPIC;
extern int H_LPIC;
extern int H_SHIFTPIC;
extern int APOGEE_LOGOPIC;
extern int VISAPIC;
extern int MCPIC;
extern int FAXPIC;
extern int H_TOPWINDOWPIC;
extern int H_LEFTWINDOWPIC;
extern int H_RIGHTWINDOWPIC;
extern int H_BOTTOMINFOPIC;
extern int C_NOTSELECTEDPIC;
extern int C_SELECTEDPIC;
extern int C_NOTSELECTED_HIPIC;
extern int C_SELECTED_HIPIC;
extern int C_BABYMODEPIC;
extern int C_EASYPIC;
extern int C_NORMALPIC;
extern int C_HARDPIC;
extern int C_EPISODE1PIC;
extern int C_EPISODE2PIC;
extern int C_EPISODE3PIC;
extern int C_EPISODE4PIC;
extern int C_EPISODE5PIC;
extern int C_EPISODE6PIC;
extern int BIGGOLDSTERNPIC;
extern int STARLOGOPIC;
extern int BLAKEWITHGUNPIC;
extern int STARINSTITUTEPIC;
extern int MEDALOFHONORPIC;
extern int SMALLGOLDSTERNPIC;
extern int BLAKEWINPIC;
extern int SHUTTLEEXPPIC;
extern int PLANETSPIC;
extern int MOUSEPIC;
extern int JOYSTICKPIC;
extern int GRAVISPADPIC;
extern int TITLEPIC;
extern int WEAPON1PIC;
extern int WEAPON2PIC;
extern int WEAPON3PIC;
extern int WEAPON4PIC;
extern int WEAPON5PIC;
extern int WAITPIC;
extern int READYPIC;
extern int N_BLANKPIC;
extern int N_0PIC;
extern int N_1PIC;
extern int N_2PIC;
extern int N_3PIC;
extern int N_4PIC;
extern int N_5PIC;
extern int N_6PIC;
extern int N_7PIC;
extern int N_8PIC;
extern int N_9PIC;
extern int N_RPIC;
extern int N_OPIC;
extern int N_LPIC;
extern int N_DASHPIC;
extern int DIM_LIGHTPIC;
extern int BRI_LIGHTPIC;
extern int ECG_HEART_GOOD;
extern int ECG_HEART_BAD;
extern int ECG_GRID_PIECE;
extern int ECG_HEARTBEAT_00;
extern int ECG_HEARTBEAT_01;
extern int ECG_HEARTBEAT_02;
extern int ECG_HEARTBEAT_03;
extern int ECG_HEARTBEAT_04;
extern int ECG_HEARTBEAT_05;
extern int ECG_HEARTBEAT_06;
extern int ECG_HEARTBEAT_07;
extern int ECG_HEARTBEAT_08;
extern int ECG_HEARTBEAT_09;
extern int ECG_HEARTBEAT_10;
extern int ECG_HEARTBEAT_11;
extern int ECG_HEARTBEAT_12;
extern int ECG_HEARTBEAT_13;
extern int ECG_HEARTBEAT_14;
extern int ECG_HEARTBEAT_15;
extern int ECG_HEARTBEAT_16;
extern int ECG_HEARTBEAT_17;
extern int ECG_HEARTBEAT_18;
extern int ECG_HEARTBEAT_19;
extern int ECG_HEARTBEAT_20;
extern int ECG_HEARTBEAT_21;
extern int ECG_HEARTBEAT_22;
extern int ECG_HEARTBEAT_23;
extern int ECG_HEARTBEAT_24;
extern int ECG_HEARTBEAT_25;
extern int ECG_HEARTBEAT_26;
extern int ECG_HEARTBEAT_27;
extern int INFOAREAPIC;
extern int TOP_STATUSBARPIC;
extern int STATUSBARPIC;
extern int PIRACYPALETTE;
extern int APOGEEPALETTE;
extern int TITLEPALETTE;
extern int ORDERSCREEN;
extern int ERRORSCREEN;
extern int INFORMANT_HINTS;
extern int NICE_SCIE_HINTS;
extern int MEAN_SCIE_HINTS;
extern int BRIEF_W1;
extern int BRIEF_I1;
extern int BRIEF_W2;
extern int BRIEF_I2;
extern int BRIEF_W3;
extern int BRIEF_I3;
extern int BRIEF_W4;
extern int BRIEF_I4;
extern int BRIEF_W5;
extern int BRIEF_I5;
extern int BRIEF_W6;
extern int BRIEF_I6;
extern int LEVEL_DESCS;
extern int POWERBALLTEXT;
extern int TICSTEXT;
extern int MUSICTEXT;
extern int RADARTEXT;
extern int HELPTEXT;
extern int SAGATEXT;
extern int LOSETEXT;
extern int ORDERTEXT;
extern int CREDITSTEXT;
extern int MUSTBE386TEXT;
extern int QUICK_INFO1_TEXT;
extern int QUICK_INFO2_TEXT;
extern int BADINFO_TEXT;
extern int CALJOY1_TEXT;
extern int CALJOY2_TEXT;
extern int READTHIS_TEXT;
extern int ELEVMSG0_TEXT;
extern int ELEVMSG1_TEXT;
extern int ELEVMSG4_TEXT;
extern int ELEVMSG5_TEXT;
extern int FLOORMSG_TEXT;
extern int YOUWIN_TEXT;
extern int CHANGEVIEW_TEXT;
extern int DIZ_ERR_TEXT;
extern int BADLEVELSTEXT;
extern int BADSAVEGAME_TEXT;

extern int TELEPORTBACKTOPPIC;
extern int TELEPORTBACKBOTPIC;
extern int TELEPORT11ONPIC;
extern int TELEPORT12ONPIC;
extern int TELEPORT13ONPIC;
extern int TELEPORT14ONPIC;
extern int TELEPORT15ONPIC;
extern int TELEPORT16ONPIC;
extern int TELEPORT17ONPIC;
extern int TELEPORT18ONPIC;
extern int TELEPORT19ONPIC;
extern int TELEPORT20ONPIC;
extern int TELEUPONPIC;
extern int TELEDNONPIC;
extern int TELEUPOFFPIC;
extern int TELEDNOFFPIC;
extern int TELEPORT11OFFPIC;
extern int TELEPORT12OFFPIC;
extern int TELEPORT13OFFPIC;
extern int TELEPORT14OFFPIC;
extern int TELEPORT15OFFPIC;
extern int TELEPORT16OFFPIC;
extern int TELEPORT17OFFPIC;
extern int TELEPORT18OFFPIC;
extern int TELEPORT19OFFPIC;
extern int TELEPORT20OFFPIC;
extern int AUTOMAP_MAG1PIC;
extern int AUTOMAP_MAG2PIC;
extern int AUTOMAP_MAG4PIC;
extern int H_6PIC;
extern int H_TILDEPIC;
extern int H_PLUSPIC;
extern int H_MINUSPIC;
extern int STARPORTPIC;
extern int BOSSPIC;
extern int THREEPLANETSPIC;
extern int SOLARSYSTEMPIC;
extern int AOGENDINGPIC;
extern int GFLOGOSPIC;
extern int BLAKEHEADPIC;
extern int PROJECTFOLDERPIC;
extern int TITLE1PIC;
extern int TITLE2PIC;
extern int WEAPON6PIC;
extern int WEAPON7PIC;
extern int W1_CORNERPIC;
extern int W2_CORNERPIC;
extern int W3_CORNERPIC;
extern int W4_CORNERPIC;
extern int W5_CORNERPIC;
extern int W6_CORNERPIC;
extern int NG_BLANKPIC;
extern int NG_0PIC;
extern int NG_1PIC;
extern int NG_2PIC;
extern int NG_3PIC;
extern int NG_4PIC;
extern int NG_5PIC;
extern int NG_6PIC;
extern int NG_7PIC;
extern int NG_8PIC;
extern int NG_9PIC;
extern int ONEXZOOMPIC;
extern int TWOXZOOMPIC;
extern int FOURXZOOMPIC;
extern int NO_KEYPIC;
extern int RED_KEYPIC;
extern int YEL_KEYPIC;
extern int BLU_KEYPIC;
extern int ENDINGPALETTE;
extern int NO386SCREEN;
extern int T_DEMO0;
extern int T_DEMO1;
extern int T_DEMO2;
extern int T_DEMO3;
extern int T_DEMO4;
extern int T_DEMO5;
extern int DECOY;
extern int DECOY2;
extern int DECOY3;
extern int DECOY4;


#endif // GFXV_INCLUDED
