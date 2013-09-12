//--------------------------------------------------------------------------
//
//                             TEXT PRESENTER
//                    Programmed by Michael D. Maynard
//                  Copyright 1993, JAM Productions, Inc.	
//                          All rights reserved.
//
// Active control codes:
//
//  ^ANnn			- define animation
//  ^AXnn			- alter X coordinate (nn = signed char)
//  ^AYnn			- alter Y coordinate (nn = signed char)
//  ^BCnn 			- set background color
//  ^BE           - ring bell
//  ^BXn          - draw box around shape (0 = no box, 1 = box)
//  ^CE				- center text between 'left margin' and 'right margin'
//  ^DCnn			- dark color (dark-light for boxes)
//  ^DM  			- use default margins
//  ^DSnn			- Display String Number 'nn'
//  ^EP				- end of page (waits for up/down arrow)
//  ^FCnn			- set font color
//  ^FNn          - set font number
//  ^HI				- hide cursor
//  ^HCnn			- highlight color
//  ^HF				- highlight color off
//  ^HO				- highlight color on
//  ^LCnn			- light color (highlight for boxes)
//  ^LJ				- left justify  --\ ^RJ doesn't handle imbedded control
//  ^LMnnn			- set left margin (if 'nnn' == "fff" uses current x)
//  ^MO           - wait for action from any input device
//  ^PA				- pause 1/2 second
//  ^PMnn			- Play Music Number 'nn'
//  ^PSnn			- Play Sound Number 'nn'
//  ^PXnnn			- move x to coordinate 'n'
//  ^PYnnn			- move y to coordinate 'n'
//  ^RMnnn			- set right margin (if 'nnn' == "fff" uses current x)
//  ^RJ				- right justify --/ codes properly. Use with caution.
//  ^RL           - restore cursor location.
//  ^RXn				- restore x location
//  ^RYn          - restore y location
//  ^SBn				- clear scaled background (0 = no clear, 1 = clear)
//  ^SCnn			- shadow color (shadow for text / pics)
//  ^SHnnn			- display shape 'n' at current x,y
//  ^SPn				- shadow pics (0 = no shadow, 1 = shadow)
//  ^STn          - shadow text (0 = no shadow, 1 = shadow)
//  ^SL           - save cursor location.
//  ^SXn				- save x location
//  ^SYn          - save y location
//  ^XX				- exit presenter
//  ^ZZ				- NOP ... This code is ignored.
//
// Other info:
//
// All 'n' values are hex numbers (0 - f), case insensitive.
// The number of N's listed is the number of digits REQUIRED by that control
// code. (IE: ^LMnnn MUST have 3 values! --> 003, 1a2, 01f, etc...)
//
// If a line consists only of control codes, the cursor is NOT advanced
// to the next line (the ending <CR><LF> is skipped). If just ONE non-control
// code is added, the number "8" for example, then the "8" is displayed
// and the cursor is advanced to the next line.
//
// ^CE must be on the same line as the text it should center!
//
// The text presenter now handles sprites, but they are NOT masked! Also,
// sprite animations will be difficult to implement unless all frames are
// of the same dimensions.
//
// A shape of type "pis_latchpic" is assumed to already be loaded in latch
// memory. No caching is performed on this type of shape! If a pic needs to
// be cached, specify "pis_pic".
//
// You can comment-out a line by beginning the line with a semi-colon.
// Commented lines are ignored by the parser. Do not comment-out the last
// line of the file for this may cause harm to all things great and small!
//
//--------------------------------------------------------------------------

#include "3d_def.h"

#include "jm_error.h"
#include "jm_io.h"

#ifdef MSVC
#pragma hdrstop
#endif



void VWL_MeasureString (const char* string, Uint16* width, Uint16* height, fontstruct* font);
void VH_UpdateScreen();
void ClearMemory (void);


//#define DRAW_TO_FRONT

// string array table is a quick, easy and expandable way to print
// any number of strings in a text file using the ^DS codes...
//
// See Macro TP_INIT_DISPLAY_STR(num,str_ptr)						- JM_TP.h
// To init strings and handle range checking....

char * piStringTable[PI_MAX_NUM_DISP_STRS];


// shape table provides a way for the presenter to access and
// display any shape.
//

