//////////////////////////////////////
//
// Graphics .H file for .FIR
// IGRAB-ed on Tue Aug 23 15:49:54 1994
//
//////////////////////////////////////

typedef enum {
		TELEPORTBACKTOPPIC=6,
		TELEPORTBACKBOTPIC,                  // 7
		// Lump Start
		TELEPORT1ONPIC,                      // 8
		TELEPORT2ONPIC,                      // 9
		TELEPORT3ONPIC,                      // 10
		TELEPORT4ONPIC,                      // 11
		TELEPORT5ONPIC,                      // 12
		TELEPORT6ONPIC,                      // 13
		TELEPORT7ONPIC,                      // 14
		TELEPORT8ONPIC,                      // 15
		TELEPORT9ONPIC,                      // 16
		TELEPORT10ONPIC,                     // 17
		TELEPORT11ONPIC,                     // 18
		TELEPORT12ONPIC,                     // 19
		TELEPORT13ONPIC,                     // 20
		TELEPORT14ONPIC,                     // 21
		TELEPORT15ONPIC,                     // 22
		TELEPORT16ONPIC,                     // 23
		TELEPORT17ONPIC,                     // 24
		TELEPORT18ONPIC,                     // 25
		TELEPORT19ONPIC,                     // 26
		TELEPORT20ONPIC,                     // 27
		TELEUPONPIC,                         // 28
		TELEDNONPIC,                         // 29
		TELEUPOFFPIC,                        // 30
		TELEDNOFFPIC,                        // 31
		TELEPORT1OFFPIC,                     // 32
		TELEPORT2OFFPIC,                     // 33
		TELEPORT3OFFPIC,                     // 34
		TELEPORT4OFFPIC,                     // 35
		TELEPORT5OFFPIC,                     // 36
		TELEPORT6OFFPIC,                     // 37
		TELEPORT7OFFPIC,                     // 38
		TELEPORT8OFFPIC,                     // 39
		TELEPORT9OFFPIC,                     // 40
		TELEPORT10OFFPIC,                    // 41
		TELEPORT11OFFPIC,                    // 42
		TELEPORT12OFFPIC,                    // 43
		TELEPORT13OFFPIC,                    // 44
		TELEPORT14OFFPIC,                    // 45
		TELEPORT15OFFPIC,                    // 46
		TELEPORT16OFFPIC,                    // 47
		TELEPORT17OFFPIC,                    // 48
		TELEPORT18OFFPIC,                    // 49
		TELEPORT19OFFPIC,                    // 50
		TELEPORT20OFFPIC,                    // 51
		BACKGROUND_SCREENPIC,                // 52
		APOGEEPIC,                           // 53
		PC13PIC,                             // 54
		LOSEPIC,                             // 55
		AUTOMAPPIC,                          // 56
		AUTOMAP_MAG1PIC,                     // 57
		AUTOMAP_MAG2PIC,                     // 58
		AUTOMAP_MAG4PIC,                     // 59
		// Lump Start
		H_ALTPIC,                            // 60
		H_CTRLPIC,                           // 61
		H_SPACEPIC,                          // 62
		H_PAUSEPIC,                          // 63
		H_ESCPIC,                            // 64
		H_LTARROWPIC,                        // 65
		H_UPARROWPIC,                        // 66
		H_DNARROWPIC,                        // 67
		H_RTARROWPIC,                        // 68
		H_ENTERPIC,                          // 69
		H_QPIC,                              // 70
		H_WPIC,                              // 71
		H_EPIC,                              // 72
		H_IPIC,                              // 73
		H_HPIC,                              // 74
		H_1PIC,                              // 75
		H_2PIC,                              // 76
		H_3PIC,                              // 77
		H_4PIC,                              // 78
		H_5PIC,                              // 79
		H_F1PIC,                             // 80
		H_F2PIC,                             // 81
		H_F3PIC,                             // 82
		H_F4PIC,                             // 83
		H_F5PIC,                             // 84
		H_F6PIC,                             // 85
		H_F7PIC,                             // 86
		H_F8PIC,                             // 87
		H_F9PIC,                             // 88
		H_F10PIC,                            // 89
		H_TABPIC,                            // 90
		H_CPIC,                              // 91
		H_FPIC,                              // 92
		H_PPIC,                              // 93
		H_MPIC,                              // 94
		H_LPIC,                              // 95
		H_SHIFTPIC,                          // 96
		H_6PIC,                              // 97
		H_TILDEPIC,                          // 98
		H_PLUSPIC,                           // 99
		H_MINUSPIC,                          // 100
		APOGEE_LOGOPIC,                      // 101
		VISAPIC,                             // 102
		MCPIC,                               // 103
		FAXPIC,                              // 104
		H_TOPWINDOWPIC,                      // 105
		H_LEFTWINDOWPIC,                     // 106
		H_RIGHTWINDOWPIC,                    // 107
		H_BOTTOMINFOPIC,                     // 108
		// Lump Start
		C_NOTSELECTEDPIC,                    // 109
		C_SELECTEDPIC,                       // 110
		C_NOTSELECTED_HIPIC,                 // 111
		C_SELECTED_HIPIC,                    // 112
		C_BABYMODEPIC,                       // 113
		C_EASYPIC,                           // 114
		C_NORMALPIC,                         // 115
		C_HARDPIC,                           // 116
		C_EPISODE1PIC,                       // 117
		C_EPISODE2PIC,                       // 118
		C_EPISODE3PIC,                       // 119
		C_EPISODE4PIC,                       // 120
		C_EPISODE5PIC,                       // 121
		C_EPISODE6PIC,                       // 122
		BIGGOLDSTERNPIC,                     // 123
		STARLOGOPIC,                         // 124
		BLAKEWITHGUNPIC,                     // 125
		STARINSTITUTEPIC,                    // 126
		MEDALOFHONORPIC,                     // 127
		SMALLGOLDSTERNPIC,                   // 128
		BLAKEWINPIC,                         // 129
		SHUTTLEEXPPIC,                       // 130
		PLANETSPIC,                          // 131
		MOUSEPIC,                            // 132
		JOYSTICKPIC,                         // 133
		GRAVISPADPIC,                        // 134
		STARPORTPIC,                         // 135
		BOSSPIC,                             // 136
		THREEPLANETSPIC,                     // 137
		SOLARSYSTEMPIC,                      // 138
		AOGENDINGPIC,                        // 139
		GFLOGOSPIC,                          // 140
		BLAKEHEADPIC,                        // 141
		PROJECTFOLDERPIC,                    // 142
		TITLE1PIC,                           // 143
		TITLE2PIC,                           // 144
		// Lump Start
		WEAPON1PIC,                          // 145
		WEAPON2PIC,                          // 146
		WEAPON3PIC,                          // 147
		WEAPON4PIC,                          // 148
		WEAPON5PIC,                          // 149
		WEAPON6PIC,                          // 150
		WEAPON7PIC,                          // 151
		W1_CORNERPIC,                        // 152
		W2_CORNERPIC,                        // 153
		W3_CORNERPIC,                        // 154
		W4_CORNERPIC,                        // 155
		W5_CORNERPIC,                        // 156
		W6_CORNERPIC,                        // 157
		WAITPIC,                             // 158
		READYPIC,                            // 159
		N_BLANKPIC,                          // 160
		N_0PIC,                              // 161
		N_1PIC,                              // 162
		N_2PIC,                              // 163
		N_3PIC,                              // 164
		N_4PIC,                              // 165
		N_5PIC,                              // 166
		N_6PIC,                              // 167
		N_7PIC,                              // 168
		N_8PIC,                              // 169
		N_9PIC,                              // 170
		N_RPIC,                              // 171
		N_OPIC,                              // 172
		N_LPIC,                              // 173
		N_DASHPIC,                           // 174
		NG_BLANKPIC,                         // 175
		NG_0PIC,                             // 176
		NG_1PIC,                             // 177
		NG_2PIC,                             // 178
		NG_3PIC,                             // 179
		NG_4PIC,                             // 180
		NG_5PIC,                             // 181
		NG_6PIC,                             // 182
		NG_7PIC,                             // 183
		NG_8PIC,                             // 184
		NG_9PIC,                             // 185
		DIM_LIGHTPIC,                        // 186
		BRI_LIGHTPIC,                        // 187
		INFOAREAPIC,                         // 188
		TOP_STATUSBARPIC,                    // 189
		STATUSBARPIC,                        // 190
		ONEXZOOMPIC,                         // 191
		TWOXZOOMPIC,                         // 192
		FOURXZOOMPIC,                        // 193
		NO_KEYPIC,                           // 194
		RED_KEYPIC,                          // 195
		YEL_KEYPIC,                          // 196
		BLU_KEYPIC,                          // 197



		POWERBALLTEXT=199,
		PIRACYPALETTE,                       // 200
		APOGEEPALETTE,                       // 201
		ENDINGPALETTE,                       // 202
		TITLEPALETTE,                        // 203
		ORDERSCREEN,                         // 204
		ERRORSCREEN,                         // 205
		NO386SCREEN,                         // 206
		MUSICTEXT,                           // 207
		RADARTEXT,                           // 208
		TICSTEXT,                            // 209
		T_DEMO0,                             // 210
		T_DEMO1,                             // 211
		T_DEMO2,                             // 212
		T_DEMO3,                             // 213
		T_DEMO4,                             // 214
		T_DEMO5,                             // 215
		INFORMANT_HINTS,                     // 216
		NICE_SCIE_HINTS,                     // 217
		MEAN_SCIE_HINTS,                     // 218
		BRIEF_W1,                            // 219
		BRIEF_I1,                            // 220
		LEVEL_DESCS,                         // 221
		DECOY,                               // 222
		DECOY2,                              // 223
		DECOY3,                              // 224
		DECOY4,                              // 225
		HELPTEXT,                            // 226
		SAGATEXT,                            // 227
		LOSETEXT,                            // 228
		ORDERTEXT,                           // 229
		CREDITSTEXT,                         // 230
		MUSTBE386TEXT,                       // 231
		QUICK_INFO1_TEXT,                    // 232
		QUICK_INFO2_TEXT,                    // 233
		BADINFO_TEXT,                        // 234
		CALJOY1_TEXT,                        // 235
		CALJOY2_TEXT,                        // 236
		READTHIS_TEXT,                       // 237
		ELEVMSG0_TEXT,                       // 238
		ELEVMSG1_TEXT,                       // 239
		ELEVMSG4_TEXT,                       // 240
		ELEVMSG5_TEXT,                       // 241
		FLOORMSG_TEXT,                       // 242
		YOUWIN_TEXT,                         // 243
		CHANGEVIEW_TEXT,                     // 244
		DIZ_ERR_TEXT,                        // 245
		BADLEVELSTEXT,                       // 246
		BADSAVEGAME_TEXT,                    // 247
		ENUMEND
	     } graphicnums;

//
// Data LUMPs
//
#define TELEPORT_LUMP_START		8
#define TELEPORT_LUMP_END		51

#define README_LUMP_START		60
#define README_LUMP_END			108

#define CONTROLS_LUMP_START		109
#define CONTROLS_LUMP_END		122

#define LATCHPICS_LUMP_START		145
#define LATCHPICS_LUMP_END		197


//
// Amount of each data item
//
#define NUMCHUNKS    248
#define NUMFONT      5
#define NUMFONTM     0
#define NUMPICS      192
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   49
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   6
#define STARTPICS    6
#define STARTPICM    198
#define STARTSPRITES 198
#define STARTTILE8   198
#define STARTTILE8M  199
#define STARTTILE16  199
#define STARTTILE16M 199
#define STARTTILE32  199
#define STARTTILE32M 199
#define STARTEXTERNS 199

//
// Thank you for using IGRAB!
//
