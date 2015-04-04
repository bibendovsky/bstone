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


#include "3d_def.h"



//---------------------------------------------------------------------------
//
//									FOOD MACHINE MESSAGES
//
//---------------------------------------------------------------------------

char food_msg1[]="\r FOOD UNIT DISPENSES\r"
							  "  SOMETHING EDIBLE.\r"
							"\r     TOKENS: XX";


//---------------------------------------------------------------------------
//
//								BEVERAGE MACHINE MESSAGES
//
//---------------------------------------------------------------------------


char bevs_msg1[]="\r FOOD UNIT DISPENSES\r"
							  "  A COLD BEVERAGE.\r"
							"\r     TOKENS: XX";


//---------------------------------------------------------------------------
//
//								GENERAL HINT MESSAGES
//
//---------------------------------------------------------------------------

#ifdef CON_HINTS

char genhint_msg1[]="\r\rTERMINALS ACCESS\rALL INFORMATION.";
char genhint_msg2[]="\r\rALL FLOORS ARE ON-LINE.";
char genhint_msg3[]="\r\rSOME SCIENTIST\rARE INFORMANTS.";
char genhint_msg4[]="\r\rELEVATOR CODES ARE\rINFORMATION.";
char genhint_msg5[]="\r\rTOO MUCH CANDY IS\rBAD FOR YOUR TEETH.";
char genhint_msg6[]="\r\rINFORMANTS ARE\rEVERYWHERE!";
char genhint_msg7[]="\r\rINFORMANTS ARE\rINTERACTIVE.";
char genhint_msg8[]="\r\rBEWARE OF EXPERIMENTS!";
char genhint_msg9[]="\r\rBEWARE OF GOLDFIRE!";
char genhint_msg10[]="\r\rTHERE ARE HIDDEN\rROOMS BEHIND PANELS.";
char genhint_msg11[]="\r\rSOME WALLS ARE PANELS.";
char genhint_msg12[]="\r\rORDER ALL 6 MISSIONS\rOF BLAKE STONE TODAY!";
char genhint_msg13[]="\r\rCALL APOGEE AND REGISTER!";
char genhint_msg14[]="\r\rTALK TO SCIENTISTS.\r";
char genhint_msg15[]="\r\rSOME SCIENTISTS\rARE INFORMANTS.";
char genhint_msg16[]="\r\rLEVEL BLUEPRINTS\rARE INFORMATION.";
char genhint_msg17[]="\r\rUSE TRANSPORTERS WHEN\rAVAILABLE.";




char * ConcessionGenHints[NUM_GEN_HINTS]=
{
			genhint_msg1,genhint_msg2,genhint_msg3,genhint_msg4,genhint_msg5,
			genhint_msg6,genhint_msg7,genhint_msg8,genhint_msg9,genhint_msg10,
			genhint_msg11,genhint_msg12,genhint_msg13,genhint_msg14,genhint_msg15,
			genhint_msg16,genhint_msg17,
};

#endif


//---------------------------------------------------------------------------
//
//								NO EAT MESSAGES
//
//---------------------------------------------------------------------------


char noeat_msg1[]="\r\r    CAN'T EAT NOW,"
								"\r     NOT HUNGRY.";



//---------------------------------------------------------------------------
//
//									GENERAL MESSAGES
//
//---------------------------------------------------------------------------


char NoAdLibCard[]="^FC57\r       MUSIC:\r"
								 "^FCA6  YOU DON'T HAVE AN\r"
										"  ADLIB COMPATABLE\r"
										"     SOUND CARD.";



char MusicOn[] =  "^FC57\r\r        MUSIC:\r"
								  "^FCA6   BACKGROUND MUSIC\r"
										 "        IS XXXX";

char SoundOn[] =  "^FC57\r\r       SOUNDS:\r"
								  "^FCA6   SOUND EFFECTS\r"
										 "       ARE XXXX";


char ekg_heartbeat_enabled[] = "\r\r     EKG HEART BEAT\r"
													"     SOUND ENABLED.";
char ekg_heartbeat_disabled[] = "\r\r     EKG HEART BEAT\r"
													 "     SOUND DISABLED.";

char attacker_info_enabled[] = "\r\rDETAILED ATTACKER INFO\r"
													"   DISPLAY ENABLED.";
char attacker_info_disabled[] = "\r\rDETAILED ATTACKER INFO\r"
													"   DISPLAY DISABLED.";

char WeaponNotAvailMsg[]  = "\r\r  SELECTED WEAPON NOT\r"
												"  CURRENTLY AVAILABLE.";