piShapeInfo piShapeTable[] = {

		{SPR_GREEN_OOZE1,pis_scaled},				// 0 - Green Ooze

		{SPR_STAT_32,pis_scaled},					// 1 - Red Key
		{SPR_STAT_33,pis_scaled},					// 2 - Yellow Key
		{SPR_STAT_34,pis_scaled},					// 3 - Green Key
		{SPR_STAT_35,pis_scaled},					// 4 - Blue Key
		{SPR_STAT_36,pis_scaled},					// 5 - Gold Key
		{SPR_STAT_26,pis_scaled},					// 6 - Charge Unit
		{SPR_STAT_24,pis_scaled},					// 7 - PISTOL
		{SPR_STAT_27,pis_scaled},					// 8 - Auto-Burst Rifle
		{SPR_STAT_28,pis_scaled},					// 9 - Particle Charged ION
		{SPR_STAT_29,pis_scaled},					// 10 - First Aid
		{SPR_STAT_30,pis_scaled},					// 11 - Full Heal
		{SPR_STAT_48,pis_scaled},					// 12 - money bag
		{SPR_STAT_49,pis_scaled},					// 13 - loot
		{SPR_STAT_50,pis_scaled},					// 14 - gold
		{SPR_STAT_51,pis_scaled},					// 15 - bonus
		{SPR_STAT_42,pis_scaled},					// 16 - Chicken Leg
		{SPR_STAT_44,pis_scaled},					// 17 - Ham
		{SPR_STAT_40,pis_scaled},					// 18 - Full Water Bowl
		{SPR_STAT_0,pis_scaled},					// 19 - Water Puddle

		// ATTACKING ACTORS

		{SPR_RENT_W1_8,pis_scaled},				// 20 - rentacopobj, Walking
		{SPR_RENT_W2_8,pis_scaled},				// 21 - rentacopobj, Walking
		{SPR_RENT_W3_8,pis_scaled},				// 22 - rentacopobj, Walking
		{SPR_RENT_W4_8,pis_scaled},				// 23 - rentacopobj, Walking

		{SPR_TERROT_1,pis_scaled},					// 24 - hang_terrotobj, Rotating
		{SPR_TERROT_2,pis_scaled},					// 25 - hang_terrotobj, Rotating
		{SPR_TERROT_3,pis_scaled},					// 26 - hang_terrotobj, Rotating
		{SPR_TERROT_4,pis_scaled},					// 27 - hang_terrotobj, Rotating
		{SPR_TERROT_5,pis_scaled},					// 28 - hang_terrotobj, Rotating
		{SPR_TERROT_6,pis_scaled},					// 29 - hang_terrotobj, Rotating
		{SPR_TERROT_7,pis_scaled},					// 30 - hang_terrotobj, Rotating
		{SPR_TERROT_8,pis_scaled},					// 31 - hang_terrotobj, Rotating

		{SPR_STAT_77,pis_scaled},    				// 32 - 1 Credit Food Unit Token
		{SPR_STAT_78,pis_scaled},    				// 33 - 5 Credit Food Unit Token
		{SPR_STAT_79,pis_scaled},  				// 34 - Auto-Charge Pistol

		{14*2,pis_scwall},    						// 35 - Food Unit Wall
		{16*2,pis_scwall},    						// 36 - Terminal Wall
		{20*2+1,pis_scwall},    					// 37 - Teleport w/Pannel on it
		{44*2,pis_scwall},    						// 38 - Barrier Switch Wall
		{31*2,pis_scwall},    						// 39 - Warp Tile

		{BIGGOLDSTERNPIC,pis_pic},    			// 40 - Big Goldstern
		{STARLOGOPIC,pis_pic},    					// 41 - Star Logo
		{BLAKEWITHGUNPIC,pis_pic},    			// 42 - Blake With Gun
		{STARINSTITUTEPIC,pis_pic},    			// 43 - Star Institute
		{MEDALOFHONORPIC,pis_pic},    			// 44 - Medal Of Honor
		{SMALLGOLDSTERNPIC,pis_pic},    			// 45 - Small Goldstern
		{BLAKEWINPIC,pis_pic},    					// 46 - Blake Win
		{AUTOMAPPIC,pis_pic},  						// 47 - AutoMap Pic

		{SPR_ELEC_POST1,pis_scaled},     	   // 48 - Barrier Post 1
		{SPR_ELEC_POST2,pis_scaled},      		// 49 - Barrier Post 2
		{SPR_ELEC_POST3,pis_scaled},      		// 50 - Barrier Post 3

		{SHUTTLEEXPPIC,pis_pic},   				// 51 - Shuttle Explode
		{PLANETSPIC,pis_pic},    					// 52 - Planets Pic
		{SPR_CUBE1,pis_scaled},						// 53 - Rotating Cube

		{H_PPIC,pis_pic},                      // 54 - M key
		{MOUSEPIC,pis_pic}, 		               // 55 - Mouse Pad			

		{SPR_OFC_W1_8,pis_scaled},      			// 56 - gen_scientist, walking
		{SPR_OFC_W2_8,pis_scaled},      			// 57 - gen_scientist, walking
		{SPR_OFC_W3_8,pis_scaled},      			// 58 - gen_scientist, walking
		{SPR_OFC_W4_8,pis_scaled},      			// 59 - gen_scientist, walking

		{SPR_POD_WALK1,pis_scaled},    			// 60 - podalien - walking
		{SPR_POD_WALK2,pis_scaled},     			// 61 - podalien - walking
		{SPR_POD_WALK3,pis_scaled},     			// 62 - podalien - walking
		{SPR_POD_WALK4,pis_scaled},     			// 63 - podalien - walking

		{SPR_ELEC_WALK1,pis_scaled},     		// 64 - electro alien - walking
		{SPR_ELEC_WALK2,pis_scaled},     		// 65 - electro alien - walking
		{SPR_ELEC_WALK3,pis_scaled},     		// 66 - electro alien - walking
		{SPR_ELEC_WALK4,pis_scaled},     		// 67 - electro alien - walking

		{SPR_ELECTRO_SPHERE_ROAM1,pis_scaled},	// 68 - electrosphere - Roaming
		{SPR_ELECTRO_SPHERE_ROAM2,pis_scaled},	// 69 - electrosphere - Roaming
		{SPR_ELECTRO_SPHERE_ROAM3,pis_scaled},	// 70 - electrosphere - Roaming

		{SPR_PRO_W1_8,pis_scaled},      			// 71 - proguard - Walking
		{SPR_PRO_W2_8,pis_scaled},      			// 72 - proguard - Walking
		{SPR_PRO_W3_8,pis_scaled},      			// 73 - proguard - Walking
		{SPR_PRO_W4_8,pis_scaled},      			// 74 - proguard - Walking

		{SPR_GENETIC_W1,pis_scaled},    			// 75 - genetic_guard - Walking
		{SPR_GENETIC_W2,pis_scaled},    			// 76 - genetic_guard - Walking
		{SPR_GENETIC_W3,pis_scaled},     		// 77 - genetic_guard - Walking
		{SPR_GENETIC_W4,pis_scaled},     		// 78 - genetic_guard - Walking

		{SPR_MUTHUM1_W1,pis_scaled},     		// 79 - mutant_human1 - walking
		{SPR_MUTHUM1_W2,pis_scaled},     		// 80 - mutant_human1 - walking
		{SPR_MUTHUM1_W3,pis_scaled},     		// 81 - mutant_human1 - walking
		{SPR_MUTHUM1_W4,pis_scaled},     		// 82 - mutant_human1 - walking

		{SPR_MUTHUM2_W1,pis_scaled},     		// 83 - mutant_human2 - walking
		{SPR_MUTHUM2_W2,pis_scaled},     		// 84 - mutant_human2 - walking
		{SPR_MUTHUM2_W3,pis_scaled},     		// 85 - mutant_human2 - walking
		{SPR_MUTHUM2_W4,pis_scaled},     		// 86 - mutant_human2 - walking

		{SPR_LCAN_ALIEN_READY,pis_scaled},  	// 87 - Large canister for lg_Aln

		{SPR_LCAN_ALIEN_W1,pis_scaled},  		// 88 - Large Canister Alien
		{SPR_LCAN_ALIEN_W2,pis_scaled},  		// 89 - Large Canister Alien
		{SPR_LCAN_ALIEN_W3,pis_scaled},  		// 90 - Large Canister Alien
		{SPR_LCAN_ALIEN_W4,pis_scaled},      	// 91 - Large Canister Alien

		{SPR_SCAN_ALIEN_READY,pis_scaled},   	// 92 - Small Cansiter

		{SPR_SCAN_ALIEN_W1,pis_scaled},      	// 93 - Sm Can Alien - Walking
		{SPR_SCAN_ALIEN_W2,pis_scaled},      	// 94 - Sm Can Alien - Walking
		{SPR_SCAN_ALIEN_W3,pis_scaled},      	// 95 - Sm Can Alien - Walking
		{SPR_SCAN_ALIEN_W4,pis_scaled},      	// 96 - Sm Can Alien - Walking

		{SPR_GURNEY_MUT_READY,pis_scaled},   	// 97 - Gurney Mutant on gurney

		{SPR_GURNEY_MUT_W1,pis_scaled},      	// 98  - Gurney Mutant - Walking
		{SPR_GURNEY_MUT_W2,pis_scaled},      	// 99  - Gurney Mutant - Walking
		{SPR_GURNEY_MUT_W3,pis_scaled},      	// 100 - Gurney Mutant - Walking
		{SPR_GURNEY_MUT_W4,pis_scaled},      	// 101 - Gurney Mutant - Walking

		{SPR_LIQUID_S1,pis_scaled},      		// 102 - liquid Alien - Shooting
		{SPR_LIQUID_S2,pis_scaled},      		// 103 - liquid Alien - Shooting
		{SPR_LIQUID_S3,pis_scaled},      		// 104 - liquid Alien - Shooting

		{SPR_SWAT_W1_8,pis_scaled},      		// 105 - SWAT guard - Walking
		{SPR_SWAT_W2_8,pis_scaled},      		// 106 - SWAT guard - Walking
		{SPR_SWAT_W3_8,pis_scaled},      		// 107 - SWAT guard - Walking
		{SPR_SWAT_W4_8,pis_scaled},     			// 108 - SWAT guard - Walking

		{SPR_GOLD_W1_8,pis_scaled},      		// 109 - Dr Goldstern - Walking
		{SPR_GOLD_W2_8,pis_scaled},      		// 110 - Dr Goldstern - Walking
		{SPR_GOLD_W3_8,pis_scaled},      		// 111 - Dr Goldstern - Walking
		{SPR_GOLD_W4_8,pis_scaled},      		// 112 - Dr Goldstern - Walking

		{JOYSTICKPIC,pis_pic},                 // 113 - Joystick

		{SPR_GSCOUT_W1_8,pis_scaled},      		// 114 - Volatile Transport - moving
		{SPR_GSCOUT_W2_8,pis_scaled},      		// 115 - Volatile Transport - moving
		{SPR_GSCOUT_W3_8,pis_scaled},      		// 116 - Volatile Transport - moving
		{SPR_GSCOUT_W4_8,pis_scaled},     		// 117 - Volatile Transport - moving

		{SPR_FSCOUT_W1_8,pis_scaled},      		// 118 - Floating Bomb - moving
		{SPR_FSCOUT_W2_8,pis_scaled},      		// 119 - Floating Bomb - moving
		{SPR_FSCOUT_W3_8,pis_scaled}, 		  	// 120 - Floating Bomb - moving
		{SPR_FSCOUT_W4_8,pis_scaled},      		// 121 - Floating Bomb - moving

		{SPR_CUBE1,pis_scaled},      				// 122 - ** OPEN **

		{SPR_ELEC_SHOT1,pis_scaled},    		  	// 123 - Electro Aln Shot - floating
		{SPR_ELEC_SHOT2,pis_scaled},      		// 124 - Electro Aln Shot - floating

		{SPR_SECURITY_NORMAL,pis_scaled},    	// 125 - Security_light - normal
		{SPR_SECURITY_ALERT,pis_scaled},     	// 126 - Security_light - Alerted

		{SPR_LIQUID_SHOT_FLY_1,pis_scaled},  	// 127 - Liquid Aln Sht - float'n
		{SPR_LIQUID_SHOT_FLY_2,pis_scaled},  	// 128 - Liquid Aln Sht - float'n
		{SPR_LIQUID_SHOT_FLY_3,pis_scaled},  	// 129 - Liquid Aln Sht - float'n

		{SPR_POD_EGG,pis_scaled},   			   // 130 - POD Alien Egg

		{SPR_DECO_ARC_1,pis_scaled},     	   // 131 - DecoArc
		{SPR_DECO_ARC_2,pis_scaled},      		// 132 - DecoArc
		{SPR_DECO_ARC_3,pis_scaled},      		// 133 - DecoArc

		{GRAVISPADPIC,pis_pic},                // 134 - Gravis Pad	
		{H_MPIC,pis_pic},                      // 135 - M key

		{SPR_CANDY_BAR,pis_scaled}, 			  	// 136 - Candy bar
		{SPR_SANDWICH,pis_scaled},   		   	// 137 - Sandwich

		{SPR_STAT_46,pis_scaled},					// 138 - Grenade Launcher
		{SPR_EXPLOSION_3,pis_scaled},				// 139 - General Explosion
		{SPR_BLACK_OOZE1,pis_scaled},				// 140 - Black Ooze

		{SPR_ELEC_ARC1,pis_scaled}, 			 	// 141 - Electric Arc Barrier 1
		{SPR_ELEC_ARC2,pis_scaled},  				// 142 - Electric Arc Barrier 2
		{SPR_ELEC_ARC3,pis_scaled},  				// 143 - Electric Arc Barrier 3

		{C_EPISODE1PIC,pis_pic},					// 144 - STAR Institute
		{C_EPISODE2PIC,pis_pic},					// 145 - Floating Fortress
		{C_EPISODE3PIC,pis_pic},               // 146 - Underground Network
		{C_EPISODE4PIC,pis_pic},               // 147 - STAR Port
		{C_EPISODE5PIC,pis_pic},               // 148 - Habitat II
		{C_EPISODE6PIC,pis_pic},               // 149 - Defense Center

		{SPR_BOSS1_W1,pis_scaled},					// 150 - Boss 1 - walking
		{SPR_BOSS1_W2,pis_scaled},					// 151 - Boss 1 - walking
		{SPR_BOSS1_W3,pis_scaled},					// 152 - Boss 1 - walking
		{SPR_BOSS1_W4,pis_scaled},					// 153 - Boss 1 - walking

		{SPR_BOSS2_W1,pis_scaled},					// 154 - Boss 2 - walking
		{SPR_BOSS2_W2,pis_scaled},					// 155 - Boss 2 - walking
		{SPR_BOSS2_W3,pis_scaled},					// 156 - Boss 2 - walking
		{SPR_BOSS2_W4,pis_scaled},					// 157 - Boss 2 - walking

		{SPR_BOSS3_W1,pis_scaled},					// 158 - Boss 3 - walking
		{SPR_BOSS3_W2,pis_scaled},					// 159 - Boss 3 - walking
		{SPR_BOSS3_W3,pis_scaled},					// 160 - Boss 3 - walking
		{SPR_BOSS3_W4,pis_scaled},					// 161 - Boss 3 - walking

		{SPR_BOSS4_W1,pis_scaled},					// 162 - Boss 4 - walking
		{SPR_BOSS4_W2,pis_scaled},					// 163 - Boss 4 - walking
		{SPR_BOSS4_W3,pis_scaled},					// 164 - Boss 4 - walking
		{SPR_BOSS4_W4,pis_scaled},					// 165 - Boss 4 - walking

		{SPR_BOSS5_W1,pis_scaled},					// 166 - Boss 5 - walking
		{SPR_BOSS5_W2,pis_scaled},					// 167 - Boss 5 - walking
		{SPR_BOSS5_W3,pis_scaled},					// 168 - Boss 5 - walking
		{SPR_BOSS5_W4,pis_scaled},					// 169 - Boss 5 - walking

		{SPR_BOSS6_W1,pis_scaled},					// 170 - Boss 6 - walking
		{SPR_BOSS6_W2,pis_scaled},					// 171 - Boss 6 - walking
		{SPR_BOSS6_W3,pis_scaled},					// 172 - Boss 6 - walking
		{SPR_BOSS6_W4,pis_scaled},					// 173 - Boss 6 - walking

		{H_ALTPIC,pis_pic},							// 174 - ALT key
		{H_CTRLPIC,pis_pic},                   // 175 - CTRL key
		{H_SPACEPIC,pis_pic},                  // 176 - SPACE key
		{H_PAUSEPIC,pis_pic},                  // 177 - PAUSE key
		{H_ESCPIC,pis_pic},                    // 178 - ESC key
		{H_LTARROWPIC,pis_pic},                // 179 - LEFT ARROW key
		{H_UPARROWPIC,pis_pic},                // 180 - UP ARROW key
		{H_DNARROWPIC,pis_pic},                // 181 - DOWN ARROW key
		{H_RTARROWPIC,pis_pic},                // 182 - RIGHT ARROW key
		{H_QPIC,pis_pic},                      // 183 - Q key
		{H_WPIC,pis_pic},                      // 184 - W key
		{H_EPIC,pis_pic},                      // 185 - E key
		{H_IPIC,pis_pic},                      // 186 - I key
		{H_HPIC,pis_pic},                      // 187 - H key
		{H_1PIC,pis_pic},                      // 188 - 1 key
		{H_2PIC,pis_pic},                      // 189 - 2 key
		{H_3PIC,pis_pic},                      // 190 - 3 key
		{H_4PIC,pis_pic},                      // 191 - 4 key
		{H_5PIC,pis_pic},                      // 192 - 5 key
		{H_F1PIC,pis_pic},                     // 193 - F1 key
		{H_F2PIC,pis_pic},                     // 194 - F2 key
		{H_F3PIC,pis_pic},                     // 195 - F3 key
		{H_F4PIC,pis_pic},                     // 196 - F4 key
		{H_F5PIC,pis_pic},                     // 197 - F5 key
		{H_F6PIC,pis_pic},                     // 198 - F6 key
		{H_F7PIC,pis_pic},                     // 199 - F7 key
		{H_F8PIC,pis_pic},                     // 200 - F8 key
		{H_F9PIC,pis_pic},                     // 201 - F9 key
		{H_F10PIC,pis_pic},                    // 202 - F10 key
		{H_ENTERPIC,pis_pic},                  // 203 - ENTER key
		{H_TABPIC,pis_pic},                  	// 204 - TAB key
		{H_CPIC,pis_pic},    	              	// 205 - C key
		{H_FPIC,pis_pic},    	              	// 206 - F key
		{SPR_CRATE_1,pis_scaled},             	// 207 - Crate
		{SPR_GSCOUT_W1_1,pis_scaled},				// 208 - VMT
		{SPR_FSCOUT_W1_1,pis_scaled},				// 209 - PerScan
		{APOGEE_LOGOPIC,pis_pic},              // 210 - Apogee Logo
		{VISAPIC,pis_pic},                  	// 211 - Visa Card
		{MCPIC,pis_pic},    	   	           	// 212 - Master Card
		{FAXPIC, pis_pic},    		           	// 213 - Fax Machine
		{H_LPIC, pis_pic},							// 214 - L key
		{H_SHIFTPIC, pis_pic},						// 215 - Shift key
		{SPR_DOORBOMB,pis_scaled},					// 216 - DoorBomb
		{SPR_AUTOMAPPER,pis_scaled},				// 217 - Radar Powerup.
		{STARPORTPIC,pis_pic},						// 218 - Star Port
		{BOSSPIC,pis_pic},          			   // 219 - AOG Boss
		{THREEPLANETSPIC,pis_pic},      		   // 220 - Three Planets
		{SOLARSYSTEMPIC,pis_pic},           	// 221 - Solar System
		{AOGENDINGPIC,pis_pic},     	         // 222 - AOG Ending
		{GFLOGOSPIC,pis_pic},         	      // 223 - Goldfire 'n Logos
		{BLAKEHEADPIC,pis_pic},       	      // 224 - Blake's Big Head
		{PROJECTFOLDERPIC,pis_pic},            // 225 - Project Folder
		{SPR_GREEN2_OOZE1,pis_scaled},			// 226 - Green2 Ooze
		{SPR_BLACK2_OOZE1,pis_scaled},			// 227 - Black2 Ooze
		{SPR_STAT_34,pis_scaled},					// 228 - BFG Cannon AKA Anti-Plasma Cannon
		{H_PLUSPIC,pis_pic},							// 229 - PLUS key
		{SPR_DOORBOMB,pis_scaled},					// 230 - Plasma Detonator
		{SPR_DETONATOR_EXP3,pis_scaled},			// 231 - Plasma Detonator Explosion
		{SPR_MGOLD_WALK1,pis_scaled},      		// 232 - Morphed Dr Goldstern - Walking
		{SPR_MGOLD_WALK2,pis_scaled},      		// 233 - Morphed Dr Goldstern - Walking
		{SPR_MGOLD_WALK3,pis_scaled},      		// 234 - Morphed Dr Goldstern - Walking
		{SPR_MGOLD_WALK4,pis_scaled},      		// 235 - Morphed Dr Goldstern - Walking
		{H_MINUSPIC,pis_pic},						// 236 - MINUS key
		{H_6PIC,pis_pic},								// 237 - 6 KEY
		{H_TILDEPIC,pis_pic},						// 238 - TILDE KEY
		{AUTOMAP_MAG1PIC,pis_pic},					// 239 - AutoMap pic MAG1 (normal)
		{AUTOMAP_MAG2PIC,pis_pic},					// 240 - AutoMap pic MAG2
		{AUTOMAP_MAG4PIC,pis_pic},					// 241 - AutoMap pic MAG4
		{SPR_VPOST1,pis_scaled},					// 242 - VPost Barrier
		{SPR_VPOST2,pis_scaled},					// 243 - VPost Barrier
		{SPR_VPOST3,pis_scaled},					// 244 - VPost Barrier
		{SPR_VPOST4,pis_scaled},					// 245 - VPost Barrier
		{SPR_VPOST5,pis_scaled},					// 246 - VPost Barrier
		{SPR_VPOST6,pis_scaled},					// 247 - VPost Barrier
		{SPR_VPOST7,pis_scaled},					// 248 - VPost Barrier
		{SPR_VPOST8,pis_scaled},					// 249 - VPost Barrier
		{SPR_VSPIKE1,pis_scaled},					// 250 - VSpike Barrier
		{SPR_VSPIKE2,pis_scaled},					// 251 - VSpike Barrier
		{SPR_VSPIKE3,pis_scaled},					// 252 - VSpike Barrier
		{SPR_VSPIKE4,pis_scaled},					// 253 - VSpike Barrier
		{SPR_VSPIKE5,pis_scaled},					// 254 - VSpike Barrier
		{SPR_VSPIKE6,pis_scaled},					// 255 - VSpike Barrier
		{SPR_VSPIKE7,pis_scaled},					// 256 - VSpike Barrier
		{SPR_VSPIKE8,pis_scaled},					// 257 - VSpike Barrier
		{SPR_CUBE1,pis_scaled},      				// 258 - Security Cube - Rotating
		{SPR_CUBE2,pis_scaled},      				// 259 - Security Cube - Rotating
		{SPR_CUBE3,pis_scaled},      				// 260 - Security Cube - Rotating
		{SPR_CUBE4,pis_scaled},      				// 261 - Security Cube - Rotating
		{SPR_CUBE5,pis_scaled},      				// 262 - Security Cube - Rotating
		{SPR_CUBE6,pis_scaled},      				// 263 - Security Cube - Rotating
		{SPR_CUBE7,pis_scaled},      				// 264 - Security Cube - Rotating
		{SPR_CUBE8,pis_scaled},      				// 265 - Security Cube - Rotating
		{SPR_CUBE9,pis_scaled},      				// 266 - Security Cube - Rotating
		{SPR_CUBE10,pis_scaled},     				// 267 - Security Cube - Rotating

		{SPR_BOSS7_W1,pis_scaled},					// 268 - Final Boss 1 Walking
		{SPR_BOSS7_W2,pis_scaled},					// 269 - Final Boss 1 Walking
		{SPR_BOSS7_W3,pis_scaled},					// 270 - Final Boss 1 Walking
		{SPR_BOSS7_W4,pis_scaled},					// 271 - Final Boss 1 Walking

		{SPR_BOSS8_W1,pis_scaled},					// 272 - Final Boss 2 Walking
		{SPR_BOSS8_W2,pis_scaled},					// 273 - Final Boss 2 Walking
		{SPR_BOSS8_W3,pis_scaled},					// 274 - Final Boss 2 Walking
		{SPR_BOSS8_W4,pis_scaled},					// 275 - Final Boss 2 Walking

		{SPR_BOSS9_W1,pis_scaled},					// 276 - Final Boss 3 Walking
		{SPR_BOSS9_W2,pis_scaled},					// 277 - Final Boss 3 Walking
		{SPR_BOSS9_W3,pis_scaled},					// 278 - Final Boss 3 Walking
		{SPR_BOSS9_W4,pis_scaled},					// 279 - Final Boss 3 Walking

		{SPR_BOSS10_W1,pis_scaled},				// 280 - Final Boss 4 Walking
		{SPR_BOSS10_W2,pis_scaled},				// 281 - Final Boss 4 Walking
		{SPR_BOSS10_W3,pis_scaled},				// 282 - Final Boss 4 Walking
		{SPR_BOSS10_W4,pis_scaled},				// 283 - Final Boss 4 Walking
};

