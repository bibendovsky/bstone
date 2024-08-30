/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include <algorithm>

#include "3d_def.h"
#include "3d_menu.h"
#include "audio.h"
#include "jm_tp.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"

#include "bstone_generic_fizzle_fx.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_utility.h"


static int get_wall_page_count()
{
	return bstone::globals::page_mgr->get_wall_count();
}

#define NUM_TILES get_wall_page_count()


/*
=============================================================================

 LOCAL CONSTANTS

=============================================================================
*/

#define LOCATION_TEXT_COLOR (0xAF)

extern char prep_msg[];


void Died();

void InitGoldsternInfo();
void InitDoorList();
void InitStaticList();
void ConnectBarriers();
void DrawHealth();
void DrawKeys();
void DrawWeapon();
void DrawScore();
void InitInfoArea();
void ForceUpdateStatusBar();
void UpdateStatusBar();

bool LoadLevel(
	int levelnum);

void SetPlaneViewSize();

std::int16_t CalcAngle(
	objtype* from_obj,
	objtype* to_obj);

void FinishPaletteShifts();

void CA_CacheScreen(
	std::int16_t chunk);

void DoActor(
	objtype* ob);

bool LevelInPlaytemp(
	int level_index);

void PreloadUpdate(
	std::uint16_t current,
	std::uint16_t total);

void PreloadGraphics();

bool SaveLevel(
	int level_index);

void CheckHighScore(
	std::int32_t score,
	std::uint16_t other);

void AddTotalPoints(
	std::uint16_t points);

void AddTotalInformants(
	std::int8_t informants);

void AddTotalEnemy(
	std::uint16_t enemies);


/*
=============================================================================

 GLOBAL VARIABLES

=============================================================================
*/

fargametype gamestuff;
fargametype old_gamestuff;

gametype gamestate;
gametype old_gamestate;

bool ingame;
bool fizzlein;
int latchpics[NUMLATCHPICS];
eaWallInfo eaList[MAXEAWALLS];
std::int8_t NumEAWalls;

tilecoord_t GoldieList[GOLDIE_MAX_SPAWNS];
GoldsternInfo_t GoldsternInfo;

PlayerWarp player_warp;

extern std::uint16_t scan_value;

int NUMWEAPONS = 0;


void ScanInfoPlane();
void SetupGameLevel();

void DrawPlayScreen(
	bool InitInfoMsg);

void LoadLatchMem();
void GameLoop();

// BBi
static void fix_level_inplace();


/*
=============================================================================

 LOCAL VARIABLES

=============================================================================
*/

//
// NOTE: This array indexs the "statinfo" array in ACT1.C and is indexed
//                 upon tile number/values.
//

std::int8_t ExpCrateShapes[] = {
	42, // Chicken Leg
	44, // Ham/Steak
	26, // Clip
	24, // Pistol
	27, // Pulse
	28, // ION
	46, // Grenade
	62, // Money Bag
	63, // Loot
	64, // Gold
	65, // Bonus
	71, // Gore 1
	74, // Gore 2
	32, // red key
	33, // yel key
	34, // grn key
	35, // blu key
	36, // gld key
};

void UpdateSoundLoc()
{
	sd_update();
}


void ClearMemory()
{
}

void INVALID_ACTOR_ERR(
	const int x,
	const int y)
{
	BSTONE_THROW_DYNAMIC_SOURCE(
		("Invalid actor at (" + std::to_string(x) + ", " + std::to_string(y) + ").").c_str());
}