char WeaponAvailMsg[]  = "\r\r   SELECTED WEAPON\r"
											" ACTIVATED AND READY.";


char RadarEnergyGoneMsg[]  = "\r\r  RADAR MAGNIFICATION\r"
												 "    ENERGY DEPLETED.";


char EnergyPackDepleted[] = "^FC19\r       WARNING:\r"
											 "^FC17ENERGY PACK DEPLETED\r"
											 "^FCA6     SWITCHING TO\r"
													"  AUTOCHARGE PISTOL.";

char WeaponMalfunction[] = "^FC19\r       WARNING:\r\r"
											"^FC17 WEAPON MALFUNCTION!\r";

char NotEnoughEnergyForWeapon[] = "^FC17\r  NOT ENOUGH ENERGY\r"
															" FOR SELECTED WEAPON\r"
													 "^FCA6    SWITCHING TO\r"
															"  AUTOCHARGE PISTOL.";


char SwitchNotActivateMsg[]  = "\r\r   WALL SWITCH NOT\r"
													"    OPERATIONAL!!";

char NoFoodTokens[]  = "\r\r  YOU DON'T HAVE ANY\r"
										 "     FOOD TOKENS!";



#ifdef BSTONE_PS
//---------------------------------------------------------------------------
//
//						        FISSION DETONATOR(S) MESSAGES
//
//---------------------------------------------------------------------------


char pd_dropped[]="^FC19\r       WARNING:\r"
								"^FCA6   FISSION DETONATOR\r"
									  "       DROPPED!";


char pd_nomore[]="^FCA6\r\r      NO FISSION\r"
										"   DETONATORS AVAIL.";


char pd_notnear[]=  "^SH035^FCA6\r  YOU MUST\r"
												 "  FIND THE\r"
												 "  SECURITY\r"
												 "    CUBE.";


char pd_getcloser[]=  "^SH035^FCA6\r TRANSPORTER\r"
													" SECURITY OUT\r"
													" OF RANGE";


char pd_floorunlocked[]=  "^SH035^FCA6\r TRANSPORTER\r"
														 "  SECURITY\r"
														 "  DISABLED.";


char pd_donthaveany[]="^SH0E6^FCA6\r NO FISSION\r"
													" DETONATOR\r"
													" AVAILABLE.";


char pd_no_computer[]=  "^SH035^FCA6\r A SECURITY \r"
													  " CUBE IS NOT\r"
													  " LOCATED IN\r"
													  " THIS SECTOR.";


char pd_floornotlocked[] ="^SH035^FCA6\r TRANSPORTER\r"
														 " SECURITY\r"
														 " ALREADY\r"
														 " DISABLED.";
#endif

//---------------------------------------------------------------------------
//
//									BONUS MSGS
//
//---------------------------------------------------------------------------


char bonus_msg1[]="^SH001^FC57\r\r ACCESS CARD:\r"
										  "^FCA6  RED LEVEL";

char bonus_msg2[]="^SH002^FC57\r\r ACCESS CARD:\r"
										  "^FCA6 YELLOW LEVEL";

char bonus_msg4[]="^SH004^FC57\r\r ACCESS CARD:\r"
										  "^FCA6  BLUE LEVEL";

char bonus_msg7[]=   "^SH006^FC57\r   WEAPON:\r"
											"^FCA6 ENERGY PACK\r"
												  "   (  UNITS)";

char bonus_msg8[]="^SH007^FC57\r\r   WEAPON:\r"
										"^FCA6  SLOW FIRE\r"
											  "  PROTECTOR\r";

char bonus_msg9[]="^SH008^FC57\r\r   WEAPON:\r"
										  "^FCA6 RAPID ASSAULT\r"
												 "   WEAPON";

char bonus_msg10[]= "^SH009^FC57\r\r   WEAPON:\r"
										  "^FCA6 DUAL NEUTRON\r"
												 "   DISRUPTER";


char bonus_msg13[]="^SH00C^FC57\r\r    BONUS:\r"
											"^FCA6  MONEY BAG";

char bonus_msg14[]="^SH00D^FC57\r\r    BONUS:\r"
											"^FCA6    LOOT";

char bonus_msg15[]="^SH00E^FC57\r\r    BONUS:\r"
											"^FCA6  GOLD BARS";

char bonus_msg16[]="^SH00F^FC57\r\r    BONUS:\r"
											"^FCA6  XYLAN ORB";


char bonus_msg21[]="^SH08A^FC57\r   WEAPON:\r"
										 "^FCA6   PLASMA\r"
												" DISCHARGE\r"
												"    UNIT";