// anim table holds info about each different animation.
//
piAnimInfo piAnimTable[] =
{
	{136,0,2,0,20,pia_shapetable,pid_cycle},		// 0 -	OPEN
	{127,0,3,0,20,pia_shapetable,pid_cycle},		// 1 -	podeggobj,
	{123,0,2,0,20,pia_shapetable,pid_cycle},		// 2 -	electroshotobj,
	{126,0,1,0,20,pia_shapetable,pid_cycle},		// 3 -	security_lightobj - Alerted

	{20,0,4,0,20,pia_shapetable,pid_cycle},		// 4 -	rentacopobj,
	{24,0,8,0,20,pia_shapetable,pid_cycle},		// 5 -	hang_terrotobj,
	{242,0,8,0,20,pia_shapetable,pid_rebound},	// 6 -   VPost
	{250,0,8,0,20,pia_shapetable,pid_rebound},	// 7 - 	VSpike
	{258,0,10,0,10,pia_shapetable,pid_cycle},		// 8 -   Security Cube
	{56,0,4,0,20,pia_shapetable,pid_cycle},		// 9 -	gen_scientistobj,
	{60,0,4,0,20,pia_shapetable,pid_cycle},		// 10 -	podobj,
	{64,0,4,0,20,pia_shapetable,pid_cycle},		// 11 -	electroobj,
	{68,0,3,0,20,pia_shapetable,pid_cycle},		// 12 -	electrosphereobj,
	{71,0,4,0,20,pia_shapetable,pid_cycle},		// 13 - 	proguardobj,
	{75,0,4,0,20,pia_shapetable,pid_cycle},		// 14 -	genetic_guardobj,
	{79,0,4,0,20,pia_shapetable,pid_cycle},		// 15 -	mutant_human1obj,
	{83,0,4,0,20,pia_shapetable,pid_cycle},		// 16 -	mutant_human2obj,
	{87,0,1,0,20,pia_shapetable,pid_cycle},		// 17 -	lcan_wait_alienobj,
	{88,0,4,0,20,pia_shapetable,pid_cycle},		// 18 -	lcan_alienobj,
	{92,0,1,0,20,pia_shapetable,pid_cycle},		// 19 -	scan_wait_alienobj,
	{93,0,4,0,20,pia_shapetable,pid_cycle},		// 20 -	scan_alienobj,
	{97,0,1,0,20,pia_shapetable,pid_cycle},		// 21 - 	gurney_waitobj,
	{98,0,4,0,20,pia_shapetable,pid_cycle},		// 22 -	gurneyobj,
	{102,0,3,0,20,pia_shapetable,pid_cycle},		// 23 -	liquidobj,
	{105,0,4,0,20,pia_shapetable,pid_cycle},		// 24 -	swatobj,
	{109,0,4,0,20,pia_shapetable,pid_cycle},		// 25 -	goldsternobj,
	{113,0,1,0,20,pia_shapetable,pid_cycle},		// 26 -	OPEN

	{131,0,3,0,20,pia_shapetable,pid_cycle},		// 27 -	flickerlightobj,
	{134,0,2,0,20,pia_shapetable,pid_cycle},		// 28 -	playerspshotobj,

	{141,0,3,0,20,pia_shapetable,pid_cycle},		// 29 -  Electric Arc barrier

	{150,0,4,0,20,pia_shapetable,pid_cycle},		// 30 -  Boss 1
	{154,0,4,0,20,pia_shapetable,pid_cycle},		// 31 -  Boss 2
	{158,0,4,0,20,pia_shapetable,pid_cycle},		// 32 -  Boss 3
	{162,0,4,0,20,pia_shapetable,pid_cycle},		// 33 -  Boss 4
	{166,0,4,0,20,pia_shapetable,pid_cycle},		// 34 -  Boss 5
	{170,0,4,0,20,pia_shapetable,pid_cycle},		// 35 -  Boss 6

	{48,0,3,0,10,pia_shapetable,pid_cycle},		// 36 -  Barrier Post
	{141,0,3,0,10,pia_shapetable,pid_cycle},		// 37 -  Barrier Arc

	{208,0,8,0,10,pia_grabscript,pid_cycle},		// 38 -  VMT
	{209,0,8,0,8,pia_grabscript,pid_cycle},		// 39 -  PerScan

	{232,0,4,0,20,pia_shapetable,pid_cycle},		// 40 - Dr. Goldstern Morph Walking

   {268,0,4,0,20,pia_shapetable,pid_cycle},		// 41 - Final Boss #1 - Walking
	{272,0,4,0,20,pia_shapetable,pid_cycle},		// 42 - Final Boss #2 - Walking
	{276,0,4,0,20,pia_shapetable,pid_cycle},		// 43 - Final Boss #3 - Walking
	{280,0,4,0,20,pia_shapetable,pid_cycle},		// 44 - Final Boss #4 - Walking

};