/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/
void ScanInfoPlane()
{
	std::uint16_t x, y;
	std::int16_t tile;
	std::uint16_t* start;
	bool gottextures = false;
	bool gotcolors = false;

	const auto spawn_aog_boss = [](int tile, int x, int y)
	{
		switch (tile)
		{
			case 232:
			case 233:
			case 250:
			case 251:
			case 268:
			case 269:
				break;

			default: BSTONE_THROW_STATIC_SOURCE("Unknown AOG boss tile.");
		}

		if (tile >= 268)
		{
			tile -= 264;
		}
		else if (tile >= 250)
		{
			tile -= 248;
		}
		else
		{
			tile -= 232;
		}

		SpawnOffsetObj(
			static_cast<enemy_t>(static_cast<int>(en_spider_mutant) + tile),
			static_cast<std::int16_t>(x),
			static_cast<std::int16_t>(y));
	};

	const auto& assets_info = get_assets_info();

	detonators_spawned = 0;

	new_actor = nullptr;
	start = mapsegs[1].data();
	for (y = 0; y < mapheight; y++)
	{
		for (x = 0; x < mapwidth; x++)
		{
			sci_mCacheInfo* ci;
			scientist_t* st = nullptr;
			std::uint8_t tilehi, tilelo, block = 0;


			tile = *start++;
#if FIXME
			//
			// Check for tiles/icons to ignore...
			//
			switch (mapsegs[0][farmapylookup[y] + x])
			{
			case SMART_OFF_TRIGGER:
			case SMART_ON_TRIGGER:
				if (!assets_info.is_ps())
				{
					BSTONE_THROW_DYNAMIC_SOURCE(
						("Smart trigger (PS) at (" + std::to_string(x) + ", " + std::to_string(y) + ").").c_str());
				}
				continue;

			case DOORTRIGGERTILE:
				// Ignore all values/icons on top of these tiles...
				continue;
			}
#endif
			tilehi = (tile & 0xff00) >> 8;
			tilelo = (tile & 0xff);

			if (y < 63 && x < 63 && (*start & 0xff00) == 0xfa00)
			{
				scan_value = *start & 0x00ff;
			}
			else
			{
				scan_value = 0xffff;
			}

			switch (tilehi)
			{
			case 0xfe: // Top/Bottom colors
				if (gotcolors)
				{
					break;
				}
				x++;
				tile = *start++;
				TopColor = tile & 0xff00;
				TopColor |= TopColor >> 8;
				BottomColor = tile & 0xff;
				BottomColor |= BottomColor << 8;
				gotcolors = true;
				continue;
				break;

			case 0xFB: // Global Ceiling/Floor textures
				if (gottextures)
				{
					break;
				}
				x++;
				tile = *start++;

				CeilingTile = START_TEXTURES + ((tile & 0xff00) >> 8);
				if (CeilingTile > NUM_TILES - 1)
				{
					BSTONE_THROW_STATIC_SOURCE("Ceiling tile/texture is out of range.");
				}

				FloorTile = START_TEXTURES + (tile & 0xff);
				if (FloorTile > NUM_TILES - 1)
				{
					BSTONE_THROW_STATIC_SOURCE("Floor tile/texture is out of range.");
				}

				gottextures = true;
				continue;
				break;


			case 0xf5: // IntraLevel warp
				*(start - 1) = *start; // Move Coord right on top
				*start = 0;
				continue;
				break;

			case 0xfa:
				continue;

			case 0xf1: // Informant messages
			case 0xf2: // "Nice" scientist messages
			case 0xf3: // "Mean" scientist messages
				switch (tilehi)
				{
				case 0xf1:
					block = static_cast<std::uint8_t>(INFORMANT_HINTS);
					st = &InfHintList;
					break;

				case 0xf2:
					block = static_cast<std::uint8_t>(NICE_SCIE_HINTS);
					st = &NiceSciList;
					break;

				case 0xf3:
					block = static_cast<std::uint8_t>(MEAN_SCIE_HINTS);
					st = &MeanSciList;
					break;
				}

				ci = &st->smInfo[st->NumMsgs];
				ci->mInfo.local_val = 0xff;
				ci->mInfo.global_val = tilelo;
				if (!ReuseMsg((mCacheInfo*)ci, st->NumMsgs, sizeof(sci_mCacheInfo)))
				{
					CacheMsg((mCacheInfo*)ci, block, ci->mInfo.global_val);
					ci->mInfo.local_val = static_cast<std::uint8_t>(InfHintList.NumMsgs);
				}

				if (++st->NumMsgs > MAX_CACHE_MSGS)
				{
					BSTONE_THROW_STATIC_SOURCE("[INFORMANTS] Too many \"cached msgs\" loaded.");
				}

				ci->areanumber = GetAreaNumber(static_cast<std::int8_t>(x), static_cast<std::int8_t>(y));

				if (ci->areanumber == 0 || ci->areanumber >= NUMAREAS)
				{
					ci->areanumber = 0xff;
				}
				continue;
				break;

			case 0:
				if (!tilelo)
				{
					continue;
				}
				break;
			}

			//
			// SPECIAL SPAWN CODING FOR BLASTABLE CRATES...
			//

			if (tile >= 432 && tile <= 485)
			{
				if (!assets_info.is_aog_sw() && tile >= 468)
				{
					SpawnOffsetObj(en_crate3, x, y);
					new_actor->temp2 = ExpCrateShapes[tile - 468];
					new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if (tile >= 475 && tile <= 478)
					{
						tile = (tile - 475) + bo_money_bag;
					}
					else
					{
						tile = 0;
					}
				}
				else if (!assets_info.is_aog_sw() && tile >= 450)
				{
					SpawnOffsetObj(en_crate2, x, y);
					new_actor->temp2 = ExpCrateShapes[tile - 450];
					new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if (tile >= 457 && tile <= 460)
					{
						tile = (tile - 457) + bo_money_bag;
					}
					else
					{
						tile = 0;
					}
				}
				else if (tile >= 432)
				{
					SpawnOffsetObj(en_crate1, x, y);
					new_actor->temp2 = ExpCrateShapes[tile - 432];
					new_actor->temp3 = static_object_to_ui16(ReserveStatic());

					if (tile >= 439 && tile <= 442)
					{
						tile = (tile - 439) + bo_money_bag;
					}
					else
					{
						tile = 0;
					}
				}

				if (tile)
				{
					if (tile > bo_loot)
					{
						tile += 3;
					}
					tile -= bo_money_bag;
					AddTotalPoints(static_points[tile]);
				}

				continue;
			}

			switch (tile)
			{
			case 19:
			case 20:
			case 21:
			case 22:
				if (assets_info.is_aog() && playstate == ex_transported)
				{
					SpawnPlayer(
						static_cast<std::int16_t>(player_warp.tilex),
						static_cast<std::int16_t>(player_warp.tiley),
						static_cast<std::int16_t>((1 - (player_warp.dir >> 1)) & 3));
				}
				else
				{
					SpawnPlayer(x, y, NORTH + tile - 19);
				}

				break;

			case 30: // Yellow Puddle
				if (!assets_info.is_aog_sw())
				{
					static_cast<void>(SpawnStatic(x, y, tile - 23));
				}
				break;

			case 71: // BFG Weapon
				if (assets_info.is_ps())
				{
					static_cast<void>(SpawnStatic(x, y, tile - 23));
				}
				break;

			case 85: // Money bag
			case 86: // Loot
			case 87: // Gold
			case 88: // Bonus
				AddTotalPoints(static_points[statinfo[tile - 23].type - bo_money_bag]);

			case 53:

			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:

			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 38:

			case 39:
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:

			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 54:

			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:

			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:

			case 72: // Gurney Mutant
			case 73: // Large Canister
			case 74: // Small Canister
			case 75: // Empty Gurney
			case 76: // Empty Large Canister
			case 77: // Empty Small Canister
			case 78: // Dead Gen. Sci.

			case 80:
			case 83: // Floor Grate
			case 84: // Floor Pipe
				static_cast<void>(SpawnStatic(x, y, tile - 23));
				break;

			case 399: // gold 1
			case 400: // gold 2
			case 401: // gold 3
				AddTotalPoints(static_points[statinfo[tile - 315].type - bo_money_bag]);

			case 381:
			case 382:
			case 383:
			case 384:
			case 385:
			case 386:
			case 387:
			case 388:
			case 390: // candy bar
			case 391: // sandwich

			case 395: // Table
			case 396: // Chair
			case 397: // Stool
			case 398: // Gore

			case 402: //
			case 403: //
			case 404: //
			case 405: //
			case 406: //
			case 407: //
			case 408: //
			case 409: //
			case 410: //
			case 411: //
			case 412: //
			case 413: //
			case 414: //
			case 415: //
			case 416: //
			case 417: //
			case 418: //
			case 419: //
			case 420: //
			case 421: //
			case 422: //
			case 423: // bo_coin
			case 424: // bo_coin5
				static_cast<void>(SpawnStatic(x, y, tile - 315));
				break;

			case 486: // Plasma Detonator
				if (assets_info.is_ps())
				{
					SpawnHiddenOfs(en_plasma_detonator_reserve, x, y); // Spawn a reserve
					static_cast<void>(SpawnStatic(x, y, 486 - 375));
				}
				break;

			case 487: // Door rubble
			case 488: // AutoMapper Bonus #1
			case 489: // BonziTree
			case 490: // Yellow Potted plant
			case 491: // Tube Plant
			case 492: // HiTech Chair
			case 493: // AOG: Rent A Cop - Dead.
			case 494: // AOG: Pro Guard - Dead.
			case 495: // AOG: Swat Guard - Dead.
				static_cast<void>(SpawnStatic(x, y, tile - 375));
				break;


			case 393: // crate 2
			case 394: // crate 3
			case 392: // crate 1
				static_cast<void>(SpawnStatic(x, y, tile - 315));
				break;

			case 81:
			case 82:
				SpawnOffsetObj(static_cast<enemy_t>(en_bloodvent + tile - 81), x, y);
				break;


				//
				// GREEN OOZE
				//

			case 208:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 207:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 206:
				SpawnOffsetObj(en_green_ooze, x, y);
				break;


				//
				// BLACK OOZE
				//

			case 212:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 211:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 210:
				SpawnOffsetObj(en_black_ooze, x, y);
				break;



				// Flickering Light
				//
			case 79:
				SpawnOffsetObj(en_flickerlight, x, y);
				new_actor->lighting = LAMP_ON_SHADING;
				break;


			case 174:
				if (assets_info.is_ps())
				{
					SpawnBarrier(en_post_barrier, x, y, false);
				}
				else
				{
					SpawnBarrier(en_arc_barrier, x, y, true);
				}
				break;

			case 175:
				//
				// 174=off,175=on
				//
				SpawnBarrier(en_post_barrier, x, y, (tile - 174) != 0);
				break;

			case 138:
			case 139:
				if (assets_info.is_ps())
				{
					//
					// 138=off,139=on
					//
					SpawnBarrier(en_arc_barrier, x, y, (tile - 138) != 0);
				}
				break;

				//
				// VPOST Barrier
				//

				//
				// Switchable
				//
			case 563: // On
			case 562: // Off
				if (assets_info.is_ps())
				{
					SpawnBarrier(en_vpost_barrier, x, y, (tile - 562) != 0);
				}
				break;


				//
				// Cycle
				//
			case 567:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 566:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 565:
				if (assets_info.is_ps())
				{
					SpawnBarrier(en_vpost_barrier, x, y, 0);
				}
				break;

				//
				// VSPIKE Barrier
				//

				//
				// Switchable
				//
			case 426: // On
			case 425: // Off
				if (assets_info.is_ps())
				{
					SpawnBarrier(en_vspike_barrier, x, y, (tile - 425) != 0);
				}
				break;


				//
				// Cycle
				//
			case 430:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 429:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 428:
				if (assets_info.is_ps())
				{
					SpawnBarrier(en_vspike_barrier, x, y, 0);
				}
				break;

				//
				// STEAM GRATE
				//

			case 178:
				SpawnStand(en_steamgrate, x, y, 0);
				break;

				//
				// STEAM PIPE
				//

			case 179:
				SpawnStand(en_steampipe, x, y, 0);
				break;


				//
				// GOLDFIRE SPAWN SITES
				//
			case 124:
				if (!loadedgame)
				{
					if (GoldsternInfo.SpawnCnt == GOLDIE_MAX_SPAWNS)
					{
						BSTONE_THROW_STATIC_SOURCE("Too many Dr. Goldfire Spawn sites in level.");
					}
					GoldsternInfo.flags = GS_FIRSTTIME;
					if (gamestate.mapon == 9)
					{
						GoldsternInfo.WaitTime = 60;
					}
					else
					{
						GoldsternInfo.WaitTime = MIN_GOLDIE_FIRST_WAIT + Random(MAX_GOLDIE_FIRST_WAIT - MIN_GOLDIE_FIRST_WAIT);
					}
					GoldieList[GoldsternInfo.SpawnCnt].tilex = static_cast<std::uint8_t>(x);
					GoldieList[GoldsternInfo.SpawnCnt].tiley = static_cast<std::uint8_t>(y);
					GoldsternInfo.SpawnCnt++;

					if (assets_info.is_ps() && gamestate.mapon == GOLD_MORPH_LEVEL)
					{
						AddTotalPoints(actor_points[goldsternobj - rentacopobj]);
						AddTotalEnemy(1);
					}
				}
				break;

				//
				// GOLDFIRE SPAWN - IMMEDEATLY
				//

			case 141:
				if (assets_info.is_ps() && !loadedgame)
				{
					if (GoldsternInfo.GoldSpawned)
					{
						BSTONE_THROW_STATIC_SOURCE("Too many FAST Goldfire spawn sites in map.");
					}

					if (GoldsternInfo.SpawnCnt == GOLDIE_MAX_SPAWNS)
					{
						BSTONE_THROW_STATIC_SOURCE("Too many Dr. Goldfire Spawn sites in level.");
					}

					GoldieList[GoldsternInfo.SpawnCnt].tilex = static_cast<std::uint8_t>(x);
					GoldieList[GoldsternInfo.SpawnCnt].tiley = static_cast<std::uint8_t>(y);

					GoldsternInfo.LastIndex = GoldsternInfo.SpawnCnt++;
					GoldsternInfo.flags = GS_COORDFOUND;

					SpawnStand(en_goldstern, x, y, 0);
					GoldsternInfo.GoldSpawned = true;
					new_actor = nullptr;
				}
				break;


				//
				// SECURITY LIGHT
				//
			case 160:
				SpawnOffsetObj(en_security_light, x, y);
				break;


				//
				// Projection Generator (AOG) / Rotating Cube (PS)
				//

			case 177:
				SpawnOffsetObj(en_rotating_cube, x, y);

				if (assets_info.is_ps())
				{
					new_actor = nullptr;
				}

				break;

				//
				// RENT-A-COP
				//
			case 180:
			case 181:
			case 182:
			case 183:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 144:
			case 145:
			case 146:
			case 147:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 108:
			case 109:
			case 110:
			case 111:
				SpawnStand(en_rentacop, x, y, tile - 108);
				break;


			case 184:
			case 185:
			case 186:
			case 187:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 148:
			case 149:
			case 150:
			case 151:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 112:
			case 113:
			case 114:
			case 115:
				SpawnPatrol(en_rentacop, x, y, tile - 112);
				break;


				//
				// officer
				//
			case 188:
			case 189:
			case 190:
			case 191:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 152:
			case 153:
			case 154:
			case 155:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 116:
			case 117:
			case 118:
			case 119:
				SpawnStand(en_gen_scientist, x, y, tile - 116);
				if (new_actor->flags & FL_INFORMANT)
				{
					AddTotalInformants(1);
					new_actor = nullptr;
				}
				break;


			case 192:
			case 193:
			case 194:
			case 195:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 156:
			case 157:
			case 158:
			case 159:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 120:
			case 121:
			case 122:
			case 123:
				SpawnPatrol(en_gen_scientist, x, y, tile - 120);
				if (new_actor->flags & FL_INFORMANT)
				{
					AddTotalInformants(1);
					new_actor = nullptr;
				}
				break;


				//
				//  PROGUARD
				//
			case 198:
			case 199:
			case 200:
			case 201:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 162:
			case 163:
			case 164:
			case 165:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 126:
			case 127:
			case 128:
			case 129:
				SpawnStand(en_proguard, x, y, tile - 126);
				break;


			case 202:
			case 203:
			case 204:
			case 205:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 36;
			case 166:
			case 167:
			case 168:
			case 169:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 36;
			case 130:
			case 131:
			case 132:
			case 133:
				SpawnPatrol(en_proguard, x, y, tile - 130);
				break;



			case 312:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}

			case 311:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}

			case 310:
				SpawnStand(en_electro_alien, x, y, 0);
				new_actor = nullptr;
				break;


				//
				// FLOATING BOMB - Stationary
				//

			case 364:
			case 365:
			case 366:
			case 367:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 346:
			case 347:
			case 348:
			case 349:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 328:
			case 329:
			case 330:
			case 331:
				SpawnStand(en_floatingbomb, x, y, tile - 328);
				new_actor->flags |= FL_STATIONARY;
				break;


				//
				// FLOATING BOMB - Start Stationary
				//

			case 296:
			case 297:
			case 298:
			case 299:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 278:
			case 279:
			case 280:
			case 281:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 260:
			case 261:
			case 262:
			case 263:
				SpawnStand(en_floatingbomb, x, y, tile - 260);
				break;


				//
				// FLOATING BOMB - Start Moving
				//

			case 300:
			case 301:
			case 302:
			case 303:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 282:
			case 283:
			case 284:
			case 285:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 264:
			case 265:
			case 266:
			case 267:
				SpawnPatrol(en_floatingbomb, x, y, tile - 264);
				break;


				//
				// VOLATILE MAT. TRANSPORT - Stationary
				//
			case 350:
			case 351:
			case 352:
			case 353:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 332:
			case 333:
			case 334:
			case 335:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 314:
			case 315:
			case 316:
			case 317:
				SpawnStand(en_volatiletransport, x, y, tile - 314);
				break;


				//
				// Black Ooze
				//
			case 313:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 295:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 277:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_black2_ooze, x, y);
				}
				break;



				//
				// Green Ooze
				//
			case 322:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 304:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 286:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_green2_ooze, x, y);
				}
				break;


				//
				// VOLATILE MAT. TRANSPORT - Moving
				//
			case 354:
			case 355:
			case 356:
			case 357:
				if (!assets_info.is_aog_sw())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 336:
			case 337:
			case 338:
			case 339:
				if (!assets_info.is_aog_sw())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
					tile -= 18;
				}
				else
				{
					break;
				}
			case 318:
			case 319:
			case 320:
			case 321:
				if (!assets_info.is_aog_sw())
				{
					SpawnPatrol(en_volatiletransport, x, y, tile - 318);
				}
				break;

				//
				// Genetic Guard
				//

			case 143:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 142:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 214:
				SpawnOffsetObj(en_genetic_guard, x, y);
				break;


				//
				// Cyborg Warrior
				//
			case 603:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 585:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 250:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_cyborg_warrior, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;


				//
				// Spider Mutant
				//
			case 601:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 583:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 232:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_spider_mutant, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;

				//
				// Acid Dragon
				//
			case 605:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 587:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}

			case 268:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_acid_dragon, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;

				//
				// Breather beast
				//
			case 602:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 584:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}

			case 233:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_breather_beast, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;

				//
				// Mech Guardian
				//
			case 606:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 588:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}

			case 269:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_mech_guardian, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;

				//
				// Reptilian Warrior
				//
			case 604:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						break;
					}
				}
				else
				{
					break;
				}
			case 586:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						break;
					}
				}
				else
				{
					break;
				}

			case 251:
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(en_reptilian_warrior, x, y);
				}
				else
				{
					spawn_aog_boss(tile, x, y);
				}
				break;


				//
				// Mutant Human type 1
				//

			case 105:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 104:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 103:
				SpawnOffsetObj(en_mutant_human1, x, y);
				break;


				//
				// Mutant Human type 2
				//
			case 125:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 107:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 106:
				SpawnOffsetObj(en_mutant_human2, x, y);
				break;


				//
				// Small Canister Alien - CONTAINED
				//

			case 136:
				if (gamestate.difficulty < gd_hard)
				{
					static_cast<void>(SpawnStatic(x, y, 74 - 23));
					break;
				}
			case 135:
				if (gamestate.difficulty < gd_medium)
				{
					static_cast<void>(SpawnStatic(x, y, 74 - 23));
					break;
				}
			case 134:
				SpawnOffsetObj(en_scan_wait_alien, x, y);
				break;



				//
				// Large Canister Alien - CONTAINED
				//
			case 172:
				if (gamestate.difficulty < gd_hard)
				{
					static_cast<void>(SpawnStatic(x, y, 73 - 23));
					break;
				}
			case 171:
				if (gamestate.difficulty < gd_medium)
				{
					static_cast<void>(SpawnStatic(x, y, 73 - 23));
					break;
				}
			case 170:
				SpawnOffsetObj(en_lcan_wait_alien, x, y);
				break;


				//
				// Gurney Mutant - ASLEEP
				//

			case 161:
				if (gamestate.difficulty < gd_hard)
				{
					static_cast<void>(SpawnStatic(x, y, 72 - 23));
					break;
				}
			case 173:
				if (gamestate.difficulty < gd_medium)
				{
					static_cast<void>(SpawnStatic(x, y, 72 - 23));
					break;
				}
			case 137:
				SpawnOffsetObj(en_gurney_wait, x, y);
				break;

				//
				// Small Canister Alien - ACTIVE/WALKING
				//

			case 288:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 289:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 290:
				SpawnOffsetObj(en_scan_alien, x, y);
				break;


				//
				// Large Canister Alien - ACTIVE/WALKING
				//
			case 270:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 271:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 272:
				SpawnOffsetObj(en_lcan_alien, x, y);
				break;


				//
				// Gurney Mutant - AWAKE
				//
			case 275:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 274:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 273:
				SpawnOffsetObj(en_gurney, x, y);
				break;


			case 293:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
			case 292:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
			case 291:
				SpawnStand(en_liquid, x, y, 0);
				break;


				// P.O.D. Alien Egg
				//
			case 294:
				if (gamestate.difficulty < gd_hard)
				{
					scan_value = 0xff;
				}

			case 276:
				if (gamestate.difficulty < gd_medium)
				{
					scan_value = 0xff;
				}

			case 306:
				SpawnOffsetObj(en_podegg, x, y);
				if (scan_value == 0xff)
				{
					new_actor->obclass = deadobj;
				}
				else
				{
					AddTotalPoints(actor_points[podobj - rentacopobj]);
					AddTotalEnemy(1);
				}
				scan_value = 0xffff;
				break;

				// Morphing Brown/LBlue Post -> Spider Mutant
				//
			case 610:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 609:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 608:
				if (assets_info.is_ps())
				{
					if (scan_value == 0xff)
					{
						static_cast<void>(SpawnStatic(x, y, 402 - 315));
					}
					else
					{
						AddTotalPoints(actor_points[en_spider_mutant]);
						AddTotalEnemy(1);
						SpawnOffsetObj(en_morphing_spider_mutant, x, y);
					}
					scan_value = 0xffff;
				}
				break;


				// Morphing Gray/Green Post -> Reptilian Warrior
				//
			case 592:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 591:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 590:
				if (assets_info.is_ps())
				{
					if (scan_value == 0xff)
					{
						static_cast<void>(SpawnStatic(x, y, 403 - 315));
					}
					else
					{
						AddTotalPoints(actor_points[en_reptilian_warrior]);
						AddTotalEnemy(1);
						SpawnOffsetObj(en_morphing_reptilian_warrior, x, y);
					}
					scan_value = 0xffff;
				}
				break;



				// Morphing Statue -> Blue Boy
				//
			case 628:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_hard)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 627:
				if (assets_info.is_ps())
				{
					if (gamestate.difficulty < gd_medium)
					{
						scan_value = 0xff;
					}
				}
				else
				{
					break;
				}

			case 626:
				if (assets_info.is_ps())
				{
					if (scan_value == 0xff)
					{
						static_cast<void>(SpawnStatic(x, y, 48 - 23));
					}
					else
					{
						AddTotalPoints(actor_points[en_mutant_human2]);
						AddTotalEnemy(1);
						SpawnOffsetObj(en_morphing_mutanthuman2, x, y);
					}
					scan_value = 0xffff;
				}
				break;


				// P.O.D. Alien
				//
			case 309:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}

			case 308:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}

			case 307:
				SpawnOffsetObj(en_pod, x, y);
				break;


				// Electro-Sphere - Vertical Hover
				//
			case 360:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}

			case 342:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}

			case 324:
				SpawnOffsetObj(en_vertsphere, x, y);
				break;

				// Electro-Sphere - Horizontal Hover
				//
			case 361:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}

			case 343:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}

			case 325:
				SpawnOffsetObj(en_horzsphere, x, y);
				break;

				// Electro-Sphere - Diagonal Hover
				//
			case 362:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}

			case 344:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}

			case 326:
				SpawnOffsetObj(en_diagsphere, x, y);
				break;




				//
				// Stationary SWAT Guards
				//

			case 252:
			case 253:
			case 254:
			case 255:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 234:
			case 235:
			case 236:
			case 237:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 216:
			case 217:
			case 218:
			case 219:
				SpawnStand(en_swat, x, y, tile - 216);
				break;


				//
				// Roaming SWAT Guards
				//

			case 256:
			case 257:
			case 258:
			case 259:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 238:
			case 239:
			case 240:
			case 241:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 220:
			case 221:
			case 222:
			case 223:
				SpawnPatrol(en_swat, x, y, tile - 220);
				break;

				//
				// STATIONARY HANGING TURRETS
				//
			case 368:
			case 369:
			case 370:
			case 371:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 126;
			case 242:
			case 243:
			case 244:
			case 245:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 224:
			case 225:
			case 226:
			case 227:
				SpawnStand(en_hang_terrot, x, y, tile - 224);
				new_actor->flags |= FL_STATIONARY;
				break;


				//
				// ROTATING HANGING TURRETS
				//
			case 372:
			case 373:
			case 374:
			case 375:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 126;
			case 246:
			case 247:
			case 248:
			case 249:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 228:
			case 229:
			case 230:
			case 231:
				SpawnStand(en_hang_terrot, x, y, tile - 228);
				break;



				// --------------------------
				// PATH OBJECTS
				// --------------------------
							//
							// Swat Guards
							//

			case 540:
			case 541:
			case 542:
			case 543:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 522:
			case 523:
			case 524:
			case 525:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 504:
			case 505:
			case 506:
			case 507:
				SpawnPatrol(en_swat, x, y, tile - 504);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

				//
				// VOLATILE MAT. TRANSPORT
				//

			case 548:
			case 549:
			case 550:
			case 551:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 530:
			case 531:
			case 532:
			case 533:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 512:
			case 513:
			case 514:
			case 515:
				SpawnPatrol(en_volatiletransport, x, y, tile - 512);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

				//
				//  FLOATING BOMB -
				//

			case 544:
			case 545:
			case 546:
			case 547:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 526:
			case 527:
			case 528:
			case 529:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 508:
			case 509:
			case 510:
			case 511:
				SpawnPatrol(en_floatingbomb, x, y, tile - 508);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

				//
				// PRO GUARD
				//

			case 594:
			case 595:
			case 596:
			case 597:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 576:
			case 577:
			case 578:
			case 579:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 558:
			case 559:
			case 560:
			case 561:
				SpawnPatrol(en_proguard, x, y, tile - 558);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;


				//
				// RENT-A-COP
				//

			case 552:
			case 553:
			case 554:
			case 555:
				if (gamestate.difficulty < gd_hard)
				{
					break;
				}
				tile -= 18;
			case 534:
			case 535:
			case 536:
			case 537:
				if (gamestate.difficulty < gd_medium)
				{
					break;
				}
				tile -= 18;
			case 516:
			case 517:
			case 518:
			case 519:
				SpawnPatrol(en_rentacop, x, y, tile - 516);
				new_actor->flags &= ~FL_RANDOM_TURN;
				break;

				// -----------------------
				// BOSS ACTORS
				// -----------------------

			case 630: // FINAL BOSS 1
			case 631: // FINAL BOSS 2
			case 632: // FINAL BOSS 3
			case 633: // FINAL BOSS 4
				if (assets_info.is_ps())
				{
					SpawnOffsetObj(static_cast<enemy_t>(en_final_boss1 + tile - 630), x, y);
				}
				break;
			}

			// If "new_actor" is an object that gives points, add those points to level total...
			//
			// "new_actor" is cleared to keep from re-adding points from the previous actor!
			//
			if (new_actor && (new_actor->obclass >= rentacopobj) && (new_actor->obclass < crate1obj))
			{
				classtype obclass = new_actor->obclass;

				switch (obclass)
				{
				case lcan_wait_alienobj:
				case scan_wait_alienobj:
				case gurney_waitobj:
					obclass++;
					break;

				default:
					break;
				}

				AddTotalPoints(actor_points[obclass - rentacopobj]);
				AddTotalEnemy(1);
				new_actor = nullptr;
			}

			// Skip past FA code...
			//
			if (scan_value != 0xffff)
			{
				x++;
				start++;
			}
		}
	}

	if (!loadedgame)
	{
		gamestuff.level[gamestate.mapon].stats.accum_inf = gamestuff.level[gamestate.mapon].stats.total_inf;
	}
}

