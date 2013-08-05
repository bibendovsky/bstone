#include "3d_def.h"



//---------------------------------------------------------------------------
//
//									FOOD MACHINE MESSAGES
//
//---------------------------------------------------------------------------

char far food_msg1[]="\r FOOD UNIT DISPENSES\r"
							  "  SOMETHING EDIBLE.\r"
							"\r     TOKENS: XX";


//---------------------------------------------------------------------------
//
//								BEVERAGE MACHINE MESSAGES
//
//---------------------------------------------------------------------------


char far bevs_msg1[]="\r FOOD UNIT DISPENSES\r"
							  "  A COLD BEVERAGE.\r"
							"\r     TOKENS: XX";


//---------------------------------------------------------------------------
//
//								GENERAL HINT MESSAGES
//
//---------------------------------------------------------------------------

#ifdef CON_HINTS

char far genhint_msg1[]="\r\rTERMINALS ACCESS\rALL INFORMATION.";
char far genhint_msg2[]="\r\rALL FLOORS ARE ON-LINE.";
char far genhint_msg3[]="\r\rSOME SCIENTIST\rARE INFORMANTS.";
char far genhint_msg4[]="\r\rELEVATOR CODES ARE\rINFORMATION.";
char far genhint_msg5[]="\r\rTOO MUCH CANDY IS\rBAD FOR YOUR TEETH.";
char far genhint_msg6[]="\r\rINFORMANTS ARE\rEVERYWHERE!";
char far genhint_msg7[]="\r\rINFORMANTS ARE\rINTERACTIVE.";
char far genhint_msg8[]="\r\rBEWARE OF EXPERIMENTS!";
char far genhint_msg9[]="\r\rBEWARE OF GOLDFIRE!";
char far genhint_msg10[]="\r\rTHERE ARE HIDDEN\rROOMS BEHIND PANELS.";
char far genhint_msg11[]="\r\rSOME WALLS ARE PANELS.";
char far genhint_msg12[]="\r\rORDER ALL 6 MISSIONS\rOF BLAKE STONE TODAY!";
char far genhint_msg13[]="\r\rCALL APOGEE AND REGISTER!";
char far genhint_msg14[]="\r\rTALK TO SCIENTISTS.\r";
char far genhint_msg15[]="\r\rSOME SCIENTISTS\rARE INFORMANTS.";
char far genhint_msg16[]="\r\rLEVEL BLUEPRINTS\rARE INFORMATION.";
char far genhint_msg17[]="\r\rUSE TRANSPORTERS WHEN\rAVAILABLE.";




char far * far ConcessionGenHints[NUM_GEN_HINTS]=
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


char far noeat_msg1[]="\r\r    CAN'T EAT NOW,"
								"\r     NOT HUNGRY.";



//---------------------------------------------------------------------------
//
//									GENERAL MESSAGES
//
//---------------------------------------------------------------------------


char far NoAdLibCard[]="^FC57\r       MUSIC:\r"
								 "^FCA6  YOU DON'T HAVE AN\r"
										"  ADLIB COMPATABLE\r"
										"     SOUND CARD.";



char far MusicOn[] =  "^FC57\r\r        MUSIC:\r"
								  "^FCA6   BACKGROUND MUSIC\r"
										 "        IS XXXX";

char far SoundOn[] =  "^FC57\r\r       SOUNDS:\r"
								  "^FCA6   SOUND EFFECTS\r"
										 "       ARE XXXX";


char far ekg_heartbeat_enabled[] = "\r\r     EKG HEART BEAT\r"
													"     SOUND ENABLED.";
char far ekg_heartbeat_disabled[] = "\r\r     EKG HEART BEAT\r"
													 "     SOUND DISABLED.";

char far attacker_info_enabled[] = "\r\rDETAILED ATTACKER INFO\r"
													"   DISPLAY ENABLED.";
char far attacker_info_disabled[] = "\r\rDETAILED ATTACKER INFO\r"
													"   DISPLAY DISABLED.";

char far WeaponNotAvailMsg[]  = "\r\r  SELECTED WEAPON NOT\r"
												"  CURRENTLY AVAILABLE.";

char far WeaponAvailMsg[]  = "\r\r   SELECTED WEAPON\r"
											" ACTIVATED AND READY.";


char far RadarEnergyGoneMsg[]  = "\r\r  RADAR MAGNIFICATION\r"
												 "    ENERGY DEPLETED.";


char far EnergyPackDepleted[] = "^FC19\r       WARNING:\r"
											 "^FC17ENERGY PACK DEPLETED\r"
											 "^FCA6     SWITCHING TO\r"
													"  AUTOCHARGE PISTOL.";

char far WeaponMalfunction[] = "^FC19\r       WARNING:\r\r"
											"^FC17 WEAPON MALFUNCTION!\r";