// anim list is created on the fly from the anim table...
// this allows a single animation to be displayed in more than
// one place...
//
piAnimInfo piAnimList[TP_MAX_ANIMS];
Uint8 TPscan;

// Bunch of general globals!
//
//static char pb[MAX_PB];
static char old_fontnumber;
static Sint16 length;

enum {jm_left,jm_right,jm_flush};
static char justify_mode = jm_left;

static Uint16 flags;

static Sint16 bgcolor,ltcolor,dkcolor,shcolor,anim_bgcolor=-1;
static Uint16 xl,yl,xh,yh;
static Uint16 cur_x, cur_y, last_cur_x, last_cur_y;
static const char *first_ch;

static const char *scan_ch;
static char temp;
static Sint16 scan_x,numanims,stemp;

static fontstruct *font;

static PresenterInfo *pi;

static Sint16 disp_str_num = -1;
static Sint16 music_num;
static Sint16 save_cx[TP_CURSOR_SAVES+1]={0,0,0,0,0,0,0,0,0};
static Sint16 save_cy[TP_CURSOR_SAVES+1]={0,0,0,0,0,0,0,0,0};
static Sint16 pagex[2],pagey[2];

//--------------------------------------------------------------------------
// TP_Presenter()
//--------------------------------------------------------------------------
void TP_Presenter(PresenterInfo *pinfo)
{
	pi=pinfo;
	bgcolor = pi->bgcolor;
	ltcolor = pi->ltcolor;
	dkcolor = pi->dkcolor;
	shcolor = pi->shcolor;
	xl=pi->xl+TP_MARGIN;
	yl=pi->yl+TP_MARGIN;
	xh=pi->xh-TP_MARGIN;
	yh=pi->yh-TP_MARGIN;
	music_num=-1;

	flags |= fl_clearscback;
	if ((pi->flags & TPF_USE_CURRENT) && (pi->cur_x != 0xffff) && (pi->cur_y != 0xffff))
	{
		if (pi->flags & TPF_SHOW_CURSOR)
		{
			cur_x = px;
			cur_y = py;
		}
		else
		{
			cur_x = pi->cur_x;
			cur_y = pi->cur_y;
		}
	}
	else
	{
		cur_x = xl;
		cur_y = yl;
	}
	first_ch = pi->script[0];
	pi->pagenum = numanims = 0;
	disp_str_num = -1;

	old_fontnumber=fontnumber;
	fontnumber=pi->fontnumber;
	TP_PurgeAllGfx();
	TP_CachePage(first_ch);
	font = (fontstruct *)grsegs[STARTFONT+fontnumber];
	flags = fl_presenting|fl_startofline;
	if (*first_ch == TP_CONTROL_CHAR)
		TP_HandleCodes();

// Display info UNDER defined region.
//
	if (pi->infoline)
	{
		char oldf=fontnumber,oldc=fontcolor;

		px=xl;
		py=yh+TP_MARGIN+1;
		fontnumber=2;
		fontcolor=0x39;
		VWB_Bar(xl-TP_MARGIN,py,xh-xl+1+(TP_MARGIN*2),8,bgcolor);
		ShPrint(pi->infoline,shcolor,false);

		if (pi->flags & TPF_SHOW_PAGES)
		{
			px=246;
			py=190;
			ShPrint("PAGE ",shcolor,false);
			pagex[0]=px;
			pagey[0]=py;
			ShPrint("   OF ",shcolor,false);
			pagex[1]=px;
			pagey[1]=py;

			TP_PrintPageNumber();
		}

		fontcolor=oldc;
		fontnumber=oldf;
	}

	font = (fontstruct *)grsegs[STARTFONT+fontnumber];
	if (!(pi->flags & TPF_USE_CURRENT))
		VWB_Bar(xl-TP_MARGIN,yl-TP_MARGIN,xh-xl+1+(TP_MARGIN*2),yh-yl+1+(TP_MARGIN*2),bgcolor);

	if (pi->flags & TPF_SHOW_CURSOR)
	{
		px = cur_x;
		py = cur_y;
		TP_Print("@",true);
	}

// Debug stuff -- draws box AROUND text presenter's printable region.
//
//	VL_Hlin(xl-TP_MARGIN,yl-TP_MARGIN,xh-xl+1+(TP_MARGIN*2),255);
//	VL_Vlin(xh+TP_MARGIN,yl-TP_MARGIN,yh-yl+1+(TP_MARGIN*2),255);
//	VL_Hlin(xl-TP_MARGIN,yh+TP_MARGIN,xh-xl+1+(TP_MARGIN*2),255);
//	VL_Vlin(xl-TP_MARGIN,yl-TP_MARGIN,yh-yl+1+(TP_MARGIN*2),255);
//
//

#ifdef DRAW_TO_FRONT
	VW_UpdateScreen();
	bufferofs=displayofs;
#endif
	while (flags & fl_presenting)
		if (*first_ch == TP_CONTROL_CHAR)
			TP_HandleCodes();
		else
			TP_WrapText();

	if (flags & fl_uncachefont)
		UNCACHEGRCHUNK(STARTFONT+fontnumber);
	fontnumber = old_fontnumber;
	pi->cur_x = cur_x;
	pi->cur_y = cur_y;

	if (pi->flags & TPF_SHOW_CURSOR)
	{
		cur_x = px = last_cur_x;
		cur_y = py = last_cur_y;
	}
	else
	{
		px = cur_x;
		py = cur_y;
	}

	pi->cur_x = cur_x;
	pi->cur_y = cur_y;

#ifdef DRAW_TO_FRONT
	displayofs = bufferofs;
	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;
#endif
}