void AddTotalPoints(
	std::uint16_t points)
{
	if (loadedgame)
	{
		return;
	}

	gamestuff.level[gamestate.mapon].stats.total_points += points;
}

void AddTotalInformants(
	std::int8_t informants)
{
	if (loadedgame)
	{
		return;
	}

	gamestuff.level[gamestate.mapon].stats.total_inf += informants;
}

void AddTotalEnemy(
	std::uint16_t enemies)
{
	if (loadedgame)
	{
		return;
	}

	gamestuff.level[gamestate.mapon].stats.total_enemy +=
		static_cast<std::uint8_t>(enemies);
}

void SetupGameLevel()
{
	bool switchon = false;
	sci_mCacheInfo* ci = InfHintList.smInfo;
	std::int16_t x, y;
	std::uint16_t* map, tile, icon;
	keytype lock;
	std::uint16_t* map1, *map2;
	std::int16_t count;

	if (!loadedgame)
	{
		InitGoldsternInfo();
	}

	US_InitRndT(true);

	//
	// load the level
	//
	const auto& assets_info = get_assets_info();

	CA_CacheMap(gamestate.mapon + assets_info.get_levels_per_episode() * gamestate.episode);
	mapon = static_cast<std::int16_t>(mapon - (gamestate.episode * assets_info.get_levels_per_episode()));

	mapwidth = mapheaderseg[mapon].width;
	mapheight = mapheaderseg[mapon].height;

	if (mapwidth != 64 || mapheight != 64)
	{
		BSTONE_THROW_STATIC_SOURCE("Map not 64 x 64.");
	}

	// BBi
	fix_level_inplace();

	LoadLocationText(static_cast<std::int16_t>(
		gamestate.mapon + (assets_info.get_levels_per_episode() * gamestate.episode)));

	//
	// copy the wall data to a data segment array
	//
	travel_table_ = TravelTable{};
	tilemap = TileMap{};
	actorat = ActorAt{};

	std::fill(
		wallheight.begin(),
		wallheight.end(),
		0);

	map = mapsegs[0].data();
	map2 = mapsegs[1].data();
	for (y = 0; y < mapheight; y++)
	{
		for (x = 0; x < mapwidth; x++)
		{
			icon = *map2++;
			tile = *map++;

			if (tile < AREATILE)
			{
				// solid wall
				tilemap[x][y] = static_cast<std::uint8_t>(tile);

				switch (tile)
				{
				case RKEY_TILE:
				case YKEY_TILE:
				case BKEY_TILE:
				case BFG_TILE:
				case ION_TILE:
				case DETONATOR_TILE:
				case CLOAK_TILE:
				case LINC_TILE:
				case CLOAK_AMBUSH_TILE:
					if (!assets_info.is_ps())
					{
						INVALID_ACTOR_ERR(x, y);
					}
				case AMBUSHTILE:
					break;

				default:
					actorat[x][y] = reinterpret_cast<objtype*>(tile);
					break;
				}
			}

			if ((assets_info.is_ps() && tile < 64) || icon == PUSHABLETILE)
			{
				travel_table_[x][y] |= TT_TRAVELED;
			}
		}
	}

	//
	// spawn doors
	//
	InitActorList(); // start spawning things with a clean slate
	InitDoorList();

	InitMsgCache((mCacheList*)&ConHintList, sizeof(ConHintList), sizeof(ConHintList.cmInfo[0]));
	InitMsgCache((mCacheList*)&InfHintList, sizeof(InfHintList), sizeof(InfHintList.smInfo[0]));
	InitMsgCache((mCacheList*)&NiceSciList, sizeof(NiceSciList), sizeof(InfHintList.smInfo[0]));
	InitMsgCache((mCacheList*)&MeanSciList, sizeof(MeanSciList), sizeof(InfHintList.smInfo[0]));

	InitStaticList();

	map = mapsegs[0].data();
	map1 = mapsegs[1].data();

	NumEAWalls = 0;
	alerted = 0;
	LastInfoAttacker = nothing;

	// BBi
	auto aog_has_red_key = false;
	auto aog_has_projection_generator = false;

	auto aog_exit_wall_count = 0;
	auto has_win_tile = false;
	// BBi

	for (y = 0; y < mapheight; y++)
	{
		for (x = 0; x < mapwidth; x++)
		{
			tile = *map++;
			lock = static_cast<keytype>(*map1);

			// [AOG] Middle Shuttle / Exit wall.
			if (assets_info.is_aog() && tile == 30)
			{
				aog_exit_wall_count += 1;

				if (aog_exit_wall_count == 1)
				{
					gamestate.wintilex = x;
					gamestate.wintiley = y;
				}
			}

			if (tile == WINTIGGERTILE)
			{
				has_win_tile = true;
			}

			if (tile >= 88 && tile <= 105)
			{
				//
				// KEYS
				//

				switch (static_cast<int>(lock))
				{
				case 55:
				case 56:
					lock = static_cast<keytype>(kt_red + lock - 55);
					*map1 = 0;
					break;

				case 58:
					lock = kt_blue;
					*map1 = 0;
					break;

				case 57:
					if (assets_info.is_ps())
					{
						lock = kt_none;
						break;
					}

					lock = kt_green;
					*map1 = 0;
					break;

				case 59:
					if (assets_info.is_ps())
					{
						lock = kt_none;
						break;
					}

					lock = kt_gold;
					*map1 = 0;
					break;

				default:
					lock = kt_none;
				}

				//
				// DOOR
				//

				switch (tile)
				{

				case 88:
				case 89:
					SpawnDoor(x, y, !(tile % 2), lock, dr_bio);
					break;

				case 90:
				case 91:
					SpawnDoor(x, y, !(tile % 2), lock, dr_normal);
					break;

				case 92:
				case 93:
					SpawnDoor(x, y, !(tile % 2), lock, dr_prison);
					break;

				case 94:
				case 95:
					SpawnDoor(x, y, !(tile % 2), lock, dr_elevator);
					break;

				case 96:
				case 97:
					SpawnDoor(x, y, !(tile % 2), lock, dr_high_security);
					break;

				case 98: // oneway left  - Vert
				case 99: // oneway up    - Horz
				case 100: // oneway right - Vert
				case 101: // oneway down  - Horz
					SpawnDoor(x, y, !(tile % 2), lock, static_cast<door_t>(dr_oneway_left + (tile - 98)));
					break;

				case 102:
				case 103:
					SpawnDoor(x, y, !(tile % 2), lock, dr_office);
					break;

				case 104:
				case 105:
					SpawnDoor(x, y, !(tile % 2), lock, dr_space);
					break;


				}
			}
			else
			{
				switch (tile)
				{
				case SODATILE:
					if (!loadedgame)
					{
						SpawnConcession(x, y, static_cast<std::uint16_t>(lock), CT_BEVS);
						*map1 = 0;
					}
					break;



				case FOODTILE:
					if (!loadedgame)
					{
						SpawnConcession(x, y, static_cast<std::uint16_t>(lock), CT_FOOD);
						*map1 = 0;
					}
					break;

				case EATILE:
					eaList[static_cast<int>(NumEAWalls)].tilex = static_cast<std::int8_t>(x);
					eaList[static_cast<int>(NumEAWalls)].tiley = static_cast<std::int8_t>(y);
					eaList[static_cast<int>(NumEAWalls)].aliens_out = 0;
					if ((lock & 0xff00) == 0xfa00)
					{
						eaList[static_cast<int>(NumEAWalls)].delay = 60 * (lock & 0xff);
					}
					else
					{
						eaList[static_cast<int>(NumEAWalls)].delay = 60 * 8 + Random(60 * 22);
					}
					if (NumEAWalls++ == MAXEAWALLS)
					{
						BSTONE_THROW_STATIC_SOURCE("Too many Electro-Alien walls in level.");
					}
					break;

				case ON_SWITCH:
					if (assets_info.is_ps())
					{
						switchon = true;
					}
				case OFF_SWITCH:
				{
					if (assets_info.is_aog())
					{
						if ((map1[0] & 0xFF00) == 0xF800)
						{
							const auto level = map1[0] & 0xFF;

							const auto switch_x = (map1[1] >> 8) & 0xFF;
							const auto switch_y = map1[1] & 0xFF;

							const auto barrier_code = UpdateBarrierTable(level, switch_x, switch_y);
							map1[0] = 0xF800 | barrier_code;
							map1[1] = 0;
						}
					}
					else
					{
						const auto switch_x = (map1[0] >> 8) & 0xFF;
						const auto switch_y = map1[0] & 0xFF;

						const auto barrier_code = UpdateBarrierTable(switch_x, switch_y, switchon);
						map1[0] = 0xF800 | barrier_code;
					}

					// Init for next time.

					switchon = false;
					break;
				}
				}
			}

			// BBi
			if (map1[0] == 55 && assets_info.is_aog())
			{
				aog_has_red_key = true;
			}

			if (map1[0] == 177 && assets_info.is_aog_full())
			{
				aog_has_projection_generator = true;
			}

			switch (map1[0])
			{
			case 445:
			case 463:
			case 481:
				aog_has_red_key = true;
				break;

			default:
				break;
			}
			// BBi

			map1++;
		}
	}

	if (assets_info.is_aog())
	{
		if (has_win_tile && aog_exit_wall_count != 1)
		{
			BSTONE_THROW_STATIC_SOURCE("Wintile requires a single middle Shuttle / Exit wall.");
		}
	}


//
// spawn actors
//

	ScanInfoPlane();
	ConnectBarriers();

	// Init informant stuff
	//
	count = InfHintList.NumMsgs;
	LastInfArea = 0xff;
	FirstGenInfMsg = 0;
	for (; (ci->areanumber != 0xff) && (count--); ci++)
	{
		FirstGenInfMsg++;
	}
	TotalGenInfMsgs = InfHintList.NumMsgs - FirstGenInfMsg;


	//
	// Take out the special tiles that were not used...
	//

	map = mapsegs[0].data();
	for (y = 0; y < mapheight; y++)
	{
		for (x = 0; x < mapwidth; x++)
		{
			tile = *map++;
			switch (tile)
			{
			case RKEY_TILE:
			case YKEY_TILE:
			case BKEY_TILE:
			case BFG_TILE:
			case ION_TILE:
			case DETONATOR_TILE:
			case CLOAK_TILE:
			case LINC_TILE:
			case CLOAK_AMBUSH_TILE:
				if (!assets_info.is_ps())
				{
					break;
				}
			case AMBUSHTILE:
				tilemap[x][y] = 0;
				if (actorat[x][y] == (objtype*)AMBUSHTILE)
				{
					actorat[x][y] = nullptr;
				}
				*(map - 1) = GetAreaNumber(static_cast<std::int8_t>(x), static_cast<std::int8_t>(y));
				break;
			}
		}
	}

	if (assets_info.is_aog())
	{
		if (!aog_has_red_key &&
			gamestate.mapon > 0 &&
			gamestate.mapon < 10 &&
			gamestuff.level[gamestate.mapon + 1].locked)
		{
			BSTONE_THROW_DYNAMIC_SOURCE(("No red key on floor " + std::to_string(gamestate.mapon) + ".").c_str());
		}

		if (assets_info.is_aog_full() &&
			gamestate.episode == 5 &&
			gamestate.mapon == 9 &&
			!aog_has_projection_generator)
		{
			BSTONE_THROW_STATIC_SOURCE("No projection generator(s) on floor 10 episode 6.");
		}
	}
	else
	{
		//
		// Check and make sure a detonator is in a 'locked' level.
		//

		if (gamestate.mapon < 20 &&
			!detonators_spawned &&
			gamestuff.level[gamestate.mapon + 1].locked)
		{
			BSTONE_THROW_STATIC_SOURCE("No Fision/Plasma Detonator on level.");
		}
	}
}


