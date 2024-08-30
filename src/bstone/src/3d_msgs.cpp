/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "id_ca.h"


static std::string empty_msg;


// FOOD MACHINE MESSAGES
std::string food_msg1;

// BEVERAGE MACHINE MESSAGES
std::string bevs_msg1;

// NO EAT MESSAGES
std::string noeat_msg1 =
"\r\r    CAN'T EAT NOW,"
"\r     NOT HUNGRY."
;

std::string NoAdLibCard =
"^FC57\r       MUSIC:\r"
"^FCA6  YOU DON'T HAVE AN\r"
"  ADLIB COMPATIBLE\r"
"     SOUND CARD."
;

std::string MusicOn =
"^FC57\r\r        MUSIC:\r"
"^FCA6   BACKGROUND MUSIC\r"
"        IS ON."
;

std::string MusicOff =
"^FC57\r\r        MUSIC:\r"
"^FCA6   BACKGROUND MUSIC\r"
"        IS OFF."
;

std::string SoundOn =
"^FC57\r\r       SOUNDS:\r"
"^FCA6   SOUND EFFECTS\r"
"       ARE ON."
;

std::string SoundOff =
"^FC57\r\r       SOUNDS:\r"
"^FCA6   SOUND EFFECTS\r"
"       ARE OFF."
;

std::string ekg_heartbeat_enabled =
"\r\r     EKG HEART BEAT\r"
"     SOUND ENABLED."
;

std::string ekg_heartbeat_disabled =
"\r\r     EKG HEART BEAT\r"
"     SOUND DISABLED."
;

std::string attacker_info_enabled =
"\r\rDETAILED ATTACKER INFO\r"
"   DISPLAY ENABLED."
;

std::string attacker_info_disabled =
"\r\rDETAILED ATTACKER INFO\r"
"   DISPLAY DISABLED."
;

std::string WeaponNotAvailMsg =
"\r\r  SELECTED WEAPON NOT\r"
"  CURRENTLY AVAILABLE."
;

std::string WeaponAvailMsg =
"\r\r   SELECTED WEAPON\r"
" ACTIVATED AND READY."
;

std::string RadarEnergyGoneMsg =
"\r\r  RADAR MAGNIFICATION\r"
"    ENERGY DEPLETED."
;

std::string EnergyPackDepleted =
"^FC19\r       WARNING:\r"
"^FC17ENERGY PACK DEPLETED\r"
"^FCA6     SWITCHING TO\r"
"  AUTOCHARGE PISTOL."
;

std::string WeaponMalfunction =
"^FC19\r       WARNING:\r\r"
"^FC17 WEAPON MALFUNCTION!\r"
;

std::string NotEnoughEnergyForWeapon =
"^FC17\r  NOT ENOUGH ENERGY\r"
" FOR SELECTED WEAPON\r"
"^FCA6    SWITCHING TO\r"
"  AUTOCHARGE PISTOL."
;

std::string SwitchNotActivateMsg =
"\r\r   WALL SWITCH NOT\r"
"    OPERATIONAL!!"
;

std::string NoFoodTokens =
"\r\r  YOU DON'T HAVE ANY\r"
"     FOOD TOKENS!"
;


// ---------------------------------------------------------------------------
//
// FISSION DETONATOR(S) MESSAGES
//
// ---------------------------------------------------------------------------

std::string pd_dropped =
"^FC19\r       WARNING:\r"
"^FCA6   FISSION DETONATOR\r"
"       DROPPED!"
;

std::string pd_nomore =
"^FCA6\r\r      NO FISSION\r"
"   DETONATORS AVAIL."
;

std::string pd_notnear =
"^SH035^FCA6\r  YOU MUST\r"
"  FIND THE\r"
"  SECURITY\r"
"    CUBE."
;

std::string pd_getcloser =
"^SH035^FCA6\r TRANSPORTER\r"
" SECURITY OUT\r"
" OF RANGE"
;

std::string pd_floorunlocked =
"^SH035^FCA6\r TRANSPORTER\r"
"  SECURITY\r"
"  DISABLED."
;

std::string pd_donthaveany =
"^SH0E6^FCA6\r NO FISSION\r"
" DETONATOR\r"
" AVAILABLE."
;

std::string pd_no_computer =
"^SH035^FCA6\r A SECURITY \r"
" CUBE IS NOT\r"
" LOCATED IN\r"
" THIS SECTOR."
;

std::string pd_floornotlocked =
"^SH035^FCA6\r TRANSPORTER\r"
" SECURITY\r"
" ALREADY\r"
" DISABLED."
;