//--------------------------------------------------------------------------
// TP_WrapText()
//--------------------------------------------------------------------------
void TP_WrapText()
{
	flags &= ~fl_startofline;

	if (stemp=TP_LineCommented(first_ch))
	{
		first_ch += stemp;
		return;
	}

// Parse script until one of the following:
//
// 1) text extends beyond right margin
// 2) NULL termination is reached
// 3) TP_RETURN_CHAR is reached
// 4) TP_CONTROL_CHAR is reached
//
	scan_x = cur_x;
	scan_ch = first_ch;
	while (((Uint16)(scan_x)+(Uint16)(ch_width(*scan_ch)) <= xh) && (*scan_ch) &&
			 (*scan_ch != TP_RETURN_CHAR) && (*scan_ch != TP_CONTROL_CHAR))
		scan_x += ch_width(*scan_ch++);

// If 'text extends beyond right margin', scan backwards for
// a SPACE
//
	if ((Uint16)scan_x+(Uint16)(ch_width(*scan_ch)) > xh)
	{
		Sint16 last_x = scan_x;
		const char *last_ch = scan_ch;

		while ((scan_ch != first_ch) && (*scan_ch != ' ') && (*scan_ch != TP_RETURN_CHAR))
			scan_x -= ch_width(*scan_ch--);

		if (scan_ch == first_ch)
		{
			if (cur_x != xl)
				goto tp_newline;

			scan_ch = last_ch;
			scan_x = last_x;
		}
	}

// print current line
//
	temp = *scan_ch;
	*(char*)scan_ch = 0;

	if ((justify_mode == jm_right) && (!(flags & fl_center)))
	{
		Uint16 width,height;

		VWL_MeasureString(first_ch,&width,&height,font);
		cur_x = xh-width+1;
		if (cur_x < xl)
			cur_x = xl;
	}

	px = cur_x;
	py = cur_y;

	length = scan_ch-first_ch+1;				// USL_DrawString only works with
//	if (length > MAX_PB)                   //
//		TP_ERROR(TP_PRESENTER_LONG_TEXT);   //
//	_fmemcpy(pb,first_ch,length);    		// near pointers...

	if (*first_ch != TP_RETURN_CHAR)
	{
		if (pi->print_delay)
			TP_SlowPrint(first_ch,pi->print_delay);
		else
			TP_Print(first_ch,false);
	}

	*(char*)*scan_ch = temp;
	first_ch = scan_ch;

tp_newline:;
	flags &= ~fl_center;

// Skip SPACE at end of wrapped line.
//
	if ((first_ch[0] == ' ') && (first_ch[1] != ' '))
		first_ch++;

// Skip end-of-line designators
//
	if (first_ch[0] == TP_RETURN_CHAR)
	{
		if (first_ch[1] == '\n')
			first_ch += 2;
		else
			first_ch++;
	}

// TP_CONTROL_CHARs don't advance to next character line
//
	if ((*scan_ch != TP_CONTROL_CHAR) && *scan_ch)
	{
		char old_color=fontcolor;

	// Remove cursor.
	//
		if (pi->flags & TPF_SHOW_CURSOR)
		{
			fontcolor = bgcolor;
			px = last_cur_x;
			py = last_cur_y;
			TP_Print("@",true);
			fontcolor = old_color;
		}

		cur_x = xl;

	// If next line will be printed out of defined region, scroll up!
	//
		if ((pi->flags & TPF_SCROLL_REGION) && (cur_y+(font_height*2) > yh))
		{
			VL_ScreenToScreen(bufferofs+((((yl+font_height+is_shadowed)*320)+xl)/4),
									bufferofs+(((yl*320)+xl)/4),
									(xh-xl+1)/4,
									(yh-yl+1)-font_height+is_shadowed);

			VWB_Bar(cur_x,cur_y,xh-xl+1+(TP_MARGIN*2),yh-cur_y+1,bgcolor);
			VW_MarkUpdateBlock(xl,yl,xh,yh);

			if (cur_y+font_height > yh)
				cur_y = yh-font_height+1-is_shadowed;
		}
		else
			cur_y += font_height+is_shadowed;

	// Display cursor.
	//
		if (pi->flags & TPF_SHOW_CURSOR)
		{
			px = cur_x;
			py = cur_y;
			TP_Print("@",true);
		}
	}
}