// ------------------------------------------------------------------------
// LoadLocationText()
// ------------------------------------------------------------------------
void LoadLocationText(
	std::int16_t textNum)
{
	char* temp;

	LoadMsg(LocationText, LEVEL_DESCS, textNum + 1, MAX_LOCATION_DESC_LEN);
	temp = strstr(LocationText, "^XX");
	if (temp)
	{
		*temp = 0;
	}
}

void DrawPlayBorder()
{
	vid_set_ui_mask_3d(
		playstate == ex_transported);
}

// --------------------------------------------------------------------------
// BMAmsg() - These messages are displayed by the Text Presenter!
// --------------------------------------------------------------------------
void BMAmsg(
	const char* msg)
{
	const std::int16_t BMAx1 = 0; // outer bevel
	const std::int16_t BMAy1 = 152;
	const std::int16_t BMAw1 = 320;
	const std::int16_t BMAh1 = 48;

	const std::int16_t BMAx2 = BMAx1 + 7; // inner bevel
	const std::int16_t BMAy2 = BMAy1 + 4;
	const std::int16_t BMAw2 = BMAw1 - 14;
	const std::int16_t BMAh2 = BMAh1 - 8;

	BevelBox(BMAx1, BMAy1, BMAw1, BMAh1, BORDER_HI_COLOR, BORDER_MED_COLOR, BORDER_LO_COLOR);
	BevelBox(BMAx2, BMAy2, BMAw2, BMAh2, BORDER_LO_COLOR, BORDER_MED_COLOR, BORDER_HI_COLOR);

	if (msg)
	{
		PresenterInfo pi;
		fontstruct* font = (fontstruct*)grsegs[STARTFONT + fontnumber].data();
		std::int8_t numlines = 1;
		const char* p = msg;
		std::int16_t cheight;

		pi = PresenterInfo{};
		pi.flags = TPF_CACHE_NO_GFX;
		pi.script[0] = p;
		while (*p)
		{
			if (*p++ == TP_RETURN_CHAR)
			{
				numlines++;
			}
		}
		cheight = font->height * numlines + 1 + (TP_MARGIN * 2);

		pi.xl = BMAx2 + 1;
		pi.yl = BMAy2 + (BMAh2 - cheight) / 2;
		pi.xh = pi.xl + BMAw2 - 3;
		pi.yh = pi.yl + cheight - 1;
		pi.bgcolor = BORDER_MED_COLOR;
		pi.ltcolor = BORDER_HI_COLOR;
		fontcolor = BORDER_TEXT_COLOR;
		pi.shcolor = pi.dkcolor = BORDER_LO_COLOR;
		pi.fontnumber = static_cast<std::int8_t>(fontnumber);
		TP_InitScript(&pi);
		TP_Presenter(&pi);
	}
}