char far NotEnoughEnergyForWeapon[] = "^FC17\r  NOT ENOUGH ENERGY\r"
															" FOR SELECTED WEAPON\r"
													 "^FCA6    SWITCHING TO\r"
															"  AUTOCHARGE PISTOL.";


char far SwitchNotActivateMsg[]  = "\r\r   WALL SWITCH NOT\r"
													"    OPERATIONAL!!";

char far NoFoodTokens[]  = "\r\r  YOU DON'T HAVE ANY\r"
										 "     FOOD TOKENS!";




//---------------------------------------------------------------------------
//
//						        FISSION DETONATOR(S) MESSAGES
//
//---------------------------------------------------------------------------


char far pd_dropped[]="^FC19\r       WARNING:\r"
								"^FCA6   FISSION DETONATOR\r"
									  "       DROPPED!";


char far pd_nomore[]="^FCA6\r\r      NO FISSION\r"
										"   DETONATORS AVAIL.";


char far pd_notnear[]=  "^SH035^FCA6\r  YOU MUST\r"
												 "  FIND THE\r"
												 "  SECURITY\r"
												 "    CUBE.";


char far pd_getcloser[]=  "^SH035^FCA6\r TRANSPORTER\r"
													" SECURITY OUT\r"
													" OF RANGE";


char far pd_floorunlocked[]=  "^SH035^FCA6\r TRANSPORTER\r"
														 "  SECURITY\r"
														 "  DISABLED.";


char far pd_donthaveany[]="^SH0E6^FCA6\r NO FISSION\r"
													" DETONATOR\r"
													" AVAILABLE.";


char far pd_no_computer[]=  "^SH035^FCA6\r A SECURITY \r"
													  " CUBE IS NOT\r"
													  " LOCATED IN\r"
													  " THIS SECTOR.";


char far pd_floornotlocked[] ="^SH035^FCA6\r TRANSPORTER\r"
														 " SECURITY\r"
														 " ALREADY\r"
														 " DISABLED.";


//---------------------------------------------------------------------------
//
//									BONUS MSGS
//
//---------------------------------------------------------------------------


char far bonus_msg1[]="^SH001^FC57\r\r ACCESS CARD:\r"
										  "^FCA6  RED LEVEL";

char far bonus_msg2[]="^SH002^FC57\r\r ACCESS CARD:\r"
										  "^FCA6 YELLOW LEVEL";

char far bonus_msg4[]="^SH004^FC57\r\r ACCESS CARD:\r"
										  "^FCA6  BLUE LEVEL";

char far bonus_msg7[]=   "^SH006^FC57\r   WEAPON:\r"
											"^FCA6 ENERGY PACK\r"
												  "   (  UNITS)";

char far bonus_msg8[]="^SH007^FC57\r\r   WEAPON:\r"
										"^FCA6  SLOW FIRE\r"
											  "  PROTECTOR\r";

char far bonus_msg9[]="^SH008^FC57\r\r   WEAPON:\r"
										  "^FCA6 RAPID ASSAULT\r"
												 "   WEAPON";

char far bonus_msg10[]= "^SH009^FC57\r\r   WEAPON:\r"
										  "^FCA6 DUAL NEUTRON\r"
												 "   DISRUPTER";


char far bonus_msg13[]="^SH00C^FC57\r\r    BONUS:\r"
											"^FCA6  MONEY BAG";

char far bonus_msg14[]="^SH00D^FC57\r\r    BONUS:\r"
											"^FCA6    LOOT";

char far bonus_msg15[]="^SH00E^FC57\r\r    BONUS:\r"
											"^FCA6  GOLD BARS";

char far bonus_msg16[]="^SH00F^FC57\r\r    BONUS:\r"
											"^FCA6  XYLAN ORB";


char far bonus_msg21[]="^SH08A^FC57\r   WEAPON:\r"
										 "^FCA6   PLASMA\r"
												" DISCHARGE\r"
												"    UNIT";

char far bonus_msg21a[]="^SH0E4^FC57\r\r   WEAPON:\r"
											 "^FCA6 ANTI-PLASMA\r"
													"   CANNON";

char far bonus_msg24[]="^SH020^FC57\r  FOOD TOKEN:\r"
										 "^FCA6   1 CREDIT\r"
											 "\r  TOKENS: XX";

char far bonus_msg25[]="^SH021^FC57\r  FOOD TOKEN:\r"
										 "^FCA6   5 CREDITS"
											 "\r  TOKENS: XX";

char far bonus_msg12[]="^SH00B^FC57\r\r   HEALTH:\r"
											"^FCA6 PLASMA BAG";

char far bonus_msg11[]="^SH00A^FC57\r\r   HEALTH:\r"
											"^FCA6  FIRST AID\r"
												  "     KIT";

char far bonus_msg17[]="^SH010^FC57\r\r    FOOD:\r"
											"^FCA6  RAW MEAT";