//--------------------------------------------------------------------------
// TP_HandleCodes()
//--------------------------------------------------------------------------
void TP_HandleCodes()
{
	ControlInfo ci;
	piAnimInfo *anim;
	piShapeInfo *shape;
	Uint16 shapenum;
	Sint16 length;
	const char *s;
	Sint16 old_bgcolor;
	signed char c;

	if ((first_ch[-2] == TP_RETURN_CHAR) && (first_ch[-1] == '\n'))
		flags |= fl_startofline;

	while (*first_ch == TP_CONTROL_CHAR)
	{
		#define TP_MORE_TEXT "<MORE>"

		char temp;

		first_ch++;
#ifndef TP_CASE_SENSITIVE
		*first_ch=toupper(*first_ch);
		*(first_ch+1)=toupper(*(first_ch+1));
#endif
        Uint16 subcode;
        Uint16 code = *reinterpret_cast<const Uint16*>(first_ch);
        first_ch += 2;

		switch (code)
		{
	// CENTER TEXT ------------------------------------------------------
	//
			case TP_CNVT_CODE('C','E'):
				length = 0;
				s = first_ch;
				while (*s && (*s != TP_RETURN_CHAR))
				{
					switch (*s)
					{
						case TP_CONTROL_CHAR:
							s++;
                            subcode = *reinterpret_cast<const Uint16*>(s);
                            s += 2;
							switch (subcode)
							{
								case TP_CNVT_CODE('S','X'):
								case TP_CNVT_CODE('R','X'):
								case TP_CNVT_CODE('S','Y'):
								case TP_CNVT_CODE('R','Y'):
								case TP_CNVT_CODE('F','N'):
								case TP_CNVT_CODE('S','T'):
								case TP_CNVT_CODE('B','X'):
								case TP_CNVT_CODE('S','P'):
									s++;
								break;

								case TP_CNVT_CODE('F','C'):
								case TP_CNVT_CODE('B','C'):
								case TP_CNVT_CODE('S','C'):
								case TP_CNVT_CODE('L','C'):
								case TP_CNVT_CODE('D','C'):
								case TP_CNVT_CODE('A','X'):
								case TP_CNVT_CODE('A','Y'):
								case TP_CNVT_CODE('H','C'):
									s += 2;
								break;

								case TP_CNVT_CODE('L','M'):
								case TP_CNVT_CODE('R','M'):
								case TP_CNVT_CODE('P','X'):
								case TP_CNVT_CODE('P','Y'):
									s += 3;
								break;

								case TP_CNVT_CODE('S','H'):
									shapenum = TP_VALUE(first_ch,3);
									s += 3;
									shape = &piShapeTable[shapenum];
									length += TP_BoxAroundShape(-1,-1,shape->shapenum,shape->shapetype);
								break;

								case TP_CNVT_CODE('A','N'):
									shapenum = TP_VALUE(first_ch,2);
									s += 2;
									anim = &piAnimTable[shapenum];
									switch (anim->animtype)
									{
										case pia_shapetable:
											shape = &piShapeTable[anim->baseshape+anim->frame];
											length += TP_BoxAroundShape(-1,-1,shape->shapenum,shape->shapetype);
										break;

										case pia_grabscript:
											shape = &piShapeTable[anim->baseshape];
											length += TP_BoxAroundShape(-1,-1,shape->shapenum+anim->frame,shape->shapetype);
										break;
									}
								break;

								case TP_CNVT_CODE('Z','Z'):
								case TP_CNVT_CODE('D','M'):
								case TP_CNVT_CODE('C','E'):
								case TP_CNVT_CODE('E','P'):
								case TP_CNVT_CODE('L','J'):
								case TP_CNVT_CODE('R','J'):
								case TP_CNVT_CODE('X','X'):
								case TP_CNVT_CODE('S','L'):
								case TP_CNVT_CODE('R','L'):
								case TP_CNVT_CODE('B','E'):
								case TP_CNVT_CODE('H','I'):
								case TP_CNVT_CODE('P','A'):
								case TP_CNVT_CODE('M','O'):
								case TP_CNVT_CODE('H','O'):
								case TP_CNVT_CODE('H','F'):
								case TP_CNVT_CODE('S','B'):
									// No parameters to pass over!
								break;
							}
						break;

						default:
							length += ch_width(*s++);
						break;
					}
				}
				cur_x += (Uint16)((xh-cur_x+1)-length)/2;
				flags |= fl_center;

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// DRAW SHAPE -------------------------------------------------------
	//
			case TP_CNVT_CODE('S','H'):
				shapenum = TP_VALUE(first_ch,3);
				first_ch += 3;
				shape = &piShapeTable[shapenum];
				TP_DrawShape(cur_x,cur_y,shape->shapenum,shape->shapetype);
			break;

	// CLEAR SCALED BACKGROUND -------------------------------------------
	//
			case TP_CNVT_CODE('S','B'):
				if (TP_VALUE(first_ch++,1))
					flags |= fl_clearscback;
				else
					flags &= ~fl_clearscback;
			break;

	// HIGHLIGHT COLOR ---------------------------------------------------
	//
			case TP_CNVT_CODE('H','C'):
				pi->highlight_color = TP_VALUE(first_ch,2);
				first_ch += 2;
			break;

	// HIGHLIGHT ON ------------------------------------------------------
	//
			case TP_CNVT_CODE('H','O'):
				pi->fontcolor = fontcolor;
				fontcolor = pi->highlight_color;
			break;

	// HIGHLIGHT OFF -----------------------------------------------------
	//
			case TP_CNVT_CODE('H','F'):
				fontcolor = pi->fontcolor;
			break;

	// ALTER X ----------------------------------------------------------
	//
			case TP_CNVT_CODE('A','X'):
				c = TP_VALUE(first_ch,2);
				first_ch += 2;
				cur_x += c;
			break;

	// ALTER Y ----------------------------------------------------------
	//
			case TP_CNVT_CODE('A','Y'):
				c = TP_VALUE(first_ch,2);
				first_ch += 2;
				cur_y += c;
			break;

	// INIT ANIMATION ---------------------------------------------------
	//
			case TP_CNVT_CODE('A','N'):
				shapenum = TP_VALUE(first_ch,2);
				first_ch += 2;
				memcpy(&piAnimList[numanims],&piAnimTable[shapenum],sizeof(piAnimInfo));
				anim = &piAnimList[numanims++];

				anim->y=cur_y;
				switch (anim->animtype)
				{
					case pia_shapetable:
						shape = &piShapeTable[anim->baseshape+anim->frame];
						anim->x=TP_DrawShape(cur_x,cur_y,shape->shapenum,shape->shapetype);
					break;

					case pia_grabscript:
						shape = &piShapeTable[anim->baseshape];
						anim->x=TP_DrawShape(cur_x,cur_y,shape->shapenum+anim->frame,shape->shapetype);
					break;
				}

				anim->diradd=1;

//				spr = &spritetable[shape->shapenum-STARTSPRITES];

				if (anim_bgcolor == -1)
					anim_bgcolor = bgcolor;
			break;

	// FONT COLOR -------------------------------------------------------
	//
			case TP_CNVT_CODE('F','C'):
				fontcolor = TP_VALUE(first_ch,2);
				first_ch += 2;
			break;

	// SHADOW COLOR ------------------------------------------------------
	//
			case TP_CNVT_CODE('S','C'):
				shcolor = TP_VALUE(first_ch,2);
				first_ch += 2;
			break;

	// LIGHT COLOR -------------------------------------------------------
	//
			case TP_CNVT_CODE('L','C'):
				ltcolor = TP_VALUE(first_ch,2);
				first_ch += 2;
			break;

	// SAVE LOCATION -----------------------------------------------------
	//
			case TP_CNVT_CODE('S','L'):
				save_cx[TP_CURSOR_SAVES] = cur_x;
				save_cy[TP_CURSOR_SAVES] = cur_y;
			break;

	// RESTORE LOCATION --------------------------------------------------
	//
			case TP_CNVT_CODE('R','L'):
				cur_x = save_cx[TP_CURSOR_SAVES];
				cur_y = save_cy[TP_CURSOR_SAVES];

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// SAVE X LOCATION ---------------------------------------------------
	//
			case TP_CNVT_CODE('S','X'):
				temp = TP_VALUE(first_ch++,1);
				if (pi->flags & TPF_SHOW_CURSOR)
					save_cx[temp] = last_cur_x;
				else
					save_cx[temp] = cur_x;
			break;

	// RESTORE X LOCATION ------------------------------------------------
	//
			case TP_CNVT_CODE('R','X'):
				temp = TP_VALUE(first_ch++,1);
				cur_x = save_cx[temp];

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// SAVE Y LOCATION ---------------------------------------------------
	//
			case TP_CNVT_CODE('S','Y'):
				temp = TP_VALUE(first_ch++,1);
				if (pi->flags & TPF_SHOW_CURSOR)
					save_cy[temp] = last_cur_y;
				else
					save_cy[temp] = cur_y;
			break;

	// RESTORE Y LOCATION ------------------------------------------------
	//
			case TP_CNVT_CODE('R','Y'):
				temp = TP_VALUE(first_ch++,1);
				cur_y = save_cy[temp];

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// NOP ---------------------------------------------------------------
	//
			case TP_CNVT_CODE('Z','Z'):
			break;

	// DARK COLOR --------------------------------------------------------
	//
			case TP_CNVT_CODE('D','C'):
				dkcolor = TP_VALUE(first_ch,2);		// ^ bgcolor;
				first_ch += 2;
			break;

	// FONT NUMBER -------------------------------------------------------
	//
			case TP_CNVT_CODE('F','N'):
				if (flags & fl_uncachefont)
				{
					UNCACHEGRCHUNK(STARTFONT+fontnumber);
					flags &= ~fl_uncachefont;
				}
				fontnumber = TP_VALUE(first_ch++,1);
				if (!grsegs[STARTFONT+fontnumber])
				{
					TP_CacheIn(ct_chunk,STARTFONT+fontnumber);
					flags |= fl_uncachefont;
				}
				else
					font = (fontstruct *)grsegs[STARTFONT+fontnumber];
			break;

	// BACKGROUND COLOR -------------------------------------------------
	//
			case TP_CNVT_CODE('B','C'):
				bgcolor = TP_VALUE(first_ch,2);
				first_ch += 2;
			break;

	// SHADOW TEXT ------------------------------------------------------
	//
			case TP_CNVT_CODE('S','T'):
				if (TP_VALUE(first_ch++,1))
					flags |= fl_shadowtext;
				else
					flags &= ~fl_shadowtext;
			break;

	// SHADOW PIC -------------------------------------------------------
	//
			case TP_CNVT_CODE('S','P'):
				if (TP_VALUE(first_ch++,1))
					flags |= fl_shadowpic;
				else
					flags &= ~fl_shadowpic;
			break;

	// BOX SHAPES -------------------------------------------------------
	//
			case TP_CNVT_CODE('B','X'):
				if (TP_VALUE(first_ch++,1))
					flags |= fl_boxshape;
				else
					flags &= ~fl_boxshape;
			break;

	// LEFT MARGIN ------------------------------------------------------
	//
			case TP_CNVT_CODE('L','M'):
				shapenum = TP_VALUE(first_ch,3);
				first_ch += 3;
				if (shapenum == 0xfff)
					xl = cur_x;
				else
					xl = shapenum;
					if (cur_x < xl)
						cur_x = xl;
			break;

	// RIGHT MARGIN -----------------------------------------------------
	//
			case TP_CNVT_CODE('R','M'):
				shapenum = TP_VALUE(first_ch,3);
				first_ch += 3;
				if (shapenum == 0xfff)
					xh = cur_x;
				else
					xh = shapenum;
			break;

	// DEFAULT MARGINS --------------------------------------------------
	//
			case TP_CNVT_CODE('D','M'):
				xl = pi->xl+TP_MARGIN;
				yl = pi->yl+TP_MARGIN;
				xh = pi->xh-TP_MARGIN;
				yh = pi->yh-TP_MARGIN;
			break;

	// SET X COORDINATE -------------------------------------------------
	//
			case TP_CNVT_CODE('P','X'):
				cur_x = TP_VALUE(first_ch,3);
				first_ch += 3;

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// SET Y COORDINATE -------------------------------------------------
	//
			case TP_CNVT_CODE('P','Y'):
				cur_y = TP_VALUE(first_ch,3);
				first_ch += 3;

				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();
			break;

	// LEFT JUSTIFY -----------------------------------------------------
	//
			case TP_CNVT_CODE('L','J'):
				justify_mode = jm_left;
			break;

	// RIGHT JUSTIFY ----------------------------------------------------
	//
			case TP_CNVT_CODE('R','J'):
				justify_mode = jm_right;
			break;

	// BELL -------------------------------------------------------------
	//
			case TP_CNVT_CODE('B','E'):
				SD_PlaySound(TERM_BEEPSND);
				SD_WaitSoundDone();
			break;

	// HIDE CURSOR ------------------------------------------------------
	//
			case TP_CNVT_CODE('H','I'):
#if 0
				px = cur_x;
				py = cur_y;
				old_color = fontcolor;
				fontcolor = TERM_BCOLOR;
				USL_DrawString("@");
				fontcolor = old_color;
#endif
			break;

	// PAUSE -----------------------------------------------------------
	//
			case TP_CNVT_CODE('P','A'):
         	{
            	char i;

					for (i=0;i<30;i++)
      	      {
         	   	VW_WaitVBL(1);
            	   CycleColors();
	            }
				}
			break;


	// MORE ------------------------------------------------------------
	//
			case TP_CNVT_CODE('M','O'):
				if (pi->print_delay)
					TP_SlowPrint(TP_MORE_TEXT,pi->print_delay);
				else
					TP_Print(TP_MORE_TEXT,false);

				LastScan = 0;
				do {
					IN_ReadControl(0,&ci);
				} while (!ci.button0 && !ci.button1 && !ci.button2 &&
							!ci.button3 && (ci.dir == dir_None) &&
							(!LastScan));

				cur_x = xl;
				VWB_Bar(cur_x,cur_y,xh-xl+1+(TP_MARGIN*2),font_height+is_shadowed,bgcolor);
				if (pi->flags & TPF_SHOW_CURSOR)
					TP_JumpCursor();

				if (LastScan == sc_Escape)
					flags &= ~fl_presenting;
				TPscan=LastScan;
			break;

	// DISPLAY STRING --------------------------------------------------
	//
			case TP_CNVT_CODE('D','S'):
			{
				const char *old_first_ch;

				disp_str_num = TP_VALUE(first_ch,2);
				if (disp_str_num >= PI_MAX_NUM_DISP_STRS)
					TP_ERROR(TP_DISPLAY_STR_NUM_BAD);

				old_first_ch = first_ch+2;

				if (first_ch = (char *)piStringTable[disp_str_num])
				{
					while (flags & fl_presenting && *first_ch)
						if (*first_ch == TP_CONTROL_CHAR)
							TP_HandleCodes();
						else
							TP_WrapText();
				}

				first_ch = old_first_ch;
			}
			break;

	// PLAY MUSIC -------------------------------------------------------
	//
			case TP_CNVT_CODE('P','M'):
				temp = TP_VALUE(first_ch,2);
				if ((temp < LASTMUSIC) && (temp != music_num))
				{
					music_num=temp;
					TP_CacheIn(ct_music,music_num);
				}
				first_ch += 2;
			break;

	// PLAY SOUND -------------------------------------------------------
	//
			case TP_CNVT_CODE('P','S'):
				temp = TP_VALUE(first_ch,2);
				if ((temp < LASTSOUND)) // && (temp != music_num))
				{
					TP_CacheIn(ct_scaled,0);
					SD_PlaySound(static_cast<soundnames>(temp));
				}
				first_ch += 2;
			break;


	// END OF PAGE ------------------------------------------------------
	//
			case TP_CNVT_CODE('E','P'):
				VW_UpdateScreen();
#ifdef DRAW_TO_FRONT
				bufferofs=displayofs;
#endif
				if (screenfaded)
					VW_FadeIn();

				if (anim_bgcolor != -1)
				{
					old_bgcolor = bgcolor;
					bgcolor=anim_bgcolor;
				}

				while (1)
				{
					CycleColors();							// specific for VGA 3D engine!
               CalcTics();

					TP_AnimatePage(numanims);
					VW_UpdateScreen();
#ifdef DRAW_TO_FRONT
					bufferofs=displayofs;
#endif
					ReadAnyControl(&ci);

					if (Keyboard[sc_PgUp])
						ci.dir = dir_North;
					else
					if (Keyboard[sc_PgDn])
						ci.dir = dir_South;

					if (pi->flags & TPF_CONTINUE && (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter]))
					{
						EscPressed = false;
						flags &= ~fl_presenting;
						break;
					}

					if (ci.button1 || Keyboard[sc_Escape])
					{
						EscPressed = true;
						flags &= ~fl_presenting;
						TPscan=sc_Escape;
						break;
					}
					else
					{
						if (((ci.dir == dir_North) || (ci.dir == dir_West)) && (pi->pagenum))
						{
							if (flags & fl_upreleased)
							{
								pi->pagenum--;
								flags &= ~fl_upreleased;
								break;
							}
						}
						else
						{
							flags |= fl_upreleased;
							if (((ci.dir == dir_South) || (ci.dir == dir_East)) && (pi->pagenum < pi->numpages-1))
							{
								if (flags & fl_dnreleased)
								{
									pi->pagenum++;
									flags &= ~fl_dnreleased;
									break;
								}
							}
							else
								flags |= fl_dnreleased;
						}
					}
				}

				if (anim_bgcolor != -1)
				{
					bgcolor = old_bgcolor;
					anim_bgcolor = -1;
				}

				cur_x = xl;
				cur_y = yl;
				if (cur_y+font_height > yh)
					cur_y = yh-font_height;
				first_ch = pi->script[pi->pagenum];

				numanims = 0;
				TP_PurgeAllGfx();
				TP_CachePage(first_ch);

				if (*first_ch == TP_CONTROL_CHAR)
				{
					TP_HandleCodes();
					flags &= ~fl_startofline;
				}
				VWB_Bar(xl,yl,xh-xl+1,yh-yl+1,bgcolor);
				TP_PrintPageNumber();
//				VWB_Bar(xl-TP_MARGIN,yl-TP_MARGIN,xh-xl+1+(TP_MARGIN*2),yh-yl+1+(TP_MARGIN*2),bgcolor);
			break;

	// EXIT PRESENTER ---------------------------------------------------
	//
			case TP_CNVT_CODE('X','X'):
				flags &= ~fl_presenting;
				VW_UpdateScreen();
			break;
		}
	}

	if ((first_ch[0] == TP_RETURN_CHAR) && (first_ch[1] == '\n') && (flags & fl_startofline))
		first_ch += 2;
}