// ----------------------------------------------------------------------
// CacheBMAmsg() - Caches in a Message Number and displays it using
//      BMAmsg()
// ----------------------------------------------------------------------
void CacheBMAmsg(std::uint16_t MsgNum)
{
	CA_CacheGrChunk(MsgNum);
	const auto string = reinterpret_cast<char*>(grsegs[MsgNum].data());

	if (string == nullptr)
	{
		auto error_message = std::string{};
		error_message.reserve(64);
		error_message += "Missing message #";
		error_message += std::to_string(MsgNum);
		error_message += '.';
		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}

	const auto pos = strstr(string, "^XX");

	if (pos != nullptr)
	{
		pos[3] = '\0';
	}

	BMAmsg(string);
	UNCACHEGRCHUNK(MsgNum);
}

void BevelBox(
	std::int16_t xl,
	std::int16_t yl,
	std::int16_t w,
	std::int16_t h,
	std::uint8_t hi,
	std::uint8_t med,
	std::uint8_t lo)
{
	std::int16_t xh = xl + w - 1, yh = yl + h - 1;
	std::uint8_t hc;

	VWB_Bar(xl, yl, w, h, med); // inside

	hc = med + 1;

	VWB_Hlin(xl, xh, yl, hi); // top
	VWB_Hlin(xl, xh, yh, lo); // bottom
	VWB_Vlin(yl, yh, xl, hi); // left
	VWB_Vlin(yl, yh, xh, lo); // right
	VWB_Plot(xl, yh, hc); // lower-left
	VWB_Plot(xh, yl, hc); // upper-right
}