// ---------------------------------------------------------------------------
//
// BONUS MSGS
//
// ---------------------------------------------------------------------------

std::string bonus_msg1 =
"^SH001^FC57\r\r ACCESS CARD:\r"
"^FCA6  RED LEVEL"
;

std::string bonus_msg2 =
"^SH002^FC57\r\r ACCESS CARD:\r"
"^FCA6 YELLOW LEVEL"
;

std::string bonus_msg4 =
"^SH004^FC57\r\r ACCESS CARD:\r"
"^FCA6  BLUE LEVEL"
;

std::string bonus_msg4a =
"^SH003^FC57\r"
"\r"
" ACCESS CARD:\r"
"^FCA6 GREEN LEVEL"
;

std::string bonus_msg4b =
"^SH005^FC57\r"
"\r"
" ACCESS CARD:\r"
"^FCA6  GOLD LEVEL"
;

std::string bonus_msg7 =
"^SH006^FC57\r   WEAPON:\r"
"^FCA6 ENERGY PACK\r"
"   (  UNITS)"
;

std::string bonus_msg8 =
"^SH007^FC57\r\r   WEAPON:\r"
"^FCA6  SLOW FIRE\r"
"  PROTECTOR\r"
;

std::string bonus_msg9 =
"^SH008^FC57\r\r   WEAPON:\r"
"^FCA6 RAPID ASSAULT\r"
"   WEAPON"
;

std::string bonus_msg10 =
"^SH009^FC57\r\r   WEAPON:\r"
"^FCA6 DUAL NEUTRON\r"
"   DISRUPTER"
;

std::string bonus_msg13 =
"^SH00C^FC57\r\r    BONUS:\r"
"^FCA6  MONEY BAG"
;

std::string bonus_msg14 =
"^SH00D^FC57\r\r    BONUS:\r"
"^FCA6    LOOT"
;

std::string bonus_msg15 =
"^SH00E^FC57\r\r    BONUS:\r"
"^FCA6  GOLD BARS"
;

std::string bonus_msg16 =
"^SH00F^FC57\r\r    BONUS:\r"
"^FCA6  XYLAN ORB"
;

std::string bonus_msg21 =
"^SH08A^FC57\r   WEAPON:\r"
"^FCA6   PLASMA\r"
" DISCHARGE\r"
"    UNIT"
;

std::string bonus_msg21a =
"^SH0E4^FC57\r\r   WEAPON:\r"
"^FCA6 ANTI-PLASMA\r"
"   CANNON"
;

std::string bonus_msg24 =
"^SH020^FC57\r  FOOD TOKEN:\r"
"^FCA6   1 CREDIT\r"
"\r  TOKENS: XX"
;

std::string bonus_msg25 =
"^SH021^FC57\r  FOOD TOKEN:\r"
"^FCA6   5 CREDITS"
"\r  TOKENS: XX"
;

std::string bonus_msg12 =
"^SH00B^FC57\r\r   HEALTH:\r"
"^FCA6 PLASMA BAG"
;

std::string bonus_msg11 =
"^SH00A^FC57\r\r   HEALTH:\r"
"^FCA6  FIRST AID\r"
"     KIT"
;

std::string bonus_msg17 =
"^SH010^FC57\r\r    FOOD:\r"
"^FCA6  RAW MEAT"
;

std::string bonus_msg18 =
"^SH011^FC57\r\r    FOOD:\r"
"^FCA6  RAW MEAT"
;

std::string bonus_msg23 =
"^SH089^FC57\r\r    FOOD:\r"
"^FCA6  SANDWICH"
;

std::string bonus_msg22 =
"^SH088^FC57\r\r    FOOD:\r"
"^FCA6  CANDY BAR"
;

std::string bonus_msg19 =
"^SH012^FC57\r\r    FOOD:\r"
"^FCA6 FRESH WATER"
;

std::string bonus_msg20 =
"^SH013^FC57\r\r    FOOD:\r"
"^FCA6 WATER PUDDLE"
;

std::string bonus_msg26 =
"^SH0D8^FC57   FISSION\r"
"  DETONATOR\r\r"
"^FCA6PRESS TILDE OR\r"
"SPACE TO DROP"
;

std::string bonus_msg27 =
"^SH0D9^FC57\r   RADAR:  \r"
"^FCA6MAGNIFICATION\r"
"   ENERGY"
;