char far bonus_msg18[]="^SH011^FC57\r\r    FOOD:\r"
											"^FCA6  RAW MEAT";

char far bonus_msg23[]="^SH089^FC57\r\r    FOOD:\r"
											"^FCA6  SANDWICH";

char far bonus_msg22[]="^SH088^FC57\r\r    FOOD:\r"
											"^FCA6  CANDY BAR";

char far bonus_msg19[]="^SH012^FC57\r\r    FOOD:\r"
											"^FCA6 FRESH WATER";

char far bonus_msg20[]="^SH013^FC57\r\r    FOOD:\r"
											"^FCA6 WATER PUDDLE";

char far bonus_msg26[]=   "^SH0D8^FC57   FISSION\r"
												 "  DETONATOR\r\r"
										  "^FCA6PRESS TILDE OR\r"
												 "SPACE TO DROP";

char far bonus_msg27[]=   "^SH0D9^FC57\r   RADAR:  \r"
											 "^FCA6MAGNIFICATION\r"
													"   ENERGY";

char far * far BonusMsg[]=
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
			bonus_msg26,bonus_msg27,
};

//---------------------------------------------------------------------------
//
//								ACTOR MSGS (ATTACKING & GEN INFO)
//
//---------------------------------------------------------------------------

//	Sector Patrol
char far actor_info4[]="^AN04^FC17\r\r  ATTACKING:\r"
										  "^FCA6 SECTOR GUARD";

//	hang_terrotobj,
char far actor_info5[]="^AN05^FC17\r  ATTACKING:\r"
										"^FCA6  AUTOMATED\r"
											  "HEAVY ARMORED\r"
											  " ROBOT TURRET";
//	Bio-Tech
char far actor_info9[]="^AN09^FC17\r\r  ATTACKING:\r"
										"^FCA6   BIO-TECH";

//	podobj,
char far actor_info10[]="^AN0A^FC17\r\r  ATTACKING:\r"
										 "^FCA6  POD ALIEN";
//	electroobj,
char far actor_info11[]="^AN0B^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH ENERGY\r"
												" PLASMA ALIEN";
//	electrosphereobj,
char far actor_info12[]="^AN0C^FC17\r\r  ATTACKING:\r"
											"^FCA6PLASMA SPHERE";
//	STAR Sentinel
char far actor_info13[]="^AN0D^FC17\r\r  ATTACKING:\r"
											"^FCA6 TECH WARRIOR";
//	genetic_guardobj,
char far actor_info14[]="^AN0E^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH-SECURITY\r"
												" GENETIC GUARD";
//	mutant_human1obj,
char far actor_info15[]="^AN0F^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" MECH-SENTINEL";
//	mutant_human2obj,
char far actor_info16[]="^AN10^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" MUTANT HUMAN";

//	lcan_alienobj,
char far actor_info18[]="^AN12^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" GENETIC ALIEN";
//	scan_alienobj,
char far actor_info20[]="^AN14^FC17\r  ATTACKING:\r"
										 "^FCA6 EXPERIMENTAL\r"
												" GENETIC ALIEN";

//	gurneyobj,
char far actor_info22[]="^AN16^FC17\r  ATTACKING:\r"
										 "^FCA6   MUTATED\r"
												"    GUARD";

//	Alien Protector (old STAR Trooper)
char far actor_info24[]="^AN18^FC17\r  ATTACKING:\r"
									    "^FCA6    ALIEN\r"
										      "  PROTECTOR";

//	goldsternobj,
char far actor_info25[]="^AN19^FC17\r\r  ATTACKING:\r"
											"^FCA6 DR GOLDFIRE";

//	gold_morphobj,
char far actor_info25m[]="^AN28^FC17\r\r  ATTACKING:\r"
											 "^FCA6   MORPHED\r"
											      " DR GOLDFIRE";

//	volatiletransportobj,
char far actor_info27[]="^SH072^FC17\r  ATTACKING:\r"
										 "^FCA6 VOLATILE MAT.\r"
												"  TRANSPORT\r"
												"  EXPLOSION";
//	floatingbombobj,
char far actor_info28[]="^SH076^FC17\r  ATTACKING:\r"
										  "^FCA6PERSCAN DRONE\r"
												 "  EXPLOSION";
//	electroshotobj,
char far actor_info31[]="^AN0B^FC17\r  ATTACKING:\r"
										 "^FCA6 HIGH ENERGY\r"
												" PLASMA ALIEN";
//	explosionobj,
char far actor_info33[]="^SH08B^FC17\r  ATTACKING:\r"
										 "^FCA6  EXPLOSION\r"
												"    BLAST";
//	liquidshotobj,
char far actor_info36[]="^AN17^FC17\r\r  ATTACKING:\r"
											"^FCA6 FLUID ALIEN";