void ShadowPrintLocationText(
	sp_type type)
{
	const char* DebugText = "-- DEBUG MODE ENABLED --";
	const char* s = nullptr, *ls_text[3] = {"-- LOADING --", "-- SAVING --", "-- CHANGE VIEW SIZE --"};
	int w, h;

	// Used for all fields...
	//
	py = 5;
	fontcolor = 0xaf;

	// Print LOCATION info...
	//
	const auto& assets_info = get_assets_info();

	switch (type)
	{
	case sp_normal:
		// Print LEVEL info...
		//

		if (assets_info.is_aog())
		{
			px = 17;
		}
		else
		{
			px = 16;
		}

		if (assets_info.is_secret_level(gamestate.mapon))
		{
			const auto secret_floor_index = assets_info.secret_floor_get_index(gamestate.mapon);
			const auto index_string = std::to_string(secret_floor_index + 1);
			const auto message = "SECRET " + index_string;

			ShPrint(message.c_str(), 0, false);
		}
		else
		{
			if (!assets_info.is_ps())
			{
				ShPrint("FLOOR: ", 0, false);
			}
			else
			{
				ShPrint("AREA: ", 0, false);
			}
			if (!type)
			{
				auto map_string = std::to_string(
					assets_info.is_aog() ? gamestate.mapon : gamestate.mapon + 1);

				ShPrint(map_string.c_str(), 0, false);
			}
		}

		// Print LIVES info...
		//
		px = 267;
		ShPrint("LIVES: ", 0, false);
		if (!type)
		{
			auto lives_string = std::to_string(gamestate.lives);
			ShPrint(lives_string.c_str(), 0, false);
		}

		// Print location text
		//

		if (DebugOk)
		{
			s = DebugText;
		}
		else
		{
			s = LocationText;
		}
		break;

	case sp_changeview:
	case sp_loading:
	case sp_saving:
		s = ls_text[type - sp_loading];
		break;

	default:
		break;
	}

	VW_MeasurePropString(s, &w, &h);
	px = static_cast<std::int16_t>(160 - w / 2);
	ShPrint(s, 0, false);
}