//--------------------------------------------------------------------------
// TP_PrintPageNumber()
//--------------------------------------------------------------------------
void TP_PrintPageNumber()
{
	char buffer[5];
	char oldf=fontnumber,oldc=fontcolor;

	if (!(pi->flags & TPF_SHOW_PAGES))
		return;

	fontnumber=2;
	fontcolor=0x39;

// Print current page number.
//
	px=pagex[0];
	py=pagey[0];
	VW_Bar(px,py,12,7,0xe3);
	sprintf(buffer,"%02d",pi->pagenum+1);
	ShPrint(buffer,shcolor,false);

// Print current page number.
//
	if ((px=pagex[1]) > -1)
	{
		py=pagey[1];
		sprintf(buffer,"%02d",pi->numpages);
		ShPrint(buffer,shcolor,false);
		pagex[1]=-1;
	}

	fontnumber=oldf;
	fontcolor=oldc;
}

//--------------------------------------------------------------------------
// TP_DrawShape()
//--------------------------------------------------------------------------
Sint16 TP_DrawShape(Sint16 x, Sint16 y, Sint16 shapenum, pisType shapetype)
{
	Sint16 width;
	void* addr;

// Mask 'x coordinate' when displaying certain shapes
//
	switch (shapetype)
	{
		case pis_pic:
		case pis_latchpic:
			x = (x+7) & 0xFFF8;
		break;
        default:
            break;
	}

// Get width of shape (also, draws a box/shadow, if needed)
//
	width=TP_BoxAroundShape(x,y,shapenum,shapetype);

// Draw this shape!
//
	switch (shapetype)
	{
		case pis_scwall:
			TP_CacheIn(ct_scaled,0);
			addr = PM_GetPage(shapenum);
			bufferofs += (y-30)*SCREENWIDTH;
			postx = x;
			postwidth = 1;
			postsource = (const Uint8*)addr;
			for (x=0;x<64;x++,postx++,postsource+=64)
			{
				wallheight[postx] = 256;
				FarScalePost();
			}
			bufferofs -= (y-30)*SCREENWIDTH;
		break;

		case pis_scaled:
			TP_CacheIn(ct_scaled,0);
			if (flags & fl_clearscback)
				VWB_Bar(x,y,64,64,bgcolor);
			MegaSimpleScaleShape(x+32,y+32,shapenum,64,0);
		break;

#if NUMPICS
		case pis_latchpic:
			LatchDrawPic(x>>3,y,shapenum);
		break;

		case pis_pic:
#if TP_640x200
		case pis_pic2x:
#endif
			VWB_DrawPic(x,y,shapenum);
//			VW_geDrawPic(x>>3,y,shapenum,shapetype == pis_pic2x);
		break;
#endif

#if NUMSPRITES
		case pis_sprite:
#if TP_640x200
		case pis_sprite2x:
#endif
//			VW_geDrawSprite(x,y-(spr->orgy>>G_P_SHIFT),shapenum,shapetype == pis_sprite2x);
		break;
#endif
        default:
            break;
	}

// Advance current x position past shape and tell calling function where
// (horizontally) this shape was drawn.
//
	cur_x += width;
	return(x);
}

//--------------------------------------------------------------------------
// TP_ResetAnims()
//--------------------------------------------------------------------------
void TP_ResetAnims()
{
	piAnimList[0].baseshape = -1;
}

//--------------------------------------------------------------------------
// TP_AnimatePage()
//--------------------------------------------------------------------------
void TP_AnimatePage(Sint16 numanims)
{
	piAnimInfo *anim=piAnimList;
	piShapeInfo *shape;

	while (numanims--)
	{
		anim->delay += tics;
		if (anim->delay >= anim->maxdelay)
		{
			anim->delay = 0;
			anim->frame += anim->diradd;
			if ((anim->frame == anim->maxframes) || (anim->frame < 0))
			{
				switch (anim->dirtype)
				{
					case pid_cycle:
						anim->frame = 0;
					break;

					case pid_rebound:
						anim->diradd = -anim->diradd;
						anim->frame += anim->diradd;
					break;
				}
			}

			switch (anim->animtype)
			{
				case pia_shapetable:
					shape = &piShapeTable[anim->baseshape+anim->frame];
					TP_DrawShape(anim->x,anim->y,shape->shapenum,shape->shapetype);
				break;

				case pia_grabscript:
					shape = &piShapeTable[anim->baseshape];
					TP_DrawShape(anim->x,anim->y,shape->shapenum+anim->frame,shape->shapetype);
				break;
			}
		}
		anim++;
	}
}

//--------------------------------------------------------------------------
// TP_BoxAroundShape()
//--------------------------------------------------------------------------
Sint16 TP_BoxAroundShape(Sint16 x1, Sint16 y1, Uint16 shapenum, pisType shapetype)
{
	Sint16 x2,y2;

	switch (shapetype)
	{
		Uint16 width;

		case pis_scwall:
		case pis_scaled:
			x2 = x1+63;
			y2 = y1+63;
		break;

#if NUMPICS
		case pis_pic:
		case pis_latchpic:
#if TP_640x200
		case pis_pic2x:
			width = pictable[shapenum-STARTPICS].width << (3+(shapetype==pis_pic2x));
#else
			width = pictable[shapenum-STARTPICS].width;
#endif
			x2 = x1+width-1;
			y2 = y1+(pictable[shapenum-STARTPICS].height)-1;
		break;
#endif

#if NUMSPRITES
		case pis_sprite:
#if TP_640x200
		case pis_sprite2x:
			width = spritetable[shapenum-STARTSPRITES].width << (3+(shapetype==pis_sprite2x));
#else
			width = spritetable[shapenum-STARTSPRITES].width << 3;
#endif
			x2 = x1+width-1;
			y2 = y1+(spritetable[shapenum-STARTSPRITES].height)-1;
		break;
#endif
        default:
            break;
	}

	if (flags & fl_boxshape)
	{
		x1 -= 1+TP_640x200;
		x2 += 1+TP_640x200;
		y1--;
		y2++;

		if (x1>=0 && y1>=0)
		{
			VWB_Hlin(x1,x2,y1,ltcolor);
			VWB_Hlin(x1,x2,y2,dkcolor);
			VWB_Vlin(y1,y2,x1,ltcolor);
			VWB_Vlin(y1,y2,x2,dkcolor);

#if TP_640x200
			VWB_Vlin(y1,y2,x1+1,ltcolor);
			VWB_Vlin(y1,y2,x2+1,dkcolor);
#endif
		}
	}

	if (flags & fl_shadowpic)
	{
		x2 += 1+TP_640x200;
		y2++;
		if (x1>=0 && y1>=0)
		{
			VWB_Hlin(x1+1+TP_640x200,x2,y2,shcolor);
			VWB_Vlin(y1+1,y2,x2,shcolor);
		}
	}

	return(x2-x1+1);
}

//--------------------------------------------------------------------------
// TP_PurgeAllGfx()
//--------------------------------------------------------------------------
void TP_PurgeAllGfx()
{
	Sint16 loop;

	if (pi->flags & TPF_CACHE_NO_GFX)
		return;										

	TP_ResetAnims();

	for (loop=STARTPICS; loop<CONTROLS_LUMP_START; loop++)
		if (grsegs[loop])
			UNCACHEGRCHUNK(loop);

	for (loop=CONTROLS_LUMP_END+1; loop<STARTPICS+NUMPICS; loop++)
		if (grsegs[loop])
			UNCACHEGRCHUNK(loop);
}

//--------------------------------------------------------------------------
// TP_CachePage()
//--------------------------------------------------------------------------
void TP_CachePage(const char *script)
{
	piAnimInfo *anim;
	Sint16 loop;
	Uint16 shapenum;
	boolean end_of_page=false;
	Sint16 numanims=0;

	if (pi->flags & TPF_CACHE_NO_GFX)
		return;

    Uint16 code;

	while (!end_of_page)
	{
		while (stemp=TP_LineCommented(script))
			script += stemp;

		switch (*script++)
		{
			case TP_CONTROL_CHAR:
#ifndef TP_CASE_SENSITIVE
				*script=toupper(*script);
				*(script+1)=toupper(*(script+1));
#endif
                code = *reinterpret_cast<const Uint16*>(script);
                script += 2;
				switch (code)
				{
					case TP_CNVT_CODE('S','H'):
						shapenum = TP_VALUE(script,3);
						script += 3;
						if (piShapeTable[shapenum].shapetype == pis_pic)
							CA_MarkGrChunk(piShapeTable[shapenum].shapenum);
					break;

					case TP_CNVT_CODE('A','N'):
						shapenum = TP_VALUE(script,2);
						script += 2;

						if (numanims++ == TP_MAX_ANIMS)
							TP_ERROR(TP_CACHEPAGE_ANIM_OF);

						anim = &piAnimTable[shapenum];
						switch (anim->animtype)
						{
							case pia_shapetable:
								for (loop=anim->baseshape;loop < anim->baseshape+anim->maxframes; loop++)
									if (piShapeTable[loop].shapetype == pis_pic)
										CA_MarkGrChunk(piShapeTable[loop].shapenum);
							break;

							case pia_grabscript:
								shapenum = piShapeTable[anim->baseshape].shapenum;
								if (piShapeTable[anim->baseshape].shapetype == pis_pic)
									for (loop=0; loop<anim->maxframes; loop++)
										CA_MarkGrChunk(shapenum+loop);
							break;
						}
					break;

					case TP_CNVT_CODE('X','X'):
					case TP_CNVT_CODE('E','P'):
						end_of_page = true;
					break;
				}
			break;
		}
	}

	TP_CacheIn(ct_marks,0);
}