char bonus_msg21a[]="^SH0E4^FC57\r\r   WEAPON:\r"
											 "^FCA6 ANTI-PLASMA\r"
													"   CANNON";

char bonus_msg24[]="^SH020^FC57\r  FOOD TOKEN:\r"
										 "^FCA6   1 CREDIT\r"
											 "\r  TOKENS: XX";

char bonus_msg25[]="^SH021^FC57\r  FOOD TOKEN:\r"
										 "^FCA6   5 CREDITS"
											 "\r  TOKENS: XX";

char bonus_msg12[]="^SH00B^FC57\r\r   HEALTH:\r"
											"^FCA6 PLASMA BAG";

char bonus_msg11[]="^SH00A^FC57\r\r   HEALTH:\r"
											"^FCA6  FIRST AID\r"
												  "     KIT";

char bonus_msg17[]="^SH010^FC57\r\r    FOOD:\r"
											"^FCA6  RAW MEAT";

char bonus_msg18[]="^SH011^FC57\r\r    FOOD:\r"
											"^FCA6  RAW MEAT";

char bonus_msg23[]="^SH089^FC57\r\r    FOOD:\r"
											"^FCA6  SANDWICH";

char bonus_msg22[]="^SH088^FC57\r\r    FOOD:\r"
											"^FCA6  CANDY BAR";

char bonus_msg19[]="^SH012^FC57\r\r    FOOD:\r"
											"^FCA6 FRESH WATER";

char bonus_msg20[]="^SH013^FC57\r\r    FOOD:\r"
											"^FCA6 WATER PUDDLE";

#ifdef BSTONE_PS
char bonus_msg26[]=   "^SH0D8^FC57   FISSION\r"
												 "  DETONATOR\r\r"
										  "^FCA6PRESS TILDE OR\r"
												 "SPACE TO DROP";
#endif

char bonus_msg27[]=   "^SH0D9^FC57\r   RADAR:  \r"
											 "^FCA6MAGNIFICATION\r"
													"   ENERGY";

char * BonusMsg[]=
{
			bonus_msg1,bonus_msg2,bonus_msg4,
			bonus_msg7,bonus_msg7,bonus_msg8,bonus_msg9,bonus_msg10,bonus_msg21,bonus_msg21a,

			bonus_msg12,bonus_msg11,
			bonus_msg18,bonus_msg17,bonus_msg23,bonus_msg22,bonus_msg19,

			bonus_msg20,
			bonus_msg13,bonus_msg14,bonus_msg15,
			bonus_msg15,bonus_msg15,bonus_msg15,
			bonus_msg16,

			0,0,0,
			bonus_msg24,bonus_msg25,
#ifdef BSTONE_AOG
            0,
#else
			bonus_msg26,
#endif
            bonus_msg27,
};

//---------------------------------------------------------------------------
//
//								ACTOR MSGS (ATTACKING & GEN INFO)
//
//---------------------------------------------------------------------------

//	Sector Patrol
char actor_info4[]="^AN04^FC17\r\r  ATTACKING:\r"
										  "^FCA6 SECTOR GUARD";

//	hang_terrotobj,
char actor_info5[]="^AN05^FC17\r  ATTACKING:\r"
										"^FCA6  AUTOMATED\r"
											  "HEAVY ARMORED\r"
											  " ROBOT TURRET";
//	Bio-Tech
char actor_info9[]="^AN09^FC17\r\r  ATTACKING:\r"
										"^FCA6   BIO-TECH";

//	podobj,
char actor_info10[]="^AN0A^FC17\r\r  ATTACKING:\r"
										 "^FCA6  POD ALIEN";
//	electroobj,
char actor_info11[]="^AN0B^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH ENERGY\r"
												" PLASMA ALIEN";
//	electrosphereobj,
char actor_info12[]="^AN0C^FC17\r\r  ATTACKING:\r"
											"^FCA6PLASMA SPHERE";
//	STAR Sentinel
char actor_info13[]="^AN0D^FC17\r\r  ATTACKING:\r"
											"^FCA6 TECH WARRIOR";
//	genetic_guardobj,
char actor_info14[]="^AN0E^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH-SECURITY\r"
												" GENETIC GUARD";
//	mutant_human1obj,
char actor_info15[]="^AN0F^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" MECH-SENTINEL";

//	mutant_human2obj,
char actor_info16[]="^AN10^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" MUTANT HUMAN";

//	lcan_alienobj,
char actor_info18[]="^AN12^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" GENETIC ALIEN";
//	scan_alienobj,
char actor_info20[]="^AN14^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" GENETIC ALIEN";