char far actor_info41[]="^SH000^FC17\r  ATTACKING:\r"
										  "^FCA6 STANDING IN\r"
												 "  BIO TOXIC\r"
												 "    WASTE.";

char far actor_info42[]="^SH08C^FC17\r  ATTACKING:\r"
										  "^FCA6 STANDING IN\r"
												 " TOXIC SLUDGE.";


char far actor_info41a[]="^SH0E2^FC17\r  ATTACKING:\r"
										   "^FCA6 STANDING IN\r"
												  " TOXIC SLUDGE.";


char far actor_info42a[]="^SH0E3^FC17\r  ATTACKING:\r"
										   "^FCA6 STANDING IN\r"
											 	  "  BIO TOXIC\r"
												  "    WASTE.";


char far actor_info43[]="^AN1D^FC17\r  ATTACKING:\r"
										 "^FCA6 ELECTRIC ARC\r"
												"   BARRIER.";


char far actor_info43a[]="^SH0F4^FC17\r  ATTACKING:\r"
											"^FCA6    POST\r"
												  "   BARRIER.";

char far actor_info43b[]="^SH0FC^FC17\r  ATTACKING:\r"
									  	   "^FCA6    SPIKE\r"
											 	  "   BARRIER.";


char far actor_info44[]="^AN1e^FC17\r  ATTACKING:\r"
  										 "^FCA6   SPIDER\r"
										      "   MUTANT";

char far actor_info45[]="^AN1f^FC17\r  ATTACKING:\r"
									    "^FCA6   BREATHER\r"
												"    BEAST";

char far actor_info46[]="^AN20^FC17\r  ATTACKING:\r"
										 "^FCA6   CYBORG\r"
										      "   WARRIOR";

char far actor_info47[]="^AN21^FC17\r  ATTACKING:\r"
										 "^FCA6  REPTILIAN\r"
												"   WARRIOR";

char far actor_info48[]="^AN22^FC17\r\r  ATTACKING:\r"
										   "^FCA6 ACID DRAGON";

char far actor_info49[]="^AN23^FC17\r  ATTACKING:\r"
										 "^FCA6   BIO-MECH\r"
												"   GUARDIAN";


char far actor_info50[]="^SH07A^FC17\r  ATTACKING:\r"
										  "^FCA6   SECURITY\r"
										   	 "    CUBE\r"
                                     "  EXPLOSION";

//	explosionobj,
char far actor_info51[]="^SH08B^FC17\r  ATTACKING:\r"
										  "^FCA6 ANTI-PLASMA\r"
										       "  EXPLOSION\r"
												 "    BLAST";
//	pd_explosionobj,
char far actor_info52[]="^SH0E6^FC17\r  ATTACKING:\r"
										  "^FCA6  DETONATOR\r"
										       "  EXPLOSION";

// Final Boss #1
char far actor_info53[]="^AN29^FC17\r  ATTACKING:\r"
										   "^FCA6  THE GIANT\r"
											     "   STALKER";

// Final Boss #2
char far actor_info54[]="^AN2A^FC17\r  ATTACKING:\r"
										   "^FCA6 THE SPECTOR\r"
											     "   DEMON";

// Final Boss #3
char far actor_info55[]="^AN2b^FC17\r  ATTACKING:\r"
										   "^FCA6 THE ARMORED\r"
                                      "   STALKER";

// Final Boss #4
char far actor_info56[]="^AN2c^FC17\r  ATTACKING:\r"
										   "^FCA6 THE CRAWLER\r"
                                      "    BEAST";




char far * far ActorInfoMsg[]=
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
			actor_info16,        // Mutant Human 2
			0,							// lg canister wait
			actor_info18,        // Lg Canister Alien
			0,							// sm canister wait
			actor_info20,        // Sm canister Alien
			0,                   // gurney wait
			actor_info22,        // Gurney Mutant
			actor_info36,        // Liquid Alien
			actor_info24,        // Alien Protector (old STAR Trooper)
			actor_info25,        // Goldstern
			actor_info25m,       // Goldstern Morphed
			actor_info27,        // Volatile Transport
			actor_info28,        // Floating Bomb

			actor_info50,			// vital defence

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
         actor_info25m,			// Gold Morph Shot obj
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
			actor_info51,        // BFG Explosion
			actor_info52,        // BFG Explosion

			actor_info44,			// Boss 1 SHOT
			actor_info45,			// Boss 2 SHOT
			actor_info46,			// Boss 3 SHOT
			actor_info47,			// Boss 4 SHOT
			actor_info48,			// Boss 5 SHOT
			actor_info49,			// Boss 6 SHOT
			actor_info54,			// Boss 8 SHOT
			actor_info56,			// Boss 10 SHOT

         0,							// Doorexplosion
         actor_info52,			// gr_explosion
};