BonusMessages BonusMsg =
{
	bonus_msg1,
	bonus_msg2,
	bonus_msg4,
	bonus_msg7,
	bonus_msg7,
	bonus_msg8,
	bonus_msg9,
	bonus_msg10,
	bonus_msg21,
	bonus_msg21a,
	bonus_msg12,
	bonus_msg11,
	bonus_msg18,
	bonus_msg17,
	bonus_msg23,
	bonus_msg22,
	bonus_msg19,
	bonus_msg20,
	bonus_msg13,
	bonus_msg14,
	bonus_msg15,
	bonus_msg15,
	bonus_msg15,
	bonus_msg15,
	bonus_msg16,
	empty_msg,
	empty_msg,
	empty_msg,
	bonus_msg24,
	bonus_msg25,
	bonus_msg26,
	bonus_msg27,
	empty_msg,
	empty_msg,
	bonus_msg4a,
	bonus_msg4b,
}; // BonusMsg


// ---------------------------------------------------------------------------
//
// ACTOR MSGS (ATTACKING & GEN INFO)
//
// ---------------------------------------------------------------------------

// Sector Patrol (AOG) / Sector Guard (PS)
std::string actor_info4;

// hang_terrotobj
std::string actor_info5 =
"^AN05^FC17\r  ATTACKING:\r"
"^FCA6  AUTOMATED\r"
"HEAVY ARMORED\r"
" ROBOT TURRET"
;

// Bio-Tech
std::string actor_info9 =
"^AN09^FC17\r\r  ATTACKING:\r"
"^FCA6   BIO-TECH"
;

// podobj
std::string actor_info10 =
"^AN0A^FC17\r\r  ATTACKING:\r"
"^FCA6  POD ALIEN"
;

// electroobj
std::string actor_info11 =
"^AN0B^FC17\r  ATTACKING:\r"
"^FCA6 HIGH ENERGY\r"
" PLASMA ALIEN"
;

// electrosphereobj
std::string actor_info12 =
"^AN0C^FC17\r\r  ATTACKING:\r"
"^FCA6PLASMA SPHERE"
;

// STAR Sentinel (AOG) / Tech Warrior (PS)
std::string actor_info13;

// genetic_guardobj
std::string actor_info14 =
"^AN0E^FC17\r  ATTACKING:\r"
"^FCA6 HIGH-SECURITY\r"
" GENETIC GUARD"
;

// mutant_human1obj
std::string actor_info15 =
"^AN0F^FC17\r  ATTACKING:\r"
"^FCA6 EXPERIMENTAL\r"
" MECH-SENTINEL"
;

// mutant_human2obj
std::string actor_info16 =
"^AN10^FC17\r  ATTACKING:\r"
"^FCA6 EXPERIMENTAL\r"
" MUTANT HUMAN"
;

// lcan_alienobj
std::string actor_info18 =
"^AN12^FC17\r  ATTACKING:\r"
"^FCA6 EXPERIMENTAL\r"
" GENETIC ALIEN"
;

// scan_alienobj
std::string actor_info20 =
"^AN14^FC17\r  ATTACKING:\r"
"^FCA6 EXPERIMENTAL\r"
" GENETIC ALIEN"
;

// gurneyobj
std::string actor_info22 =
"^AN16^FC17\r  ATTACKING:\r"
"^FCA6   MUTATED\r"
"    GUARD"
;

// Star Trooper (AOG) / Alien Protector (PS)
std::string actor_info24;

// goldsternobj
std::string actor_info25 =
"^AN19^FC17\r\r  ATTACKING:\r"
"^FCA6 DR GOLDFIRE"
;

// gold_morphobj
std::string actor_info25m =
"^AN28^FC17\r\r  ATTACKING:\r"
"^FCA6   MORPHED\r"
" DR GOLDFIRE"
;

// volatiletransportobj
std::string actor_info27 =
"^SH072^FC17\r  ATTACKING:\r"
"^FCA6 VOLATILE MAT.\r"
"  TRANSPORT\r"
"  EXPLOSION"
;

// floatingbombobj
std::string actor_info28 =
"^SH076^FC17\r  ATTACKING:\r"
"^FCA6PERSCAN DRONE\r"
"  EXPLOSION"
;

// electroshotobj
std::string actor_info31 =
"^AN0B^FC17\r  ATTACKING:\r"
"^FCA6 HIGH ENERGY\r"
" PLASMA ALIEN"
;

// explosionobj
std::string actor_info33 =
"^SH08B^FC17\r  ATTACKING:\r"
"^FCA6  EXPLOSION\r"
"    BLAST"
;