//--------------------------------------------------------------------------
// TP_VALUE()
//--------------------------------------------------------------------------
Uint16 TP_VALUE(const char *ptr,char num_nybbles)
{
	char ch,nybble,shift;
	Uint16 value=0;

	for (nybble=0; nybble<num_nybbles; nybble++)
	{
		shift = 4*(num_nybbles-nybble-1);

		ch = *ptr++;
		if (isxdigit(ch))
        {
			if (isalpha(ch))
				value |= (toupper(ch)-'A'+10)<<shift;
			else
				value |= (ch-'0')<<shift;
        }
	}

	return(value);
}

//--------------------------------------------------------------------------
// TP_JumpCursor()
//--------------------------------------------------------------------------
void TP_JumpCursor()
{
	char old_color = fontcolor;

	fontcolor = bgcolor;
	px = last_cur_x;
	py = last_cur_y;
	TP_Print("@",true);
	px = cur_x = last_cur_x;
	py = cur_y = last_cur_y;
	fontcolor = old_color;
	TP_Print("@",true);
}

//--------------------------------------------------------------------------
// TP_Print()
//--------------------------------------------------------------------------
void TP_Print(const char *str,boolean single_char)
{

//
// The only test needed SHOULD be the first one ...
// "flags & fl shadowtext" ... Testing for '@' keeps the terminal's
// cursor from being shadowed -- sorta' specific for AOG...
//
// This should eventually be changed...
//
	LastScan = 0;

	last_cur_x = cur_x;
	last_cur_y = cur_y;

	if ((flags & fl_shadowtext) && (*str != '@'))
	{
		if (fontcolor == bgcolor)
			ShPrint(str,bgcolor,single_char);
		else
			ShPrint(str,shcolor,single_char);
	}
	else
		if (single_char)
		{
			char buf[2] = {0,0};

			buf[0] = *str;
			USL_DrawString(buf);
		}
		else
			USL_DrawString(str);

	cur_x = px;
	cur_y = py;

	if ((pi->flags & TPF_ABORTABLE) && LastScan)
		flags &= ~fl_presenting;
}

//--------------------------------------------------------------------------
// TP_SlowPrint()
//--------------------------------------------------------------------------
boolean TP_SlowPrint(const char *str, char delay)
{
	char old_color = fontcolor;
	Sint16 old_x,old_y;
	Sint32 tc;
	boolean aborted=false;

	while (*str)
	{
		if (pi->flags & TPF_SHOW_CURSOR)
		{
		// Remove the cursor.
		//
			fontcolor = bgcolor;
			px = old_x = last_cur_x;
			py = old_y = last_cur_y;
			TP_Print("@",true);
			px = old_x;
			py = old_y;
			fontcolor = old_color;
		}

	// If user aborted, print the whole string ...
	// Otherwise, just print a character ...
	//
		if (aborted)
			TP_Print(str,false);
		else
			TP_Print(str++,true);

	// Print cursor
	//
		if (pi->flags & TPF_SHOW_CURSOR)
			TP_Print("@",true);

		VW_UpdateScreen();

	// Break out on abort!
	//
		if (aborted)
			break;

	// Liven up the audio aspect!
	//
		if (pi->flags & TPF_TERM_SOUND)
			if (*str != ' ')
				SD_PlaySound(TERM_TYPESND);

	// Delay and check for abort (if needed).
	//
		if (!aborted)
		{
			LastScan=0;
			tc = TimeCount;
			while (TimeCount-tc < delay)
			{
				VW_WaitVBL(1);
				CycleColors();
				if (pi->flags & TPF_ABORTABLE)
				{
					if ((pi->flags & TPF_ABORTABLE) && LastScan)
					{
						aborted = true;
						break;
					}
				}
			}
		}
	}

	if (aborted)
		flags &= ~fl_presenting;

	return(aborted);
}

//--------------------------------------------------------------------------
// TP_LoadScript()
//--------------------------------------------------------------------------
Sint32 TP_LoadScript(const char *filename,PresenterInfo *pi, Uint16 id_cache)
{
	Sint32 size;

	if (id_cache)
	{
		const char *p;

		pi->id_cache=id_cache;
		CA_CacheGrChunk(id_cache);
		pi->scriptstart = grsegs[id_cache];
		if (!(p=strstr(static_cast<const char*>(grsegs[id_cache]),"^XX")))
      	TP_ERROR(TP_CANT_FIND_XX_TERMINATOR);

        // FIXME
		size = p-((char*)grsegs[id_cache] + 1);
	}
	else
	{
		pi->id_cache = -1;
		if (!(size=IO_LoadFile(filename,&pi->scriptstart)))
			return(0);
	}

    // FIXME
	pi->script[0] = (char*)pi->scriptstart;

	*(char*)pi->script[0][size+4] = 0;		 			// Last byte is trashed!
	pi->flags |= TPF_CACHED_SCRIPT;
	TP_InitScript(pi);

	return(size);
}

//-------------------------------------------------------------------------
// TP_FreeScript()
//-------------------------------------------------------------------------
void TP_FreeScript(PresenterInfo *pi,Uint16 id_cache)
{
	TP_PurgeAllGfx();

	if (id_cache)
	{
		UNCACHEGRCHUNK(id_cache);
	}
	else
		if ((pi->script) && (pi->flags & TPF_CACHED_SCRIPT)) {
            free(pi->scriptstart);
            pi->scriptstart = NULL;
        }
}

//-------------------------------------------------------------------------
// TP_InitScript()
//-------------------------------------------------------------------------
void TP_InitScript(PresenterInfo *pi)
{
	const char *script = pi->script[0];
    Uint16 code;

	pi->numpages = 1;		// Assume at least 1 page
	while (*script)
	{
		while (stemp=TP_LineCommented(script))
		{
			script += stemp;
			if (!*script)
				goto end_func;
		}

		switch (*script++)
		{
			case TP_CONTROL_CHAR:
#ifndef TP_CASE_SENSITIVE
				*script=toupper(*script);
				*(script+1)=toupper(*(script+1));
#endif
                code = *reinterpret_cast<const Uint16*>(script);
                script += 2;
				switch (code)
				{
					case TP_CNVT_CODE('E','P'):
						if (pi->numpages < TP_MAX_PAGES)
							pi->script[pi->numpages++] = script;
						else
							TP_ERROR(TP_INITSCRIPT_PAGES_OF);
					break;
				}
			break;

#if 0
			case '\r':
				if (*script == '\n')
				{
					*(script-1) = TP_RETURN_CHAR;
					*script = '*';			// This byte should always be skipped!
					script++;
				}
			break;
#endif
		}
	}

end_func:;
	pi->numpages--;	// Last page defined is not a real page.
}

//-------------------------------------------------------------------------
// TP_CacheIn()
//-------------------------------------------------------------------------
void TP_CacheIn(tpCacheType type, Sint16 chunk)
{
	Sint16 first_ch_offset=first_ch-pi->script[0];
	Sint16 loop,offset[TP_MAX_PAGES];

// Cache graphics and re-assign pointers
//
	switch (type)
	{
		case ct_scaled:
			if (flags & fl_pagemanager)
				break;

			flags |= fl_pagemanager;
		break;

		case ct_music:
		case ct_marks:
		case ct_chunk:
			if (flags & fl_pagemanager)
			{
				ClearMemory();
				flags &= ~fl_pagemanager;
			}

			switch (type)
			{
				case ct_chunk:
					CA_CacheGrChunk(chunk);
				break;

				case ct_marks:
					CA_CacheMarks();
				break;

				case ct_music:
					StartCPMusic(chunk);
				break;

				default:
                    break;
			}
		break;
	}

// Re-assign font pointer
//
	font = (fontstruct *)grsegs[STARTFONT+fontnumber];

// Re-assign script pointers IF this is a cached script!
//
	if (pi->flags & TPF_CACHED_SCRIPT)
	{
	// Calc offset of each page pointer
	//
		for (loop=1; loop<pi->numpages; loop++)
			offset[loop] = pi->script[loop]-pi->script[loop-1];

	// Re-assign all page pointers
	//
		if (pi->id_cache != -1)
			pi->scriptstart=grsegs[pi->id_cache];
		pi->script[0] = static_cast<char*>(pi->scriptstart);
		for (loop=1; loop<pi->numpages; loop++)
			pi->script[loop] = pi->script[loop-1]+offset[loop];

	// Reset text pointer
	//
		first_ch = pi->script[0]+first_ch_offset;
	}
}

//-------------------------------------------------------------------------
// TP_LineCommented()
//-------------------------------------------------------------------------
Sint16 TP_LineCommented(const char *s)
{
	const char *o=s;

// If a line starts with a semi-colon, the entire line is considered a
// comment and is ignored!
//
	if ((*s == ';') && (*(s-2) == TP_RETURN_CHAR))
	{
		while (*s != TP_RETURN_CHAR)
			s++;
		s += 2;
	}

	return((char)(s-o));
}

/* Code very similar to this crashed the system during COMPILE.
** The compiler found the error "fptr = sptr + offset" (sptr size unknown),
** but this shitty MDS computer crashed with an exception error after that!!

int MDS_COMPUTERS_SUCK_SHIT()
{
	char far *fptr;
	void _seg *sptr;
	Sint16 offset;

	sptr = 0xa000;
	offset = 1000;
	fptr = sptr + offset;
	offset += 1000;
}

**
**
*/