//	gurneyobj,
char actor_info22[]="^AN16^FC17\r  ATTACKING:\r"
										 "^FCA6   MUTATED\r"
												"    GUARD";

//	Alien Protector (old STAR Trooper)
char actor_info24[]="^AN18^FC17\r  ATTACKING:\r"
									    "^FCA6    ALIEN\r"
										      "  PROTECTOR";

//	goldsternobj,
char actor_info25[]="^AN19^FC17\r\r  ATTACKING:\r"
											"^FCA6 DR GOLDFIRE";

#ifdef BSTONE_PS
//	gold_morphobj,
char actor_info25m[]="^AN28^FC17\r\r  ATTACKING:\r"
											 "^FCA6   MORPHED\r"
											      " DR GOLDFIRE";
#endif

//	volatiletransportobj,
char actor_info27[]="^SH072^FC17\r  ATTACKING:\r"
										 "^FCA6 VOLATILE MAT.\r"
												"  TRANSPORT\r"
												"  EXPLOSION";
//	floatingbombobj,
char actor_info28[]="^SH076^FC17\r  ATTACKING:\r"
										  "^FCA6PERSCAN DRONE\r"
												 "  EXPLOSION";
//	electroshotobj,
char actor_info31[]="^AN0B^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH ENERGY\r"
												" PLASMA ALIEN";
//	explosionobj,
char actor_info33[]="^SH08B^FC17\r  ATTACKING:\r"
										 "^FCA6  EXPLOSION\r"
												"    BLAST";
//	liquidshotobj,
char actor_info36[]="^AN17^FC17\r\r  ATTACKING:\r"
											"^FCA6 FLUID ALIEN";


char actor_info41[]="^SH000^FC17\r  ATTACKING:\r"
										  "^FCA6 STANDING IN\r"
												 "  BIO TOXIC\r"
												 "    WASTE.";

char actor_info42[]="^SH08C^FC17\r  ATTACKING:\r"
										  "^FCA6 STANDING IN\r"
												 " TOXIC SLUDGE.";


char actor_info41a[]="^SH0E2^FC17\r  ATTACKING:\r"
										   "^FCA6 STANDING IN\r"
												  " TOXIC SLUDGE.";


char actor_info42a[]="^SH0E3^FC17\r  ATTACKING:\r"
										   "^FCA6 STANDING IN\r"
											 	  "  BIO TOXIC\r"
												  "    WASTE.";


char actor_info43[]="^AN1D^FC17\r  ATTACKING:\r"
										 "^FCA6 ELECTRIC ARC\r"
												"   BARRIER.";


char actor_info43a[]="^SH0F4^FC17\r  ATTACKING:\r"
											"^FCA6    POST\r"
												  "   BARRIER.";

char actor_info43b[]="^SH0FC^FC17\r  ATTACKING:\r"
									  	   "^FCA6    SPIKE\r"
											 	  "   BARRIER.";


char actor_info44[]="^AN1e^FC17\r  ATTACKING:\r"
  										 "^FCA6   SPIDER\r"
										      "   MUTANT";

char actor_info45[]="^AN1f^FC17\r  ATTACKING:\r"
									    "^FCA6   BREATHER\r"
												"    BEAST";

char actor_info46[]="^AN20^FC17\r  ATTACKING:\r"
										 "^FCA6   CYBORG\r"
										      "   WARRIOR";

char actor_info47[]="^AN21^FC17\r  ATTACKING:\r"
										 "^FCA6  REPTILIAN\r"
												"   WARRIOR";

char actor_info48[]="^AN22^FC17\r\r  ATTACKING:\r"
										   "^FCA6 ACID DRAGON";

char actor_info49[]="^AN23^FC17\r  ATTACKING:\r"
										 "^FCA6   BIO-MECH\r"
												"   GUARDIAN";

#ifdef BSTONE_PS
char actor_info50[]="^SH07A^FC17\r  ATTACKING:\r"
										  "^FCA6   SECURITY\r"
										   	 "    CUBE\r"
                                     "  EXPLOSION";

//	explosionobj,
char actor_info51[]="^SH08B^FC17\r  ATTACKING:\r"
										  "^FCA6 ANTI-PLASMA\r"
										       "  EXPLOSION\r"
												 "    BLAST";
//	pd_explosionobj,
char actor_info52[]="^SH0E6^FC17\r  ATTACKING:\r"
										  "^FCA6  DETONATOR\r"
										       "  EXPLOSION";
#endif