// liquidshotobj
std::string actor_info36 =
"^AN17^FC17\r\r  ATTACKING:\r"
"^FCA6 FLUID ALIEN"
;

std::string actor_info41 =
"^SH000^FC17\r  ATTACKING:\r"
"^FCA6 STANDING IN\r"
"  BIO TOXIC\r"
"    WASTE."
;

std::string actor_info42 =
"^SH08C^FC17\r  ATTACKING:\r"
"^FCA6 STANDING IN\r"
" TOXIC SLUDGE."
;

std::string actor_info41a =
"^SH0E2^FC17\r  ATTACKING:\r"
"^FCA6 STANDING IN\r"
" TOXIC SLUDGE."
;

std::string actor_info42a =
"^SH0E3^FC17\r  ATTACKING:\r"
"^FCA6 STANDING IN\r"
"  BIO TOXIC\r"
"    WASTE."
;

std::string actor_info43 =
"^AN1D^FC17\r  ATTACKING:\r"
"^FCA6 ELECTRIC ARC\r"
"   BARRIER."
;

std::string actor_info43a =
"^SH0F4^FC17\r  ATTACKING:\r"
"^FCA6    POST\r"
"   BARRIER."
;

std::string actor_info43b =
"^SH0FC^FC17\r  ATTACKING:\r"
"^FCA6    SPIKE\r"
"   BARRIER."
;

std::string actor_info44 =
"^AN1e^FC17\r  ATTACKING:\r"
"^FCA6   SPIDER\r"
"   MUTANT"
;

std::string actor_info45 =
"^AN1f^FC17\r  ATTACKING:\r"
"^FCA6   BREATHER\r"
"    BEAST"
;

std::string actor_info46 =
"^AN20^FC17\r  ATTACKING:\r"
"^FCA6   CYBORG\r"
"   WARRIOR"
;

std::string actor_info47 =
"^AN21^FC17\r  ATTACKING:\r"
"^FCA6  REPTILIAN\r"
"   WARRIOR"
;

std::string actor_info48 =
"^AN22^FC17\r\r  ATTACKING:\r"
"^FCA6 ACID DRAGON"
;

std::string actor_info49 =
"^AN23^FC17\r  ATTACKING:\r"
"^FCA6   BIO-MECH\r"
"   GUARDIAN"
;

std::string actor_info50;

// explosionobj
std::string actor_info51 =
"^SH08B^FC17\r  ATTACKING:\r"
"^FCA6 ANTI-PLASMA\r"
"  EXPLOSION\r"
"    BLAST"
;

// pd_explosionobj
std::string actor_info52 =
"^SH0E6^FC17\r  ATTACKING:\r"
"^FCA6  DETONATOR\r"
"  EXPLOSION"
;

// Final Boss #1
std::string actor_info53 =
"^AN29^FC17\r  ATTACKING:\r"
"^FCA6  THE GIANT\r"
"   STALKER"
;

// Final Boss #2
std::string actor_info54 =
"^AN2A^FC17\r  ATTACKING:\r"
"^FCA6 THE SPECTOR\r"
"   DEMON"
;

// Final Boss #3
std::string actor_info55 =
"^AN2b^FC17\r  ATTACKING:\r"
"^FCA6 THE ARMORED\r"
"   STALKER"
;

// Final Boss #4
std::string actor_info56 =
"^AN2c^FC17\r  ATTACKING:\r"
"^FCA6 THE CRAWLER\r"
"    BEAST"
;