void DrawTopInfo(
	sp_type type)
{
	auto old = static_cast<std::int8_t>(fontnumber);

	LatchDrawPic(0, 0, TOP_STATUSBARPIC);
	fontnumber = 2;
	ShadowPrintLocationText(type);
	fontnumber = old;
}

void DrawPlayScreen(
	bool InitInfoMsg)
{
	if (loadedgame)
	{
		return;
	}

	if (playstate != ex_transported)
	{
		VW_FadeOut();
	}

	WindowW = 253;
	WindowH = 8;
	fontnumber = 2;

	DrawPlayBorder();

	LatchDrawPic(0, 200 - STATUSLINES, STATUSBARPIC);
	LatchDrawPic(0, 0, TOP_STATUSBARPIC);

	ShadowPrintLocationText(sp_normal);

	DrawHealth();
	DrawKeys();
	DrawWeapon();
	DrawScore();

	InitInfoArea();

	if (InitInfoMsg)
	{
		DISPLAY_MSG("R.E.B.A.\rAGENT: BLAKE STONE\rALL SYSTEMS READY.", MP_max_val, MT_NOTHING);
	}
	else
	{
		DisplayNoMoMsgs();
	}

	ForceUpdateStatusBar();
}

void DrawWarpIn()
{
	vid_is_hud = true;

	InitInfoArea();

	DisplayInfoMsg(
		"\r\r    TRANSPORTING...",
		MP_POWERUP,
		2 * 60,
		MT_GENERAL);

	DrawHealth();
	DrawKeys();
	DrawWeapon();
	DrawScore();
	WindowW = 253;
	WindowH = 8;
	fontnumber = 2;

	VW_Bar(
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
		BLACK);

	LatchDrawPic(0, 200 - STATUSLINES, STATUSBARPIC);
	LatchDrawPic(0, 0, TOP_STATUSBARPIC);

	ShadowPrintLocationText(sp_normal);
	UpdateStatusBar();
	sd_play_player_item_sound(WARPINSND);
	fizzlein = true;

	ThreeDRefresh();

	vid_is_hud = false;
}

void Warped()
{
	vid_is_hud = true;

	std::int16_t iangle;

	DisplayInfoMsg("\r\r\r   TRANSPORTING OUT", MP_POWERUP, 7 * 60, MT_GENERAL);
	const auto old_weapon = gamestate.weapon;
	gamestate.weapon = -1; // take away weapon

	ThreeDRefresh();

	if (screenfaded)
	{
		VW_FadeIn();
	}

	iangle = (((player->dir + 4) % 8) >> 1) * 90;

	RotateView(iangle, 2);

	IN_ClearKeysDown();
	sd_play_player_item_sound(WARPINSND);
	bstone::GenericFizzleFX fizzle(
		BLACK,
		true);

	fizzle.initialize(gp_vanilla_fizzle_fx());

	static_cast<void>(fizzle.present());

	IN_UserInput(100);
	sd_wait_sound_done();

	gamestate.weapon = old_weapon;
	gamestate.attackframe = gamestate.attackcount = gamestate.weaponframe = 0;

	vid_is_hud = false;
}


void Died()
{
	vid_is_hud = true;

	const std::uint8_t DEATHROTATE = 2;

	std::int16_t iangle;

	gamestate.weapon = -1; // take away weapon
	sd_play_player_voice_sound(PLAYERDEATHSND);
	iangle = CalcAngle(player, killerobj);

	RotateView(iangle, DEATHROTATE);

	//
	// fade to red
	//
	FinishPaletteShifts();

	bstone::GenericFizzleFX fizzle_fx(
		0x17,
		true);

	fizzle_fx.initialize(gp_vanilla_fizzle_fx());

	static_cast<void>(fizzle_fx.present());

	IN_UserInput(100);

	sd_wait_sound_done();
	StopMusic();

	gamestate.lives -= 1;

	if (gamestate.lives >= 0)
	{
		old_gamestate.lives = gamestate.lives;
		gamestate = old_gamestate;

		gamestuff = old_gamestuff;

#if 0
		gamestate.health = 100;
		gamestate.weapons = 1 << wp_autocharge;
		gamestate.weapon = gamestate.chosenweapon = wp_autocharge;

		gamestate.ammo = STARTAMMO;
		gamestate.attackframe = gamestate.attackcount =
			gamestate.weaponframe = 0;
#endif

		DrawHealth();
		DrawKeys();
		DrawWeapon();
		DrawScore();
		DrawKeys();
		ForceUpdateStatusBar();
	}

	vid_is_hud = false;
}

// --------------------------------------------------------------------------
// LoseScreen()   - Displays the Goldstern/DamagedReba message...
// --------------------------------------------------------------------------
void LoseScreen()
{
	PresenterInfo pi;

	VW_FadeOut();

	pi = PresenterInfo{};
	pi.flags = TPF_USE_CURRENT | TPF_SHOW_CURSOR | TPF_SCROLL_REGION | TPF_CONTINUE | TPF_TERM_SOUND | TPF_ABORTABLE;
	pi.xl = 14;
	pi.yl = 141;
	pi.xh = 14 + 293;
	pi.yh = 141 + 32;
	pi.ltcolor = 15;
	pi.bgcolor = 0;
	pi.dkcolor = 1;
	pi.shcolor = 1;
	pi.fontnumber = 2;
	pi.cur_x = static_cast<std::uint16_t>(-1);
	pi.print_delay = 2;

	ClearMemory();
	StopMusic();

	CA_CacheScreen(LOSEPIC);
	VW_UpdateScreen();

	TP_LoadScript(nullptr, &pi, LOSETEXT);

	// Now Presenting... The Loser Prize.. I nice message directly from Dr.
	// ==============    Goldstern himself!  Oooo Ohhhhh <clap> <clap> ...
	//

	VW_FadeIn();
	TP_Presenter(&pi);
	VW_FadeOut();

	TP_FreeScript(&pi, LOSETEXT);

	screenfaded = true;

	IN_ClearKeysDown();
}

// --------------------------------------------------------------------------
// RotateView()
//
// PARAMETERS:
//      DestAngle - Destination angle to rotate player->angle to.
//      RotSpeed  - Rotation Speed
// --------------------------------------------------------------------------
void RotateView(
	std::int16_t DestAngle,
	std::uint8_t RotSpeed)
{
	std::int16_t curangle, clockwise, counter, change;
	objtype* obj;
	bool old_godmode = godmode;

	if (player->angle > DestAngle)
	{
		counter = player->angle - DestAngle;
		clockwise = ANGLES - player->angle + DestAngle;
	}
	else
	{
		clockwise = DestAngle - player->angle;
		counter = player->angle + ANGLES - DestAngle;
	}

	godmode = true;
	curangle = player->angle;

	controly = 0;
	if (clockwise < counter)
	{
		//
		// rotate clockwise
		//
		if (curangle > DestAngle)
		{
			curangle -= ANGLES;
		}
		controlx = -1;
		do
		{
			change = tics * RotSpeed;
			if (curangle + change > DestAngle)
			{
				change = DestAngle - curangle;
			}

			curangle += change;
			player->angle += change;
			if (player->angle >= ANGLES)
			{
				player->angle -= ANGLES;
			}

			for (obj = player->next; obj; obj = obj->next)
			{
				DoActor(obj);
			}
			ThreeDRefresh();
			CalcTics();
		} while (curangle != DestAngle);
	}
	else
	{
		//
		// rotate counterclockwise
		//
		if (curangle < DestAngle)
		{
			curangle += ANGLES;
		}
		controlx = 1;
		do
		{
			change = -tics * RotSpeed;
			if (curangle + change < DestAngle)
			{
				change = DestAngle - curangle;
			}

			curangle += change;
			player->angle += change;
			if (player->angle < 0)
			{
				player->angle += ANGLES;
			}

			for (obj = player->next; obj; obj = obj->next)
			{
				DoActor(obj);
			}
			ThreeDRefresh();
			CalcTics();
		} while (curangle != DestAngle);
	}

	controlx = 0;
	player->dir = static_cast<dirtype>(((player->angle + 22) % 360) / 45);
	godmode = old_godmode;

}