// Final Boss #1
char actor_info53[]="^AN29^FC17\r  ATTACKING:\r"
										   "^FCA6  THE GIANT\r"
											     "   STALKER";

// Final Boss #2
char actor_info54[]="^AN2A^FC17\r  ATTACKING:\r"
										   "^FCA6 THE SPECTOR\r"
											     "   DEMON";

// Final Boss #3
char actor_info55[]="^AN2b^FC17\r  ATTACKING:\r"
										   "^FCA6 THE ARMORED\r"
                                      "   STALKER";

// Final Boss #4
char actor_info56[]="^AN2c^FC17\r  ATTACKING:\r"
										   "^FCA6 THE CRAWLER\r"
                                      "    BEAST";



char * ActorInfoMsg[]=
{
//			0,0,						// nothing,player
//			0,0,0,					// inert,fixup,dead

			actor_info4,			// Sector Patrol
			actor_info5,			// Turret
			actor_info9,			// Bio-Tech
			actor_info10,        // Pod
			actor_info11,        // Electro-Alien
			actor_info12,        // Electro-Sphere
			actor_info13,        // STAR Sentinel
			actor_info14,        // Genetic Guard
			actor_info15,        // Mutant Human 1
#ifdef BSTONE_AOG
            0,
#else
			actor_info16,        // Mutant Human 2
#endif
			0,							// lg canister wait
			actor_info18,        // Lg Canister Alien
			0,							// sm canister wait
			actor_info20,        // Sm canister Alien
			0,                   // gurney wait
			actor_info22,        // Gurney Mutant
			actor_info36,        // Liquid Alien
			actor_info24,        // Alien Protector (old STAR Trooper)
			actor_info25,        // Goldstern
#ifdef BSTONE_AOG
            0,
#else
			actor_info25m,       // Goldstern Morphed
#endif
			actor_info27,        // Volatile Transport
			actor_info28,        // Floating Bomb

#ifdef BSTONE_AOG
            0,
#else
			actor_info50,			// vital defence
#endif

			actor_info44,			// Spider Mutant
			actor_info45,			// breather beast
			actor_info46,			// cyborg warrior

			actor_info47,			// reptilian warrior
			actor_info48,			// acid dragon
			actor_info49,			// mech guardian

         actor_info53,			// Final Boss 1
			actor_info54,			// Final Boss 2
			actor_info55,			// Final Boss 3
			actor_info56,			// Final Boss 4

			0,0,0,0,					// blake,crate 1, crate 2, crate 3,

			actor_info41,        // Green Ooze
			actor_info42,        // Black Ooze
			actor_info41a,       // Green2 Ooze
			actor_info42a,       // Black2 Ooze
			0,							// Pod Egg

			actor_info44,			// morphing_spider mutant
			actor_info47,			// morphing_reptilian warrior
			actor_info16,        // morphing_Mutant Human 2

			0,                   // SPACER

			actor_info31,        // Electro-Alien SHOT
			0,                   // Post Barrier
			actor_info43,        // Arc Barrier
			actor_info43a,       // VPost Barrier
			actor_info43b,       // VSpike Barrier

#ifdef BSTONE_AOG
            0,
#else
         actor_info25m,			// Gold Morph Shot obj
#endif

			0,                   // Security Light
			actor_info33,        // Explosion
			0, 0,                // Steam Grate, Steam Pipe
			actor_info36,        // Liquid SHOT

			actor_info18,        // Lg Canister Alien SHOT
			actor_info10,        // POD Alien SHOT
			actor_info20,        // Genetic Alien SHOT
			actor_info16,        // Mutant Human 2 SHOT
			actor_info15,        // Mutant Human 1 SHOT

			0,0,						// vent drip, player sp shot,
			0,							// flicker light,
			0,0,						// Door Bomb, Door Bomb reserve
			0,							// grenade,

         0,							// BFG Shot

#ifdef BSTONE_AOG
         0,
         0,
#else
			actor_info51,        // BFG Explosion
			actor_info52,        // BFG Explosion
#endif

			actor_info44,			// Boss 1 SHOT
			actor_info45,			// Boss 2 SHOT
			actor_info46,			// Boss 3 SHOT
			actor_info47,			// Boss 4 SHOT
			actor_info48,			// Boss 5 SHOT
			actor_info49,			// Boss 6 SHOT
			actor_info54,			// Boss 8 SHOT
			actor_info56,			// Boss 10 SHOT

         0,							// Doorexplosion

#ifdef BSTONE_AOG
         0,
#else
         actor_info52,			// gr_explosion
#endif
};