ActorMessages ActorInfoMsg =
{
	actor_info4, // Sector Patrol
	actor_info5, // Turret
	actor_info9, // Bio-Tech
	actor_info10, // Pod
	actor_info11, // Electro-Alien
	actor_info12, // Electro-Sphere
	actor_info13, // STAR Sentinel
	actor_info14, // Genetic Guard
	actor_info15, // Mutant Human 1
	actor_info16, // Mutant Human 2
	empty_msg, // lg canister wait
	actor_info18, // Lg Canister Alien
	empty_msg, // sm canister wait
	actor_info20, // Sm canister Alien
	empty_msg, // gurney wait
	actor_info22, // Gurney Mutant
	actor_info36, // Liquid Alien
	actor_info24, // Alien Protector (old STAR Trooper)
	actor_info25, // Goldstern
	actor_info25m, // Goldstern Morphed
	actor_info27, // Volatile Transport
	actor_info28, // Floating Bomb
	actor_info50, // vital defence

	actor_info44, // Spider Mutant
	actor_info45, // breather beast
	actor_info46, // cyborg warrior

	actor_info47, // reptilian warrior
	actor_info48, // acid dragon
	actor_info49, // mech guardian

	actor_info53, // Final Boss 1
	actor_info54, // Final Boss 2
	actor_info55, // Final Boss 3
	actor_info56, // Final Boss 4

	empty_msg, // blake
	empty_msg, // crate 1
	empty_msg, // crate 2
	empty_msg, // crate 3,

	actor_info41, // Green Ooze
	actor_info42, // Black Ooze
	actor_info41a, // Green2 Ooze
	actor_info42a, // Black2 Ooze
	empty_msg, // Pod Egg

	actor_info44, // morphing_spider mutant
	actor_info47, // morphing_reptilian warrior
	actor_info16, // morphing_Mutant Human 2

	empty_msg, // SPACER

	actor_info31, // Electro-Alien SHOT
	empty_msg, // Post Barrier
	actor_info43, // Arc Barrier
	actor_info43a, // VPost Barrier
	actor_info43b, // VSpike Barrier

	actor_info25m, // Gold Morph Shot obj

	empty_msg, // Security Light
	actor_info33, // Explosion
	empty_msg, // Steam Grate
	empty_msg, // Steam Pipe
	actor_info36, // Liquid SHOT

	actor_info18, // Lg Canister Alien SHOT
	actor_info10, // POD Alien SHOT
	actor_info20, // Genetic Alien SHOT
	actor_info16, // Mutant Human 2 SHOT
	actor_info15, // Mutant Human 1 SHOT

	empty_msg, // player sp shot,
	empty_msg, // vent drip
	empty_msg, // flicker light,
	empty_msg, // Door Bomb
	empty_msg, // Door Bomb reserve
	empty_msg, // grenade,
	empty_msg, // BFG Shot

	actor_info51, // BFG Explosion
	actor_info52, // BFG Explosion

	actor_info44, // Boss 1 SHOT
	actor_info45, // Boss 2 SHOT
	actor_info46, // Boss 3 SHOT
	actor_info47, // Boss 4 SHOT
	actor_info48, // Boss 5 SHOT
	actor_info49, // Boss 6 SHOT
	actor_info54, // Boss 8 SHOT
	actor_info56, // Boss 10 SHOT

	empty_msg, // Doorexplosion

	actor_info52, // gr_explosion
}; // ActorInfoMsg

void initialize_messages()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		bevs_msg1 =
			"\r FOOD UNIT DISPENSES\r"
			"  A COLD BEVERAGE.\r"
			"\r     TOKENS: XX"
			;

		food_msg1 =
			"\r FOOD UNIT DISPENSES\r"
			"  SOMETHING EDIBLE.\r"
			"\r     TOKENS: XX"
			;

		actor_info4 =
			"^AN04^FC17\r\r  ATTACKING:\r"
			"^FCA6 SECTOR GUARD"
			;

		actor_info13 =
			"^AN0D^FC17\r\r  ATTACKING:\r"
			"^FCA6 TECH WARRIOR"
			;

		actor_info24 =
			"^AN18^FC17\r  ATTACKING:\r"
			"^FCA6    ALIEN\r"
			"  PROTECTOR"
			;

		actor_info50 =
			"^SH07A^FC17\r  ATTACKING:\r"
			"^FCA6   SECURITY\r"
			"    CUBE\r"
			"  EXPLOSION"
			;
	}
	else
	{
		bevs_msg1 =
			"\r"
			"\r"
			" FOOD UNIT DISPENSES\r"
			"  A COLD BEVERAGE."
			;

		food_msg1 =
			"\r"
			" FOOD UNIT DISPENSES\r"
			" SOMETHING RESEMBLING\r"
			"        FOOD."
			;

		actor_info4 =
			"^AN04^FC17\r"
			"\r"
			"  ATTACKING:\r"
			"^FCA6SECTOR PATROL"
			;

		actor_info13 =
			"^AN0D^FC17\r"
			"\r"
			"  ATTACKING:\r"
			"^FCA6STAR SENTINEL"
			;

		actor_info24 =
			"^AN18^FC17\r"
			"\r"
			"  ATTACKING:\r"
			"^FCA6 STAR TROOPER"
			;

		actor_info50 =
			"^SH07A^FC17\r"
			"  ATTACKING:\r"
			"^FCA6  PROJECTION\r"
			"  GENERATOR\r"
			"  EXPLOSION\r"
			;
	}
}
