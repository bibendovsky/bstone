//////////////////////////////////////
//
// Graphics .H file for .BS1
// IGRAB-ed on Wed Mar 30 18:12:07 1994
//
//////////////////////////////////////

typedef enum {
		TELEPORTBACKPIC=6,
		// Lump Start
		TELEPORT1ONPIC,                      // 7
		TELEPORT2ONPIC,                      // 8
		TELEPORT3ONPIC,                      // 9
		TELEPORT4ONPIC,                      // 10
		TELEPORT5ONPIC,                      // 11
		TELEPORT6ONPIC,                      // 12
		TELEPORT7ONPIC,                      // 13
		TELEPORT8ONPIC,                      // 14
		TELEPORT9ONPIC,                      // 15
		TELEPORT10ONPIC,                     // 16
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
		TELEPORT1OFFPIC,                     // 31
		TELEPORT2OFFPIC,                     // 32
		TELEPORT3OFFPIC,                     // 33
		TELEPORT4OFFPIC,                     // 34
		TELEPORT5OFFPIC,                     // 35
		TELEPORT6OFFPIC,                     // 36
		TELEPORT7OFFPIC,                     // 37
		TELEPORT8OFFPIC,                     // 38
		TELEPORT9OFFPIC,                     // 39
		TELEPORT10OFFPIC,                    // 40
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
		BACKGROUND_SCREENPIC,                // 51
		APOGEEPIC,                           // 52
		PIRACYPIC,                           // 53
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
		APOGEE_LOGOPIC,                      // 99
		VISAPIC,                             // 100
		MCPIC,                               // 101
		FAXPIC,                              // 102
		H_TOPWINDOWPIC,                      // 103
		H_LEFTWINDOWPIC,                     // 104
		H_RIGHTWINDOWPIC,                    // 105
		H_BOTTOMINFOPIC,                     // 106
		// Lump Start
		C_NOTSELECTEDPIC,                    // 107
		C_SELECTEDPIC,                       // 108
		C_NOTSELECTED_HIPIC,                 // 109
		C_SELECTED_HIPIC,                    // 110
		C_BABYMODEPIC,                       // 111
		C_EASYPIC,                           // 112
		C_NORMALPIC,                         // 113
		C_HARDPIC,                           // 114
		C_EPISODE1PIC,                       // 115
		C_EPISODE2PIC,                       // 116
		C_EPISODE3PIC,                       // 117
		C_EPISODE4PIC,                       // 118
		C_EPISODE5PIC,                       // 119
		C_EPISODE6PIC,                       // 120
		BIGGOLDSTERNPIC,                     // 121
		STARLOGOPIC,                         // 122
		BLAKEWITHGUNPIC,                     // 123
		STARINSTITUTEPIC,                    // 124
		MEDALOFHONORPIC,                     // 125
		SMALLGOLDSTERNPIC,                   // 126
		BLAKEWINPIC,                         // 127
		SHUTTLEEXPPIC,                       // 128
		PLANETSPIC,                          // 129
		MOUSEPIC,                            // 130
		JOYSTICKPIC,                         // 131
		GRAVISPADPIC,                        // 132
		STARPORTPIC,                         // 133
		BOSSPIC,                             // 134
		THREEPLANETSPIC,                     // 135
		SOLARSYSTEMPIC,                      // 136
		AOGENDINGPIC,                        // 137
		GFLOGOSPIC,                          // 138
		BLAKEHEADPIC,                        // 139
		PROJECTFOLDERPIC,                    // 140
		TITLEPIC,                            // 141
		// Lump Start
		WEAPON1PIC,                          // 142
		WEAPON2PIC,                          // 143
		WEAPON3PIC,                          // 144
		WEAPON4PIC,                          // 145
		WEAPON5PIC,                          // 146
		WEAPON6PIC,                          // 147
		WEAPON7PIC,                          // 148
		W1_CORNERPIC,                        // 149
		W2_CORNERPIC,                        // 150
		W3_CORNERPIC,                        // 151
		W4_CORNERPIC,                        // 152
		W5_CORNERPIC,                        // 153
		W6_CORNERPIC,                        // 154
		WAITPIC,                             // 155
		READYPIC,                            // 156
		N_BLANKPIC,                          // 157
		N_0PIC,                              // 158
		N_1PIC,                              // 159
		N_2PIC,                              // 160
		N_3PIC,                              // 161
		N_4PIC,                              // 162
		N_5PIC,                              // 163
		N_6PIC,                              // 164
		N_7PIC,                              // 165
		N_8PIC,                              // 166
		N_9PIC,                              // 167
		N_RPIC,                              // 168
		N_OPIC,                              // 169
		N_LPIC,                              // 170
		N_DASHPIC,                           // 171
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
		DIM_LIGHTPIC,                        // 183
		BRI_LIGHTPIC,                        // 184
		INFOAREAPIC,                         // 185
		TOP_STATUSBARPIC,                    // 186
		STATUSBARPIC,                        // 187
		ONEXZOOMPIC,                         // 188
		TWOXZOOMPIC,                         // 189
		FOURXZOOMPIC,                        // 190
		NO_KEYPIC,                           // 191
		RED_KEYPIC,                          // 192
		YEL_KEYPIC,                          // 193
		BLU_KEYPIC,                          // 194



		POWERBALLTEXT=196,
		PIRACYPALETTE,                       // 197
		APOGEEPALETTE,                       // 198
		ENDINGPALETTE,                       // 199
		TITLEPALETTE,                        // 200
		ORDERSCREEN,                         // 201
		ERRORSCREEN,                         // 202
		NO386SCREEN,                         // 203
		MUSICTEXT,                           // 204
		RADARTEXT,                           // 205
		TICSTEXT,                            // 206
		T_DEMO0,                             // 207
		T_DEMO1,                             // 208
		T_DEMO2,                             // 209
		T_DEMO3,                             // 210
		T_DEMO4,                             // 211
		T_DEMO5,                             // 212
		INFORMANT_HINTS,                     // 213
		NICE_SCIE_HINTS,                     // 214
		MEAN_SCIE_HINTS,                     // 215
		BRIEF_W1,                            // 216
		BRIEF_I1,                            // 217
		BRIEF_W2,                            // 218
		BRIEF_I2,                            // 219
		BRIEF_W3,                            // 220
		BRIEF_I3,                            // 221
		BRIEF_W4,                            // 222
		BRIEF_I4,                            // 223
		BRIEF_W5,                            // 224
		BRIEF_I5,                            // 225
		BRIEF_W6,                            // 226
		BRIEF_I6,                            // 227
		LEVEL_DESCS,                         // 228
		DECOY,                               // 229
		DECOY2,                              // 230
		DECOY3,                              // 231
		DECOY4,                              // 232
		HELPTEXT,                            // 233
		SAGATEXT,                            // 234
		LOSETEXT,                            // 235
		ORDERTEXT,                           // 236
		CREDITSTEXT,                         // 237
		MUSTBE386TEXT,                       // 238
		QUICK_INFO1_TEXT,                    // 239
		QUICK_INFO2_TEXT,                    // 240
		BADINFO_TEXT,                        // 241
		CALJOY1_TEXT,                        // 242
		CALJOY2_TEXT,                        // 243
		READTHIS_TEXT,                       // 244
		ELEVMSG0_TEXT,                       // 245
		ELEVMSG1_TEXT,                       // 246
		ELEVMSG4_TEXT,                       // 247
		ELEVMSG5_TEXT,                       // 248
		FLOORMSG_TEXT,                       // 249
		YOUWIN_TEXT,                         // 250
		CHANGEVIEW_TEXT,                     // 251
		DIZ_ERR_TEXT,                        // 252
		BADLEVELSTEXT,                       // 253
		BADSAVEGAME_TEXT,                    // 254
		ENUMEND
	     } graphicnums;

//
// Data LUMPs
//
#define TELEPORT_LUMP_START		7
#define TELEPORT_LUMP_END		50

#define README_LUMP_START		60
#define README_LUMP_END			106

#define CONTROLS_LUMP_START		107
#define CONTROLS_LUMP_END		120

#define LATCHPICS_LUMP_START		142
#define LATCHPICS_LUMP_END		194


//
// Amount of each data item
//
#define NUMCHUNKS    255
#define NUMFONT      5
#define NUMFONTM     0
#define NUMPICS      189
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   59
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   6
#define STARTPICS    6
#define STARTPICM    195
#define STARTSPRITES 195
#define STARTTILE8   195
#define STARTTILE8M  196
#define STARTTILE16  196
#define STARTTILE16M 196
#define STARTTILE32  196
#define STARTTILE32M 196
#define STARTEXTERNS 196

//
// Thank you for using IGRAB!
//