void apply_bonus_queue()
{
	while (gamestuff.level[gamestate.mapon].bonus_queue != 0)
	{
		GivePoints(0, false);
	}
}

void GameLoop()
{
	// BBi
	vid_set_ui_mask_3d(false);
	// BBi

	bool quit = false;

	extern bool sd_sq_active_;

	auto Score = std::string{};
	bool died;

restartgame:

	ClearMemory();
	SETFONTCOLOR(0, 15);
	DrawPlayScreen(true);

	died = false;
	const auto& assets_info = get_assets_info();

	do
	{
		extern std::int16_t pickquick;

		ingame = true;

		if (died && pickquick)
		{
			char string[] = "  Auto Quick Load?  ";

			WindowX = WindowY = 0;
			WindowW = 320;
			WindowH = 152;

			if (Confirm(string))
			{
				playstate = ex_stillplaying;
				DrawPlayBorder();
				VW_UpdateScreen();
				US_ControlPanel(ScanCode::sc_f9);
			}

			DrawPlayBorder();
			VW_UpdateScreen();
		}

		if (!sd_sq_active_)
		{
			StartMusic(false);
		}

		if (!(loadedgame || LevelInPlaytemp(gamestate.mapon)))
		{
			DrawKeys();
			DrawScore();
		}

		startgame = false;
		if (!loadedgame)
		{
			// BBi
			VL_Bar(0, ref_view_top_y, vga_ref_width, ref_view_height, BLACK);
			vid_clear_3d();
			// BBi

			DrawTopInfo(sp_loading);
			DisplayPrepingMsg(prep_msg);

			LoadLevel(gamestate.mapon);
		}

		SetPlaneViewSize();
		if (loadedgame)
		{
			loadedgame = false;
		}

		if (died)
		{
			WindowY = 188;
			PreloadUpdate(1, 1);
			died = false;
			DrawPlayScreen(true);
		}
		else
		{
			PreloadGraphics();
			if (playstate == ex_transported)
			{
				DrawWarpIn();
			}
			else
			{
				DrawPlayScreen(false);
			}
		}

		if (!sd_sq_active_)
		{
			StartMusic(false);
		}

		PlayLoop();
		died = false;

		StopMusic();
		ingame = false;

		if (startgame || loadedgame)
		{
			goto restartgame;
		}

		switch (playstate)
		{

		case ex_transported: // Same as ex_completed
			Warped();

			if (assets_info.is_aog())
			{
				player_warp.tilex = player->tilex;
				player_warp.tiley = player->tiley;
				player_warp.dir = player->dir;
			}

		case ex_completed:
		case ex_secretlevel:
			if (assets_info.is_aog())
			{
				last_map_tile_x = player->tilex;
				last_map_tile_y = player->tiley;
			}
		case ex_warped:
			ClearMemory();
// FIXME
#if 0
			gamestate.mapon++;
#endif
			old_gamestate = gamestate;
			old_gamestuff = gamestuff;
			ClearNClose();
			DrawTopInfo(sp_loading);
#if 0
			DisplayPrepingMsg(prep_msg);
#endif
			WindowY = 181;
			StartMusic(false);
			SaveLevel(gamestate.lastmapon);
			break;

		case ex_died:
			if (InstantQuit)
			{
				InstantQuit = false;
			}
			else
			{
				Died();

				died = true; // don't "get psyched!"

				if (gamestate.lives > -1)
				{
					ClearMemory();
					break; // more lives left
				}

				LoseScreen();
			}


		case ex_victorious:
			vid_is_hud = true;

			MainMenu[MM_SAVE_MISSION].active = AT_DISABLED;
			MainMenu[MM_VIEW_SCORES].routine = &CP_ViewScores;
			MainMenu[MM_VIEW_SCORES].string = "HIGH SCORES";

			if (playstate == ex_victorious)
			{
				apply_bonus_queue();

				ThreeDRefresh();
				ThreeDRefresh();
			}

			ClearMemory();

			if (playstate == ex_victorious)
			{
				fontnumber = 1;
				CA_CacheGrChunk(STARTFONT + 1);
				update = Update{};
				CacheBMAmsg(YOUWIN_TEXT);

				// BBi
#if 0
				VW_ScreenToScreen(PAGE1START, bufferofs, 320, 200);
#endif

				UNCACHEGRCHUNK(STARTFONT + 1);
				sd_play_player_item_sound(BONUS1SND);
				sd_wait_sound_done();
				IN_UserInput(5 * 60);
				ClearMemory();
			}

			VW_FadeOut();

			vid_is_hud = false;

			Score = std::to_string(gamestate.score);
			piStringTable[0] = Score.c_str();

			if (playstate == ex_victorious)
			{
				if (!assets_info.is_ps())
				{
					vid_is_movie = true;

					auto movie = MovieId::intro;

					switch (gamestate.episode)
					{
					case 0:
					case 1:
					case 3:
						movie = MovieId::final_2;
						break;

					case 2:
					case 4:
						movie = MovieId::final_3;
						break;

					case 5:
						movie = MovieId::final;
						break;
					}

					DoMovie(movie);
				}
				else
				{
					CA_CacheGrChunk(ENDINGPALETTE);

					DoMovie(MovieId::final, grsegs[ENDINGPALETTE].data());

					UNCACHEGRCHUNK(ENDINGPALETTE);
				}

				vid_is_movie = false;

				NewViewSize(); // Recreates & Allocs the ScaleDirectory
				Breifing(BT_WIN, gamestate.episode);
			}

			CheckHighScore(gamestate.score, gamestate.mapon + 1);

			return;

		case ex_abort:
			quit = true;
			break;

		default:
			ClearMemory();
			break;
		}
	} while (!quit);


	// BBi
	vid_is_hud = false;
	// BBi
}

// BBi
std::string xy_to_string(
	const int x,
	const int y)
{
	return '(' + std::to_string(x) + ", " + std::to_string(y) + ')';
}

static void fix_level_inplace()
{
	if (loadedgame)
	{
		return;
	}

	const auto& assets_info = get_assets_info();

	if (assets_info.are_modded_levels())
	{
		return;
	}

	//
	// Fix standing bio-tech near volatile containers.
	//
	// E2L6 (38, 26)
	// E2L6 (55, 33)
	//
	if (assets_info.is_aog_full() &&
		gamestate.episode == 1 &&
		gamestate.mapon == 6)
	{
		{
			// Replace standing bio-tech with a moving one.
			constexpr auto x = 38;
			constexpr auto y = 26;

			constexpr auto index = (y * MAPSIZE) + x;

			if (mapsegs[1][index] == 153)
			{
				mapsegs[1][index] = 157;

				bstone::globals::logger->log_information(
					("[FIX][E2L6] Changing bio-tech at " + xy_to_string(x, y) +
						" from standing to moving.").c_str());
			}
		}

		{
			// Replace standing bio-tech with a moving one.
			constexpr auto x = 55;
			constexpr auto y = 33;

			constexpr auto index = (y * MAPSIZE) + x;

			if (mapsegs[1][index] == 153)
			{
				mapsegs[1][index] = 157;

				bstone::globals::logger->log_information(
					("[FIX][E2L6] Changing bio-tech at " + xy_to_string(x, y) +
						" from standing to moving.").c_str());
			}
		}
	}

	//
	// Fix bio-tech placed on special tile (AREATILE).
	// E5L2 (18, 43)
	//
	if (assets_info.is_aog_full() &&
		gamestate.episode == 4 &&
		gamestate.mapon == 2)
	{
		constexpr auto y = 43;
		constexpr auto old_x = 18;
		constexpr auto new_x = 17;

		constexpr auto old_index = (y * MAPSIZE) + old_x;
		constexpr auto new_index = (y * MAPSIZE) + new_x;

		if (mapsegs[1][old_index] != 0 && mapsegs[1][new_index] == 0)
		{
			bstone::swop(mapsegs[1][old_index], mapsegs[1][new_index]);

			bstone::globals::logger->log_information(
				("[FIX][E5L2] Moving bio-tech at " + xy_to_string(old_x, y) +
					" one tile to the left.").c_str());
		}
	}
}
