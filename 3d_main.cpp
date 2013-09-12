// 3D_MAIN.C

#include "3d_def.h"
#ifdef MSVC
#pragma hdrstop
#endif

#include "jm_lzh.h"


/*
=============================================================================

											BLAKE STONE
						 (C)opyright 1993, JAM Productions, Inc.

						 3D engine licensed by ID Software, Inc.
					Shareware distribution by Apogee Software, Inc.

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


void ConnectBarriers(void);
void FreeMusic(void);
void ClearMemory (void);
void CA_CacheScreen (Sint16 chunk);
void VH_UpdateScreen();
void PlayDemo (Sint16 demonumber);
void	DrawHighScores(void);
void freed_main();
void PreloadUpdate(Uint16 current, Uint16 total);
void OpenAudioFile(void);


int _argc;
char** _argv;


#define SKIP_TITLE_AND_CREDITS		(0)


#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

#define VIEWWIDTH       256                     // size of view window
#define VIEWHEIGHT      144


#define MAX_DEST_PATH_LEN	30

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

extern Sint16 pickquick;


void DrawCreditsPage(void);
void unfreed_main(void);
void ShowPromo(void);

const char * MainStrs[] = {
										"q","nowait","l","e",
										"version","system",
										"dval","tics","mem","powerball","music","d",
										"radar",BETA_CODE,
										nil
};

Sint16 starting_episode,starting_level,starting_difficulty;

char destPath[MAX_DEST_PATH_LEN+1];
char tempPath[MAX_DEST_PATH_LEN+15];

#if BETA_TEST
char bc_buffer[]=BETA_CODE;
#endif

void InitPlaytemp(void);


char QuitMsg[] = {"Unit: $%02x Error: $%02x"};

#ifdef CEILING_FLOOR_COLORS
Uint16 TopColor,BottomColor;
#endif

boolean         nospr;
boolean         IsA386;

Sint16 dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

//
// proejection variables
//
fixed           focallength;
Uint16        screenofs;
Sint16             viewwidth;
Sint16             viewheight;
Sint16             centerx;
Sint16             shootdelta;                     // pixels away from centerx a target can be
fixed           scale,maxslope;
Sint32            heightnumerator;
Sint16                     minheightdiv;


boolean         startgame,loadedgame;
Sint16             mouseadjustment;

char	configname[13]="CONFIG.";

Sint16 view_xl,view_xh,view_yl,view_yh;

#if IN_DEVELOPMENT
Uint16	democount=0,jim=0;
#endif

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

#if 0

unsigned mspeed;

void CalcSpeedRating()
{
	Sint16 loop;

	for (loop=0; loop<10; loop++)
	{
		ThreeDRefresh();
		mspeed += tics;
	}
}

#endif


// BBi
extern statetype s_ofs_stand;
extern statetype s_ofs_chase1;
extern statetype s_ofs_chase1s;
extern statetype s_ofs_chase2;
extern statetype s_ofs_chase3;
extern statetype s_ofs_chase3s;
extern statetype s_ofs_chase4;
extern statetype s_ofs_pain;
extern statetype s_ofs_die1;
extern statetype s_ofs_die1s;
extern statetype s_ofs_die2;
extern statetype s_ofs_die3;
extern statetype s_ofs_die4;
extern statetype s_ofs_die5;
extern statetype s_ofs_attack1;
extern statetype s_ofs_attack2;
extern statetype s_ofs_attack3;
extern statetype s_ofs_spit1;
extern statetype s_ofs_spit2;
extern statetype s_ofs_spit3;
extern statetype s_ofs_shoot1;
extern statetype s_ofs_shoot2;
extern statetype s_ofs_shoot3;
extern statetype s_ofs_pod_attack1;
extern statetype s_ofs_pod_attack1a;
extern statetype s_ofs_pod_attack2;
extern statetype s_ofs_pod_spit1;
extern statetype s_ofs_pod_spit2;
extern statetype s_ofs_pod_spit3;
extern statetype s_ofs_pod_death1;
extern statetype s_ofs_pod_death2;
extern statetype s_ofs_pod_death3;
extern statetype s_ofs_pod_ouch;
extern statetype s_ofs_bounce;
extern statetype s_ofs_ouch;
extern statetype s_ofs_esphere_death1;
extern statetype s_ofs_esphere_death2;
extern statetype s_ofs_esphere_death3;
extern statetype s_ofs_random;
extern statetype s_ofs_static;
extern statetype s_hold;
extern statetype s_ofs_smart_anim;
extern statetype s_ofs_smart_anim2;
extern statetype s_barrier_transition;
extern statetype s_vpost_barrier;
extern statetype s_spike_barrier;
extern statetype s_barrier_shutdown;
extern statetype s_rent_stand;
extern statetype s_rent_path1;
extern statetype s_rent_path1s;
extern statetype s_rent_path2;
extern statetype s_rent_path3;
extern statetype s_rent_path3s;
extern statetype s_rent_path4;
extern statetype s_rent_pain;
extern statetype s_rent_shoot1;
extern statetype s_rent_shoot2;
extern statetype s_rent_shoot3;
extern statetype s_rent_chase1;
extern statetype s_rent_chase1s;
extern statetype s_rent_chase2;
extern statetype s_rent_chase3;
extern statetype s_rent_chase3s;
extern statetype s_rent_chase4;
extern statetype s_rent_die1;
extern statetype s_rent_die2;
extern statetype s_rent_die3;
extern statetype s_rent_die3s;
extern statetype s_rent_die4;
extern statetype s_ofcstand;
extern statetype s_ofcpath1;
extern statetype s_ofcpath1s;
extern statetype s_ofcpath2;
extern statetype s_ofcpath3;
extern statetype s_ofcpath3s;
extern statetype s_ofcpath4;
extern statetype s_ofcpain;
extern statetype s_ofcshoot1;
extern statetype s_ofcshoot2;
extern statetype s_ofcshoot3;
extern statetype s_ofcchase1;
extern statetype s_ofcchase1s;
extern statetype s_ofcchase2;
extern statetype s_ofcchase3;
extern statetype s_ofcchase3s;
extern statetype s_ofcchase4;
extern statetype s_ofcdie1;
extern statetype s_ofcdie2;
extern statetype s_ofcdie3;
extern statetype s_ofcdie4;
extern statetype s_ofcdie5;
extern statetype s_swatstand;
extern statetype s_swatpath1;
extern statetype s_swatpath1s;
extern statetype s_swatpath2;
extern statetype s_swatpath3;
extern statetype s_swatpath3s;
extern statetype s_swatpath4;
extern statetype s_swatpain;
extern statetype s_swatshoot1;
extern statetype s_swatshoot2;
extern statetype s_swatshoot3;
extern statetype s_swatshoot4;
extern statetype s_swatshoot5;
extern statetype s_swatshoot6;
extern statetype s_swatshoot7;
extern statetype s_swatchase1;
extern statetype s_swatchase1s;
extern statetype s_swatchase2;
extern statetype s_swatchase3;
extern statetype s_swatchase3s;
extern statetype s_swatchase4;
extern statetype s_swatwounded1;
extern statetype s_swatwounded2;
extern statetype s_swatwounded3;
extern statetype s_swatwounded4;
extern statetype s_swatunwounded1;
extern statetype s_swatunwounded2;
extern statetype s_swatunwounded3;
extern statetype s_swatunwounded4;
extern statetype s_swatdie1;
extern statetype s_swatdie2;
extern statetype s_swatdie3;
extern statetype s_swatdie4;
extern statetype s_swatdie5;
extern statetype s_prostand;
extern statetype s_propath1;
extern statetype s_propath1s;
extern statetype s_propath2;
extern statetype s_propath3;
extern statetype s_propath3s;
extern statetype s_propath4;
extern statetype s_propain;
extern statetype s_proshoot1;
extern statetype s_proshoot2;
extern statetype s_proshoot3;
extern statetype s_proshoot4;
extern statetype s_proshoot5;
extern statetype s_proshoot6;
extern statetype s_proshoot6a;
extern statetype s_prochase1;
extern statetype s_prochase1s;
extern statetype s_prochase2;
extern statetype s_prochase3;
extern statetype s_prochase3s;
extern statetype s_prochase4;
extern statetype s_prodie1;
extern statetype s_prodie2;
extern statetype s_prodie3;
extern statetype s_prodie3a;
extern statetype s_prodie4;
extern statetype s_electro_appear1;
extern statetype s_electro_appear2;
extern statetype s_electro_appear3;
extern statetype s_electro_chase1;
extern statetype s_electro_chase2;
extern statetype s_electro_chase3;
extern statetype s_electro_chase4;
extern statetype s_electro_ouch;
extern statetype s_electro_shoot1;
extern statetype s_electro_shoot2;
extern statetype s_electro_shoot3;
extern statetype s_electro_shot1;
extern statetype s_electro_shot2;
extern statetype s_ofs_shot1;
extern statetype s_ofs_shot2;
extern statetype s_electro_die1;
extern statetype s_electro_die2;
extern statetype s_electro_die3;
extern statetype s_liquid_wait;
extern statetype s_liquid_move;
extern statetype s_liquid_rise1;
extern statetype s_liquid_rise2;
extern statetype s_liquid_rise3;
extern statetype s_liquid_stand;
extern statetype s_liquid_fall1;
extern statetype s_liquid_fall2;
extern statetype s_liquid_fall3;
extern statetype s_liquid_shoot1;
extern statetype s_liquid_shoot2;
extern statetype s_liquid_shoot3;
extern statetype s_liquid_ouch;
extern statetype s_liquid_die1;
extern statetype s_liquid_die2;
extern statetype s_liquid_die3;
extern statetype s_liquid_die4;
extern statetype s_liquid_dead;
extern statetype s_liquid_shot;
extern statetype s_blake1;
extern statetype s_blake2;
extern statetype s_blake3;
extern statetype s_blake4;
extern statetype s_goldstand;
extern statetype s_goldpath1;
extern statetype s_goldpath1s;
extern statetype s_goldpath2;
extern statetype s_goldpath3;
extern statetype s_goldpath3s;
extern statetype s_goldpath4;
extern statetype s_goldpain;
extern statetype s_goldshoot1;
extern statetype s_goldshoot2;
extern statetype s_goldshoot3;
extern statetype s_goldshoot4;
extern statetype s_goldshoot5;
extern statetype s_goldshoot6;
extern statetype s_goldshoot7;
extern statetype s_goldchase1;
extern statetype s_goldchase1s;
extern statetype s_goldchase2;
extern statetype s_goldchase3;
extern statetype s_goldchase3s;
extern statetype s_goldchase4;
extern statetype s_goldwarp_it;
extern statetype s_goldwarp_it1;
extern statetype s_goldwarp_it2;
extern statetype s_goldwarp_it3;
extern statetype s_goldwarp_it4;
extern statetype s_goldwarp_it5;
extern statetype s_goldwarp_out1;
extern statetype s_goldwarp_out2;
extern statetype s_goldwarp_out3;
extern statetype s_goldwarp_out4;
extern statetype s_goldwarp_out5;
extern statetype s_goldwarp_in1;
extern statetype s_goldwarp_in2;
extern statetype s_goldwarp_in3;
extern statetype s_goldwarp_in4;
extern statetype s_goldwarp_in5;
extern statetype s_goldmorphwait1;
extern statetype s_goldmorph1;
extern statetype s_goldmorph2;
extern statetype s_goldmorph3;
extern statetype s_goldmorph4;
extern statetype s_goldmorph5;
extern statetype s_goldmorph6;
extern statetype s_goldmorph7;
extern statetype s_goldmorph8;
extern statetype s_mgold_chase1;
extern statetype s_mgold_chase2;
extern statetype s_mgold_chase3;
extern statetype s_mgold_chase4;
extern statetype s_mgold_shoot1;
extern statetype s_mgold_shoot2;
extern statetype s_mgold_shoot3;
extern statetype s_mgold_shoot4;
extern statetype s_mgold_pain;
extern statetype s_security_light;
extern statetype s_scout_stand;
extern statetype s_scout_path1;
extern statetype s_scout_path2;
extern statetype s_scout_path3;
extern statetype s_scout_path4;
extern statetype s_scout_run;
extern statetype s_scout_run2;
extern statetype s_scout_run3;
extern statetype s_scout_run4;
extern statetype s_scout_dead;
extern statetype s_steamgrate;
extern statetype s_steamrelease1;
extern statetype s_steamrelease2;
extern statetype s_steamrelease3;
extern statetype s_steamrelease4;
extern statetype s_steamrelease5;
extern statetype s_steamrelease6;
extern statetype s_terrot_wait;
extern statetype s_terrot_found;
extern statetype s_terrot_shoot1;
extern statetype s_terrot_shoot2;
extern statetype s_terrot_shoot3;
extern statetype s_terrot_shoot4;
extern statetype s_terrot_seek1;
extern statetype s_terrot_seek1s;
extern statetype s_terrot_die1;
extern statetype s_terrot_die2;
extern statetype s_terrot_die3;
extern statetype s_terrot_die4;
extern statetype s_terrot_die5;
extern statetype s_player;
extern statetype s_attack;


statetype* states_list[] = {
    NULL,
    &s_ofs_stand,
    &s_ofs_chase1,
    &s_ofs_chase1s,
    &s_ofs_chase2,
    &s_ofs_chase3,
    &s_ofs_chase3s,
    &s_ofs_chase4,
    &s_ofs_pain,
    &s_ofs_die1,
    &s_ofs_die1s,
    &s_ofs_die2,
    &s_ofs_die3,
    &s_ofs_die4,
    &s_ofs_die5,
    &s_ofs_attack1,
    &s_ofs_attack2,
    &s_ofs_attack3,
    &s_ofs_spit1,
    &s_ofs_spit2,
    &s_ofs_spit3,
    &s_ofs_shoot1,
    &s_ofs_shoot2,
    &s_ofs_shoot3,
    &s_ofs_pod_attack1,
    &s_ofs_pod_attack1a,
    &s_ofs_pod_attack2,
    &s_ofs_pod_spit1,
    &s_ofs_pod_spit2,
    &s_ofs_pod_spit3,
    &s_ofs_pod_death1,
    &s_ofs_pod_death2,
    &s_ofs_pod_death3,
    &s_ofs_pod_ouch,
    &s_ofs_bounce,
    &s_ofs_ouch,
    &s_ofs_esphere_death1,
    &s_ofs_esphere_death2,
    &s_ofs_esphere_death3,
    &s_ofs_random,
    &s_ofs_static,
    &s_hold,
    &s_ofs_smart_anim,
    &s_ofs_smart_anim2,
    &s_barrier_transition,
    &s_vpost_barrier,
    &s_spike_barrier,
    &s_barrier_shutdown,
    &s_rent_stand,
    &s_rent_path1,
    &s_rent_path1s,
    &s_rent_path2,
    &s_rent_path3,
    &s_rent_path3s,
    &s_rent_path4,
    &s_rent_pain,
    &s_rent_shoot1,
    &s_rent_shoot2,
    &s_rent_shoot3,
    &s_rent_chase1,
    &s_rent_chase1s,
    &s_rent_chase2,
    &s_rent_chase3,
    &s_rent_chase3s,
    &s_rent_chase4,
    &s_rent_die1,
    &s_rent_die2,
    &s_rent_die3,
    &s_rent_die3s,
    &s_rent_die4,
    &s_ofcstand,
    &s_ofcpath1,
    &s_ofcpath1s,
    &s_ofcpath2,
    &s_ofcpath3,
    &s_ofcpath3s,
    &s_ofcpath4,
    &s_ofcpain,
    &s_ofcshoot1,
    &s_ofcshoot2,
    &s_ofcshoot3,
    &s_ofcchase1,
    &s_ofcchase1s,
    &s_ofcchase2,
    &s_ofcchase3,
    &s_ofcchase3s,
    &s_ofcchase4,
    &s_ofcdie1,
    &s_ofcdie2,
    &s_ofcdie3,
    &s_ofcdie4,
    &s_ofcdie5,
    &s_swatstand,
    &s_swatpath1,
    &s_swatpath1s,
    &s_swatpath2,
    &s_swatpath3,
    &s_swatpath3s,
    &s_swatpath4,
    &s_swatpain,
    &s_swatshoot1,
    &s_swatshoot2,
    &s_swatshoot3,
    &s_swatshoot4,
    &s_swatshoot5,
    &s_swatshoot6,
    &s_swatshoot7,
    &s_swatchase1,
    &s_swatchase1s,
    &s_swatchase2,
    &s_swatchase3,
    &s_swatchase3s,
    &s_swatchase4,
    &s_swatwounded1,
    &s_swatwounded2,
    &s_swatwounded3,
    &s_swatwounded4,
    &s_swatunwounded1,
    &s_swatunwounded2,
    &s_swatunwounded3,
    &s_swatunwounded4,
    &s_swatdie1,
    &s_swatdie2,
    &s_swatdie3,
    &s_swatdie4,
    &s_swatdie5,
    &s_prostand,
    &s_propath1,
    &s_propath1s,
    &s_propath2,
    &s_propath3,
    &s_propath3s,
    &s_propath4,
    &s_propain,
    &s_proshoot1,
    &s_proshoot2,
    &s_proshoot3,
    &s_proshoot4,
    &s_proshoot5,
    &s_proshoot6,
    &s_proshoot6a,
    &s_prochase1,
    &s_prochase1s,
    &s_prochase2,
    &s_prochase3,
    &s_prochase3s,
    &s_prochase4,
    &s_prodie1,
    &s_prodie2,
    &s_prodie3,
    &s_prodie3a,
    &s_prodie4,
    &s_electro_appear1,
    &s_electro_appear2,
    &s_electro_appear3,
    &s_electro_chase1,
    &s_electro_chase2,
    &s_electro_chase3,
    &s_electro_chase4,
    &s_electro_ouch,
    &s_electro_shoot1,
    &s_electro_shoot2,
    &s_electro_shoot3,
    &s_electro_shot1,
    &s_electro_shot2,
    &s_ofs_shot1,
    &s_ofs_shot2,
    &s_electro_die1,
    &s_electro_die2,
    &s_electro_die3,
    &s_liquid_wait,
    &s_liquid_move,
    &s_liquid_rise1,
    &s_liquid_rise2,
    &s_liquid_rise3,
    &s_liquid_stand,
    &s_liquid_fall1,
    &s_liquid_fall2,
    &s_liquid_fall3,
    &s_liquid_shoot1,
    &s_liquid_shoot2,
    &s_liquid_shoot3,
    &s_liquid_ouch,
    &s_liquid_die1,
    &s_liquid_die2,
    &s_liquid_die3,
    &s_liquid_die4,
    &s_liquid_dead,
    &s_liquid_shot,
    &s_blake1,
    &s_blake2,
    &s_blake3,
    &s_blake4,
    &s_goldstand,
    &s_goldpath1,
    &s_goldpath1s,
    &s_goldpath2,
    &s_goldpath3,
    &s_goldpath3s,
    &s_goldpath4,
    &s_goldpain,
    &s_goldshoot1,
    &s_goldshoot2,
    &s_goldshoot3,
    &s_goldshoot4,
    &s_goldshoot5,
    &s_goldshoot6,
    &s_goldshoot7,
    &s_goldchase1,
    &s_goldchase1s,
    &s_goldchase2,
    &s_goldchase3,
    &s_goldchase3s,
    &s_goldchase4,
    &s_goldwarp_it,
    &s_goldwarp_it1,
    &s_goldwarp_it2,
    &s_goldwarp_it3,
    &s_goldwarp_it4,
    &s_goldwarp_it5,
    &s_goldwarp_out1,
    &s_goldwarp_out2,
    &s_goldwarp_out3,
    &s_goldwarp_out4,
    &s_goldwarp_out5,
    &s_goldwarp_in1,
    &s_goldwarp_in2,
    &s_goldwarp_in3,
    &s_goldwarp_in4,
    &s_goldwarp_in5,
    &s_goldmorphwait1,
    &s_goldmorph1,
    &s_goldmorph2,
    &s_goldmorph3,
    &s_goldmorph4,
    &s_goldmorph5,
    &s_goldmorph6,
    &s_goldmorph7,
    &s_goldmorph8,
    &s_mgold_chase1,
    &s_mgold_chase2,
    &s_mgold_chase3,
    &s_mgold_chase4,
    &s_mgold_shoot1,
    &s_mgold_shoot2,
    &s_mgold_shoot3,
    &s_mgold_shoot4,
    &s_mgold_pain,
    &s_security_light,
    &s_scout_stand,
    &s_scout_path1,
    &s_scout_path2,
    &s_scout_path3,
    &s_scout_path4,
    &s_scout_run,
    &s_scout_run2,
    &s_scout_run3,
    &s_scout_run4,
    &s_scout_dead,
    &s_steamgrate,
    &s_steamrelease1,
    &s_steamrelease2,
    &s_steamrelease3,
    &s_steamrelease4,
    &s_steamrelease5,
    &s_steamrelease6,
    &s_terrot_wait,
    &s_terrot_found,
    &s_terrot_shoot1,
    &s_terrot_shoot2,
    &s_terrot_shoot3,
    &s_terrot_shoot4,
    &s_terrot_seek1,
    &s_terrot_seek1s,
    &s_terrot_die1,
    &s_terrot_die2,
    &s_terrot_die3,
    &s_terrot_die4,
    &s_terrot_die5,
    &s_player,
    &s_attack,
    NULL,
};

static int get_state_index(statetype* state)
{
    int i;

    if (state == NULL)
        return 0;

    for (i = 1; states_list[i] != NULL; ++i) {
        if (states_list[i] == state)
            return i;
    }

    return -1;
}
// BBi


/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig()
{
    FILE* stream = NULL;

    MakeDestPath(configname);

    stream = fopen(tempPath, "wb");

    if (stream != NULL) {
        int i;
        Sint16 value_i16;

        for (i = 0; i < MaxScores; ++i) {
            HighScore* score = &Scores[i];

            fwrite(score->name, 1, sizeof(score->name), stream);
            fwrite(&score->score, 1, sizeof(score->score), stream);
            fwrite(&score->completed, 1, sizeof(score->completed), stream);
            fwrite(&score->episode, 1, sizeof(score->episode), stream);
            fwrite(&score->ratio, 1, sizeof(score->ratio), stream);
        }

        value_i16 = (Sint16)SoundMode;
        fwrite(&value_i16, 1, sizeof(value_i16), stream);

        value_i16 = (Sint16)MusicMode;
        fwrite(&value_i16, 1, sizeof(value_i16), stream);

        value_i16 = (Sint16)DigiMode;
        fwrite(&value_i16, 1, sizeof(value_i16), stream);

        fwrite(&mouseenabled, 1, sizeof(mouseenabled), stream);
        fwrite(&joystickenabled, 1, sizeof(joystickenabled), stream);
        fwrite(&joypadenabled, 1, sizeof(joypadenabled), stream);
        fwrite(&joystickprogressive, 1, sizeof(joystickprogressive), stream);
        fwrite(&joystickport, 1, sizeof(joystickport), stream);

        fwrite(dirscan, 1, sizeof(dirscan), stream);
        fwrite(buttonscan, 1, sizeof(buttonscan), stream);
        fwrite(buttonmouse, 1, sizeof(buttonmouse), stream);
        fwrite(buttonjoy, 1, sizeof(buttonjoy), stream);

        fwrite(&viewsize, 1, sizeof(viewsize), stream);
        fwrite(&mouseadjustment, 1, sizeof(mouseadjustment), stream);
        fwrite(&gamestate.flags, 1, sizeof(gamestate.flags), stream);

        fclose(stream);
    }
}


//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

boolean ShowQuickMsg;
void NewGame (Sint16 difficulty,Sint16 episode)
{
	Uint16 oldf=gamestate.flags,loop;

	InitPlaytemp();
	playstate = ex_stillplaying;

	ShowQuickMsg=true;
	memset (&gamestuff,0,sizeof(gamestuff));
	memset (&gamestate,0,sizeof(gamestate));

	memset(&gamestate.barrier_table,0xff,sizeof(gamestate.barrier_table));
	memset(&gamestate.old_barrier_table,0xff,sizeof(gamestate.old_barrier_table));
	gamestate.flags = oldf & ~(GS_KILL_INF_WARN);
//	LoadAccessCodes();

	gamestate.difficulty = difficulty;


//
// The following are set to 0 by the memset() to gamestate - Good catch! :JR
//
//	gamestate.rzoom
//	gamestate.rpower
//	gamestate.old_door_bombs
// gamestate.plasma_detonators
//

	gamestate.weapons	 = 1<<wp_autocharge;			// |1<<wp_plasma_detonators;
	gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
	gamestate.old_weapons[0] = gamestate.weapons;
	gamestate.old_weapons[1] = gamestate.weapon;
	gamestate.old_weapons[2] = gamestate.chosenweapon;

	gamestate.health = 100;
	gamestate.old_ammo = gamestate.ammo = STARTAMMO;
//	gamestate.dollars = START_DOLLARS;
//	gamestate.cents   = START_CENTS;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	gamestate.episode=episode;
	gamestate.flags |= (GS_CLIP_WALLS|GS_ATTACK_INFOAREA);	//|GS_DRAW_CEILING|GS_DRAW_FLOOR);

#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
	if (gamestate.flags & GS_STARTLEVEL)
	{
		gamestate.mapon = starting_level;
		gamestate.difficulty = starting_difficulty;
		gamestate.episode = starting_episode;
	}
	else
#endif
		gamestate.mapon = 0;

	gamestate.key_floor = gamestate.mapon+1;
	startgame = true;

	for (loop=0; loop<MAPS_WITH_STATS; loop++)
	{
		gamestuff.old_levelinfo[loop].stats.overall_floor=100;
		if (loop)
			gamestuff.old_levelinfo[loop].locked=true;
	}

//	normalshade_div = SHADE_DIV;
//	shade_max = SHADE_MAX;
	ExtraRadarFlags = InstantWin = InstantQuit = 0;

	pickquick = 0;
}

//===========================================================================

//==========================================================================
//
//             'LOAD/SAVE game' and 'LOAD/SAVE level' code
//
//==========================================================================

boolean LevelInPlaytemp(char levelnum);

#define WriteIt(c,p,s)	cksize+=WriteInfo(c,(char *)p,s,handle)
#define ReadIt(d,p,s)	ReadInfo(d,(char *)p,s,handle)

#define LZH_WORK_BUFFER_SIZE	8192		

void* lzh_work_buffer;
Sint32 checksum;

//--------------------------------------------------------------------------
// InitPlaytemp()
//--------------------------------------------------------------------------
void InitPlaytemp()
{
	Sint16 handle;

	MakeDestPath(PLAYTEMP_FILE);
	if ((handle=open(tempPath,O_CREAT|O_TRUNC|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		MAIN_ERROR(INITPLAYTEMP_OPEN_ERR);

	close(handle);
}

//--------------------------------------------------------------------------
// DoChecksum()
//--------------------------------------------------------------------------
Sint32 DoChecksum(Uint8 *source,Uint16 size,Sint32 checksum)
{
	Uint16 i;

	for (i=0;i<size-1;i++)
		checksum += source[i]^source[i+1];

	return(checksum);
}

//--------------------------------------------------------------------------
// FindChunk()
//--------------------------------------------------------------------------
Sint32 FindChunk(Sint16 file, const char *chunk)
{
	Sint32 chunklen;
	char fchunk[5]={0,0,0,0,0};

	while (1)
	{
		if (read(file,fchunk,4)!=4)			// read chunk id
			break;
		read(file,&chunklen,4);					// read chunk length

		if (strstr(fchunk,chunk))				// look for chunk (sub-check!)
			return(chunklen);						// chunk found!

		lseek(file,chunklen,SEEK_CUR);		// skip this chunk
	}

	lseek(file,0,SEEK_END);						// make sure we're at the end
	return(0);
}

//--------------------------------------------------------------------------
// NextChunk()
//--------------------------------------------------------------------------
Sint32 NextChunk(Sint16 file)
{
	Sint32 chunklen;
	char fchunk[5]={0,0,0,0,0};

	if (read(file,fchunk,4) != 4)			// read chunk id
	{
		lseek(file,0,SEEK_END);				// make sure we're at the end
		return(0);
	}

	read(file,&chunklen,4);					// read chunk length
	return(chunklen);
}

char LS_current=-1,LS_total=-1;

//--------------------------------------------------------------------------
// ReadInfo()
//--------------------------------------------------------------------------
void ReadInfo(boolean decompress,char *dst, Uint16 size, Sint16 file)
{
	Uint16 csize,dsize;

	PreloadUpdate(LS_current++,LS_total);

	if (decompress)
	{
		IO_FarRead(file,(char *)&csize,sizeof(csize));
		IO_FarRead(file,lzh_work_buffer,csize);
		checksum=DoChecksum(reinterpret_cast<Uint8*>(lzh_work_buffer),csize,checksum);
		dsize=LZH_Decompress(lzh_work_buffer,dst,size,csize,SRC_MEM|DEST_MEM);
		if (dsize != size)
			MAIN_ERROR(READINFO_BAD_DECOMP);
	}
	else
	{
		IO_FarRead(file,dst,size);
		checksum=DoChecksum(reinterpret_cast<Uint8*>(dst),size,checksum);
	}
}

//--------------------------------------------------------------------------
// WriteInfo()
//--------------------------------------------------------------------------
Uint16 WriteInfo(boolean compress, char *src, Uint16 size, Sint16 file)
{
	Uint16 csize;

	PreloadUpdate(LS_current++,LS_total);

	if (compress)
	{
		csize=LZH_Compress(src,lzh_work_buffer,size,SRC_MEM|DEST_MEM);
		if (csize > LZH_WORK_BUFFER_SIZE)
			MAIN_ERROR(WRITEINFO_BIGGER_BUF);
		IO_FarWrite (file,(char *)&csize,sizeof(csize));
		IO_FarWrite (file,lzh_work_buffer,csize);
		checksum=DoChecksum(reinterpret_cast<Uint8*>(lzh_work_buffer),csize,checksum);
		csize += sizeof(csize);
	}
	else
	{
		IO_FarWrite (file,src,size);
		checksum=DoChecksum(reinterpret_cast<Uint8*>(src),size,checksum);
		csize=size;
	}

	return(csize);
}



//--------------------------------------------------------------------------
// LoadLevel()
//--------------------------------------------------------------------------

// FIXME
#if 0
boolean LoadLevel(Sint16 levelnum)
{
	extern boolean ShowQuickMsg;
	extern boolean ForceLoadDefault;
	extern Uint16 destoff;

	boolean oldloaded=loadedgame;
	Sint32 oldchecksum;
	objtype *ob;
	statobj_t *statptr;
	Sint16 handle,picnum;
	void* temp;
	Uint16 count;
	char *ptr;
	char chunk[5]="LVxx";

extern Sint16 nsd_table[];
extern Sint16 sm_table[];

char mod;

	WindowY=181;
	gamestuff.level[levelnum].locked=false;

	mod = levelnum % 6;
	normalshade_div = nsd_table[mod];
	shade_max = sm_table[mod];
	normalshade=(3*(maxscale>>2))/normalshade_div;

// Open PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	handle=open(tempPath,O_RDONLY|O_BINARY);

// If level exists in PLAYTEMP file, use it; otherwise, load it from scratch!
//
	sprintf(&chunk[2],"%02x",levelnum);
	if ((handle==-1) || (!FindChunk(handle,chunk)) || ForceLoadDefault)
	{
		close(handle);

		PreloadUpdate(LS_current+((LS_total-LS_current)>>1),LS_total);
		SetupGameLevel();
		gamestate.flags |= GS_VIRGIN_LEVEL;
		gamestate.turn_around=0;

		PreloadUpdate(1,1);
		ForceLoadDefault=false;
		goto overlay;
	}

	gamestate.flags &= ~GS_VIRGIN_LEVEL;

// Setup for LZH decompression
//
	LZH_Startup();
    lzh_work_buffer = malloc(LZH_WORK_BUFFER_SIZE);

// Read all sorts of stuff...
//
	checksum = 0;

	loadedgame=true;
	SetupGameLevel();
	loadedgame=oldloaded;

	ReadIt(true, tilemap, sizeof(tilemap));
	ReadIt(true, actorat, sizeof(actorat));
	ReadIt(true, areaconnect, sizeof(areaconnect));
	ReadIt(true, areabyplayer, sizeof(areabyplayer));

// Restore 'save game' actors
//
	ReadIt(false, &count, sizeof(count));
    temp = malloc(count * sizeof(*ob));
	ReadIt(true, temp, count*sizeof(*ob));
	ptr=temp;

	InitActorList ();							// start with "player" actor
	memcpy(new_actor,ptr,sizeof(*ob)-4);		// don't copy over links!
	ptr += sizeof(*ob);						//

	while (--count)
	{
		GetNewActor();
		memcpy(new_actor,ptr,sizeof(*ob)-4);		// don't copy over links!
		actorat[new_actor->tilex][new_actor->tiley]=new_actor;
#if LOOK_FOR_DEAD_GUYS
		if (new_actor->flags & FL_DEADGUY)
			DeadGuys[NumDeadGuys++]=new_actor;
#endif
		ptr += sizeof(*ob);
	}

    free(temp);
    temp = NULL;


   //
	//  Re-Establish links to barrier switches
	//

	ob = objlist;
	do
	{
		switch (ob->obclass)
		{
			case arc_barrierobj:
			case post_barrierobj:
         case vspike_barrierobj:
         case vpost_barrierobj:
				ob->temp2 = ScanBarrierTable(ob->tilex,ob->tiley);
			break;
		}
	} while (ob = ob->next);

	ConnectBarriers();

// Read all sorts of stuff...
//
	ReadIt(false, &laststatobj, sizeof(laststatobj));
	ReadIt(true, statobjlist, sizeof(statobjlist));
	ReadIt(true, doorposition, sizeof(doorposition));
	ReadIt(true, doorobjlist, sizeof(doorobjlist));
	ReadIt(false, &pwallstate, sizeof(pwallstate));
	ReadIt(false, &pwallx, sizeof(pwallx));
	ReadIt(false, &pwally, sizeof(pwally));
	ReadIt(false, &pwalldir, sizeof(pwalldir));
	ReadIt(false, &pwallpos, sizeof(pwallpos));
	ReadIt(false, &pwalldist, sizeof(pwalldist));
	ReadIt(true, TravelTable, sizeof(TravelTable));
	ReadIt(true, &ConHintList, sizeof(ConHintList));
	ReadIt(true, eaList, sizeof(eaWallInfo)*MAXEAWALLS);
	ReadIt(true, &GoldsternInfo, sizeof(GoldsternInfo));
   ReadIt(true, &GoldieList,sizeof(GoldieList));			
	ReadIt(false, gamestate.barrier_table,sizeof(gamestate.barrier_table));
	ReadIt(false, &gamestate.plasma_detonators,sizeof(gamestate.plasma_detonators));

// Read and evaluate checksum
//
	PreloadUpdate(LS_current++,LS_total);
	IO_FarRead (handle,(void *)&oldchecksum,sizeof(oldchecksum));

	if (oldchecksum != checksum)
	{
		Sint16 old_wx=WindowX,old_wy=WindowY,old_ww=WindowW,old_wh=WindowH,
			 old_px=px,old_py=py;

		WindowX=0; WindowY=16; WindowW=320; WindowH=168;
		CacheMessage(BADINFO_TEXT);
		WindowX=old_wx; WindowY=old_wy; WindowW=old_ww; WindowH=old_wh;
		px=old_px; py=old_py;

		IN_ClearKeysDown();
		IN_Ack();

		gamestate.score = 0;
		gamestate.nextextra = EXTRAPOINTS;
		gamestate.lives = 1;

		gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
		gamestate.weapons = 1<<wp_autocharge;		// |1<<wp_plasma_detonators;

		gamestate.ammo = 8;
	}

	close(handle);

// Clean-up LZH compression
//
    free(lzh_work_buffer);
    lzh_work_buffer = NULL;

	LZH_Shutdown();
	NewViewSize(viewsize);

// Check for Strange Door and Actor combos
//
	CleanUpDoors_N_Actors();


overlay:;

	return(true);
}
#endif // 0

boolean LoadLevel(int levelnum)
{
    extern boolean ShowQuickMsg;
    extern boolean ForceLoadDefault;
    extern Uint16 destoff;

    int i;
    int j;
    boolean oldloaded = loadedgame;
    Sint32 oldchecksum;
    objtype* ob;
    Sint16 handle;
    void* temp;
    Uint16 count;
    objtype* ptr;
    objtype** actorat_ptr;
    char chunk[5] = "LVxx";
    size_t ob_size;

    extern Sint16 nsd_table[];
    extern Sint16 sm_table[];

    char mod;

    WindowY = 181;
    gamestuff.level[levelnum].locked=false;

    mod = levelnum % 6;
    normalshade_div = nsd_table[mod];
    shade_max = sm_table[mod];
    normalshade = (3 * (maxscale >> 2)) / normalshade_div;

    // Open PLAYTEMP file
    //
    MakeDestPath(PLAYTEMP_FILE);
    handle = open(tempPath, O_RDONLY | O_BINARY);

    // If level exists in PLAYTEMP file, use it; otherwise, load it from scratch!
    //
    sprintf(&chunk[2], "%02x",levelnum);

    if ((handle == -1) || (!FindChunk(handle, chunk)) || ForceLoadDefault) {
        close(handle);

        PreloadUpdate(LS_current + ((LS_total - LS_current) >> 1), LS_total);
        SetupGameLevel();
        gamestate.flags |= GS_VIRGIN_LEVEL;
        gamestate.turn_around = 0;

        PreloadUpdate(1, 1);
        ForceLoadDefault = false;
        goto overlay;
    }

    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Read all sorts of stuff...
    //
    checksum = 0;

    loadedgame = true;
    SetupGameLevel();
    loadedgame = oldloaded;

    ReadIt(false, tilemap, sizeof(tilemap));
    ReadIt(false, actorat, sizeof(actorat));

    actorat_ptr = (objtype**)actorat;

    for (i = 0; i < MAPSIZE; ++i) {
        for (j = 0; j < MAPSIZE; ++j) {
            size_t value = (size_t)(*actorat_ptr);

            if ((value & 0x80000000) != 0) {
                value ^= 0x80000000;
                *actorat_ptr = &objlist[value];
            }

            ++actorat_ptr;
        }
    }

    ReadIt(false, areaconnect, sizeof(areaconnect));
    ReadIt(false, areabyplayer, sizeof(areabyplayer));

    // Restore 'save game' actors
    //
    ReadIt(false, &count, sizeof(count));
    temp = malloc(count * sizeof(objtype));
    ReadIt(false, temp, count * sizeof(objtype));
    ptr = (objtype*)temp;

    // start with "player" actor
    InitActorList();

    ob_size = offsetof(objtype, next);

    // don't copy over links!
    memcpy(new_actor, ptr, ob_size);

    new_actor->state = states_list[(size_t)new_actor->state];

    ++ptr;

    while (--count) {
        GetNewActor();

        // don't copy over links!
        memcpy(new_actor, ptr, ob_size);

        actorat[new_actor->tilex][new_actor->tiley] = new_actor;

#if LOOK_FOR_DEAD_GUYS
        if ((new_actor->flags & FL_DEADGUY) != 0)
            DeadGuys[NumDeadGuys++] = new_actor;
#endif

        new_actor->state = states_list[(size_t)new_actor->state];

        ++ptr;
    }

    free(temp);
    temp = NULL;


    //
    //  Re-Establish links to barrier switches
    //

    ob = objlist;
    do {
        switch (ob->obclass) {
        case arc_barrierobj:
        case post_barrierobj:
        case vspike_barrierobj:
        case vpost_barrierobj:
            ob->temp2 = ScanBarrierTable(ob->tilex, ob->tiley);
            break;
        default:
            break;
        }
    } while ((ob = ob->next) != NULL);

    ConnectBarriers();

    // Read all sorts of stuff...
    //
    ReadIt(false, &laststatobj, sizeof(laststatobj));

    if (laststatobj != NULL)
        laststatobj = &statobjlist[(size_t)laststatobj];

    ReadIt(false, statobjlist, sizeof(statobjlist));

    for (i = 0; i < MAXSTATS; ++i) {
        ptrdiff_t offset = (ptrdiff_t)statobjlist[i].visspot;

        if (offset >= 0)
            statobjlist[i].visspot = &((Uint8*)spotvis)[offset];
        else
            statobjlist[i].visspot = NULL;
    }

    ReadIt(false, doorposition, sizeof(doorposition));
    ReadIt(false, doorobjlist, sizeof(doorobjlist));
    ReadIt(false, &pwallstate, sizeof(pwallstate));
    ReadIt(false, &pwallx, sizeof(pwallx));
    ReadIt(false, &pwally, sizeof(pwally));
    ReadIt(false, &pwalldir, sizeof(pwalldir));
    ReadIt(false, &pwallpos, sizeof(pwallpos));
    ReadIt(false, &pwalldist, sizeof(pwalldist));
    ReadIt(false, TravelTable, sizeof(TravelTable));
    ReadIt(false, &ConHintList, sizeof(ConHintList));

    for (i = 0; i < MAX_CACHE_MSGS; ++i)
        ConHintList.cmInfo[i].mInfo.mSeg = NULL;

    ReadIt(false, eaList, sizeof(eaWallInfo) * MAXEAWALLS);
    ReadIt(false, &GoldsternInfo, sizeof(GoldsternInfo));
    ReadIt(false, &GoldieList,sizeof(GoldieList));
    ReadIt(false, gamestate.barrier_table,sizeof(gamestate.barrier_table));
    ReadIt(false, &gamestate.plasma_detonators,sizeof(gamestate.plasma_detonators));

    // Read and evaluate checksum
    //
    PreloadUpdate(LS_current++, LS_total);
    IO_FarRead(handle, &oldchecksum, sizeof(oldchecksum));

    if (oldchecksum != checksum) {
        Sint16 old_wx = WindowX;
        Sint16 old_wy = WindowY;
        Sint16 old_ww = WindowW;
        Sint16 old_wh = WindowH;
        Sint16 old_px = px;
        Sint16 old_py = py;

        WindowX = 0;
        WindowY = 16;
        WindowW = 320;
        WindowH = 168;

        CacheMessage(BADINFO_TEXT);

        WindowX = old_wx;
        WindowY = old_wy;
        WindowW = old_ww;
        WindowH = old_wh;

        px = old_px;
        py = old_py;

        IN_ClearKeysDown();
        IN_Ack();

        gamestate.score = 0;
        gamestate.nextextra = EXTRAPOINTS;
        gamestate.lives = 1;

        gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
        gamestate.weapons = 1 << wp_autocharge;

        gamestate.ammo = 8;
    }

    close(handle);

    NewViewSize(viewsize);

    // Check for Strange Door and Actor combos
    //
    CleanUpDoors_N_Actors();

overlay:

    return true;
}

//--------------------------------------------------------------------------
// SaveLevel()
//--------------------------------------------------------------------------

// FIXME
#if 0
boolean SaveLevel(Sint16 levelnum)
{
	objtype *ob;
	Sint16 handle;
	struct ffblk finfo;
	Sint32 offset,cksize;
	char chunk[5]="LVxx";
	Uint16 gflags = gamestate.flags;
	boolean rt_value=false;
	memptr temp;
	Uint16 count;
	char *ptr;
	char oldmapon;

	WindowY=181;

// Make sure floor stats are saved!
//
	oldmapon=gamestate.mapon;
	gamestate.mapon=gamestate.lastmapon;
	ShowStats(0,0,ss_justcalc,&gamestuff.level[gamestate.mapon].stats);
	gamestate.mapon=oldmapon;

// Yeah! We're no longer a virgin!
//
	gamestate.flags &= ~GS_VIRGIN_LEVEL;

// Open PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	if ((handle=open(tempPath,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		MAIN_ERROR(SAVELEVEL_DISKERR);

// Remove level chunk from file
//
	sprintf(&chunk[2],"%02x",levelnum);
	DeleteChunk(handle,chunk);

// Setup LZH compression
//
	LZH_Startup();
	MM_GetPtr(&lzh_work_buffer,LZH_WORK_BUFFER_SIZE);

// Write level chunk id
//
	write(handle,chunk,4);
	lseek(handle,4,SEEK_CUR);		// leave four bytes for chunk size

// Write all sorts of info...
//
	checksum = cksize = 0;
	WriteIt(true, tilemap, sizeof(tilemap));
	WriteIt(true, actorat, sizeof(actorat));
	WriteIt(true, areaconnect, sizeof(areaconnect));
	WriteIt(true, areabyplayer, sizeof(areabyplayer));

// Write actor list...
//
	MM_GetPtr(&temp,sizeof(objlist));
	for (ob=player,count=0,ptr=temp; ob; ob=ob->next,count++,ptr+=sizeof(*ob))
		memcpy(ptr,ob,sizeof(*ob));
	WriteIt(false, &count, sizeof(count));
	WriteIt(true, temp, count*sizeof(*ob));
	MM_FreePtr(&temp);

// Write all sorts of info...
//
	WriteIt(false, &laststatobj, sizeof(laststatobj));
	WriteIt(true, statobjlist, sizeof(statobjlist));
	WriteIt(true, doorposition, sizeof(doorposition));
	WriteIt(true, doorobjlist, sizeof(doorobjlist));
	WriteIt(false, &pwallstate, sizeof(pwallstate));
	WriteIt(false, &pwallx, sizeof(pwallx));
	WriteIt(false, &pwally, sizeof(pwally));
	WriteIt(false, &pwalldir, sizeof(pwalldir));
	WriteIt(false, &pwallpos, sizeof(pwallpos));
	WriteIt(false, &pwalldist, sizeof(pwalldist));
	WriteIt(true, TravelTable, sizeof(TravelTable));
	WriteIt(true, &ConHintList, sizeof(ConHintList));
	WriteIt(true, eaList, sizeof(eaWallInfo)*MAXEAWALLS);
	WriteIt(true, &GoldsternInfo, sizeof(GoldsternInfo));
	WriteIt(true, &GoldieList,sizeof(GoldieList));
	WriteIt(false, gamestate.barrier_table,sizeof(gamestate.barrier_table));
	WriteIt(false, &gamestate.plasma_detonators,sizeof(gamestate.plasma_detonators));

// Write checksum and determine size of file
//
	WriteIt(false, &checksum, sizeof(checksum));
	offset=tell(handle);

// Write chunk size, set file size, and close file
//
	lseek(handle,-(cksize+4),SEEK_CUR);
	write(handle,&cksize,4);

	chsize(handle,offset);
	close(handle);
	rt_value=true;

// Clean-up LZH compression
//
exit_func:;
	MM_FreePtr(&lzh_work_buffer);
	LZH_Shutdown();
	NewViewSize(viewsize);
	gamestate.flags = gflags;

	return(rt_value);
}
#endif // 0

boolean SaveLevel(int levelnum)
{
    int i;
    int j;
    objtype* ob;
    Sint16 handle;
    Sint32 offset;
    Sint32 cksize;
    char chunk[5] = "LVxx";
    Uint16 gflags = gamestate.flags;
    boolean rt_value = false;
    void* temp;
    Uint16 count;
    objtype* ptr;
    objtype** actorat_ptr;
    statobj_t* statobj_ptr;
    char oldmapon;

    WindowY = 181;

    // Make sure floor stats are saved!
    //
    oldmapon = gamestate.mapon;
    gamestate.mapon = gamestate.lastmapon;
    ShowStats(0, 0, ss_justcalc, &gamestuff.level[gamestate.mapon].stats);
    gamestate.mapon = oldmapon;

    // Yeah! We're no longer a virgin!
    //
    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Open PLAYTEMP file
    //
    MakeDestPath(PLAYTEMP_FILE);

    handle = open(tempPath, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

    if (handle == -1)
        MAIN_ERROR(SAVELEVEL_DISKERR);

    // Remove level chunk from file
    //
    sprintf(&chunk[2], "%02x", levelnum);
    DeleteChunk(handle, chunk);

    // Write level chunk id
    //
    write(handle, chunk, 4);

    // leave four bytes for chunk size
    lseek(handle, 4, SEEK_CUR);

    checksum = 0;
    cksize = 0;

    WriteIt(false, tilemap, sizeof(tilemap));

    //
    // actorat
    //

    temp = malloc(sizeof(actorat));
    actorat_ptr = (objtype**)temp;
    for (i = 0; i < MAPSIZE; ++i) {
        for (j = 0; j < MAPSIZE; ++j) {
            size_t value = (size_t)actorat[i][j];

            if (value >= (size_t)objlist) {
                value -= (size_t)objlist;
                value /= sizeof(objtype);
                value |= 0x80000000;
            }

            *actorat_ptr = (objtype*)value;
            ++actorat_ptr;
        }
    }

    WriteIt(false, temp, sizeof(actorat));
    free(temp);

    WriteIt(false, areaconnect, sizeof(areaconnect));
    WriteIt(false, areabyplayer, sizeof(areabyplayer));

    //
    // oblist
    //

    temp = malloc(sizeof(objlist));
    ptr = (objtype*)temp;
    count = 0;

    for (ob = player; ob != NULL; ob = ob->next) {
        int state_index = get_state_index(ob->state);

        if (state_index < 0)
            Quit("SaveLevel: unknown object's state.");

        memcpy(ptr, ob, sizeof(objtype));

        ptr->state = (statetype*)state_index;

        ++count;
        ++ptr;
    }

    WriteIt(false, &count, sizeof(count));
    WriteIt(false, temp, count * sizeof(objtype));
    free(temp);

    //
    // laststatobj
    //

    statobj_ptr = laststatobj;

    if (statobj_ptr != NULL)
        statobj_ptr = (statobj_t*)(statobj_ptr - statobjlist);

    WriteIt(false, &statobj_ptr, sizeof(statobj_ptr));


    //
    // statobjlist
    //

    temp = malloc(sizeof(statobjlist));
    memcpy(temp, statobjlist, sizeof(statobjlist));
    statobj_ptr = (statobj_t*)temp;

    for (i = 0; i < MAXSTATS; ++i) {
        ptrdiff_t offset = statobj_ptr->visspot - &spotvis[0][0];
        statobj_ptr->visspot = (Uint8*)offset;

        ++statobj_ptr;
    }

    WriteIt(false, temp, sizeof(statobjlist));
    free(temp);

    WriteIt(false, doorposition, sizeof(doorposition));
    WriteIt(false, doorobjlist, sizeof(doorobjlist));
    WriteIt(false, &pwallstate, sizeof(pwallstate));
    WriteIt(false, &pwallx, sizeof(pwallx));
    WriteIt(false, &pwally, sizeof(pwally));
    WriteIt(false, &pwalldir, sizeof(pwalldir));
    WriteIt(false, &pwallpos, sizeof(pwallpos));
    WriteIt(false, &pwalldist, sizeof(pwalldist));
    WriteIt(false, TravelTable, sizeof(TravelTable));
    WriteIt(false, &ConHintList, sizeof(ConHintList));
    WriteIt(false, eaList, sizeof(eaWallInfo) * MAXEAWALLS);
    WriteIt(false, &GoldsternInfo, sizeof(GoldsternInfo));
    WriteIt(false, &GoldieList,sizeof(GoldieList));
    WriteIt(false, gamestate.barrier_table, sizeof(gamestate.barrier_table));
    WriteIt(false, &gamestate.plasma_detonators, sizeof(gamestate.plasma_detonators));

    // Write checksum and determine size of file
    //
    WriteIt(false, &checksum, sizeof(checksum));
    offset = tell(handle);

    // Write chunk size, set file size, and close file
    //
    lseek(handle, -(cksize + 4), SEEK_CUR);
    write(handle, &cksize, 4);

    chsize(handle, offset);
    close(handle);

    rt_value = true;

    NewViewSize(viewsize);
    gamestate.flags = gflags;

    return rt_value;
}

//--------------------------------------------------------------------------
// DeleteChunk()
//--------------------------------------------------------------------------
Sint32 DeleteChunk(Sint16 handle, const char *chunk)
{
	Sint32 filesize,cksize,offset,bmove;
	Sint16 dhandle;

	lseek(handle,0,SEEK_SET);
	filesize=lseek(handle,0,SEEK_END);
	lseek(handle,0,SEEK_SET);

	if (cksize=FindChunk(handle,chunk))
	{
		offset=lseek(handle,0,SEEK_CUR)-8; 		// move back to CKID/SIZE
		bmove=filesize-(offset+8+cksize);	 	// figure bytes to move

		if (bmove)										// any data to move?
		{
		// Move data: FROM --> the start of NEXT chunk through the end of file.
		//              TO --> the start of THIS chunk.
		//
		// (ie: erase THIS chunk and re-write it at the end of the file!)
		//
			lseek(handle,cksize,SEEK_CUR);			// seek source to NEXT chunk

			MakeDestPath(PLAYTEMP_FILE);
			if ((dhandle=open(tempPath,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
				MAIN_ERROR(SAVELEVEL_DISKERR);

			lseek(dhandle,offset,SEEK_SET);  		// seek dest to THIS chunk
			IO_CopyHandle(handle,dhandle,bmove);	// copy "bmove" bytes

			close(dhandle);

			lseek(handle,offset+bmove,SEEK_SET);	// go to end of data moved
		}
		else
			lseek(handle,offset,SEEK_SET);
	}

	return(cksize);
}


// FIXME
#if 0
char SavegameInfoText[] =
    "\n\r"
    "\n\r"
    "-------------------------------------\n\r"
    "    Blake Stone: Aliens Of Gold\n\r"
    "Copyright 1993, JAM Productions, Inc.\n\r"
    "\n\r"
    "SAVEGAME file is from version: "__VERSION__"\n\r"
    " Compile Date :"__DATE__" : "__TIME__"\n\r"
    "-------------------------------------\n\r"
    "\x1a";
#endif // 0

char SavegameInfoText[] =
    "bstone (planet strike) save game file\n"
    "version: " BS_SAVE_GAME_VERSION "\n";


//--------------------------------------------------------------------------
// LoadTheGame()
//--------------------------------------------------------------------------

// FIXME
#if 0
boolean LoadTheGame(Sint16 handle)
{
	extern Sint16 lastmenumusic;

	Sint16 shandle;
	Sint32 cksize;
	void* temp=NULL;
	boolean rt_value=false;
   char InfoSpace[400];
   void* tempspace;

// Setup LZH decompression
//
	LZH_Startup();
	lzh_work_buffer = malloc(LZH_WORK_BUFFER_SIZE);


// Read in VERSion chunk
//
	if (!FindChunk(handle,"VERS"))
		goto cleanup;

	cksize = sizeof(SavegameInfoText);
	read(handle, InfoSpace, cksize);
	if (memcmp(InfoSpace, SavegameInfoText, cksize))
   {
		// Old Version of game

		Sint16 old_wx=WindowX,old_wy=WindowY,old_ww=WindowW,old_wh=WindowH,
			 old_px=px,old_py=py;

		WindowX=0; WindowY=16; WindowW=320; WindowH=168;
		CacheMessage(BADSAVEGAME_TEXT);
		SD_PlaySound (NOWAYSND);
		WindowX=old_wx; WindowY=old_wy; WindowW=old_ww; WindowH=old_wh;
		px=old_px; py=old_py;

	  	IN_ClearKeysDown();
	  	IN_Ack();

      VW_FadeOut();
      screenfaded = true;

     	goto cleanup;
	}

// Read in HEAD chunk
//
	if (!FindChunk(handle,"HEAD"))
		goto cleanup;

	ReadIt(true, &gamestate, sizeof(gamestate));
	ReadIt(true, &gamestuff, sizeof(gamestuff));

// Reinitialize page manager
//
#if DUAL_SWAP_FILES
	PM_Shutdown();
	PM_Startup ();
	PM_UnlockMainMem();
#endif


// Start music for the starting level in this loaded game.
//

	FreeMusic();
	StartMusic(false);

// Copy all remaining chunks to PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	if ((shandle=open(tempPath,O_CREAT|O_RDWR|O_TRUNC|O_BINARY,S_IREAD|S_IWRITE))==-1)
		goto cleanup;

	while (cksize=NextChunk(handle))
	{
		cksize += 8;								// include chunk ID and LENGTH
		lseek(handle,-8,SEEK_CUR);				// seek to start of chunk
		temp = malloc(cksize);				// alloc temp buffer
		IO_FarRead(handle,temp,cksize);		// read chunk from SAVEGAME file
		IO_FarWrite(shandle,temp,cksize);	// write chunk to PLAYTEMP file
		free(temp);						// free temp buffer
	}

	close(shandle);
	rt_value=true;

// Clean-up LZH decompression
//
cleanup:;
    free(lzh_work_buffer);
    lzh_work_buffer = NULL;

	LZH_Shutdown();
	NewViewSize(viewsize);

// Load current level
//
	if (rt_value)
	{
		LoadLevel(0xff);
		ShowQuickMsg=false;
	}

	return(rt_value);
}
#endif // 0

boolean LoadTheGame(int handle)
{
    extern Sint16 lastmenumusic;

    int shandle;
    Sint32 cksize;
    void* temp = NULL;
    boolean rt_value = false;
    char InfoSpace[400];

    // Read in VERSion chunk
    //
    if (!FindChunk(handle, "VERS"))
        goto cleanup;

    cksize = sizeof(SavegameInfoText);
    read(handle, InfoSpace, cksize);

    if (memcmp(InfoSpace, SavegameInfoText, cksize) != 0) {
        // Old Version of game

        Sint16 old_wx = WindowX;
        Sint16 old_wy = WindowY;
        Sint16 old_ww = WindowW;
        Sint16 old_wh = WindowH;
        Sint16 old_px = px;
        Sint16 old_py = py;

        WindowX = 0;
        WindowY = 16;
        WindowW = 320;
        WindowH = 168;

        CacheMessage(BADSAVEGAME_TEXT);
        SD_PlaySound(NOWAYSND);

        WindowX = old_wx;
        WindowY = old_wy;
        WindowW = old_ww;
        WindowH = old_wh;

        px = old_px;
        py = old_py;

        IN_ClearKeysDown();
        IN_Ack();

        VW_FadeOut();
        screenfaded = true;

        goto cleanup;
    }

    // Read in HEAD chunk
    //
    if (!FindChunk(handle, "HEAD"))
        goto cleanup;

    ReadIt(false, &gamestate, sizeof(gamestate));

    gamestate.msg = NULL;

    ReadIt(false, &gamestuff, sizeof(gamestuff));

    // Reinitialize page manager
    //
#if DUAL_SWAP_FILES
    PM_Shutdown();
    PM_Startup ();
    PM_UnlockMainMem();
#endif


    // Start music for the starting level in this loaded game.
    //

    FreeMusic();
    StartMusic(false);

    // Copy all remaining chunks to PLAYTEMP file
    //
    MakeDestPath(PLAYTEMP_FILE);

    shandle = open(
        tempPath,
        O_CREAT | O_RDWR | O_TRUNC | O_BINARY,
        S_IREAD | S_IWRITE);

    if (shandle == -1)
        goto cleanup;

    while (cksize = NextChunk(handle)) {
        // include chunk ID and LENGTH
        cksize += 8;

        lseek(handle, -8, SEEK_CUR);
        temp = malloc(cksize);

        // read chunk from SAVEGAME file
        IO_FarRead(handle,temp,cksize);

        // write chunk to PLAYTEMP file
        IO_FarWrite(shandle,temp,cksize);

        free(temp);
    }

    close(shandle);
    rt_value = true;

cleanup:
    NewViewSize(viewsize);

    // Load current level
    //
    if (rt_value) {
        LoadLevel(0xff);
        ShowQuickMsg = false;
    }

    return rt_value;
}



//--------------------------------------------------------------------------
// SaveTheGame()
//--------------------------------------------------------------------------

// FIXME
#if 0
boolean SaveTheGame(Sint16 handle, char *description)
{
	struct ffblk finfo;
	Uint32 cksize,offset;
	Sint16 shandle;
	memptr temp;
	char nbuff[GAME_DESCRIPTION_LEN+1];
	boolean rt_value=false,exists;

//
// Save PLAYTEMP becuase we'll want to restore it to the way it was
// before the save.
//
//	IO_CopyFile(PLAYTEMP_FILE,OLD_PLAYTEMP_FILE);
//

// Save current level -- saves it into PLAYTEMP.
//
	SaveLevel(0xff);

// Setup LZH compression
//
	LZH_Startup();
	MM_GetPtr(&lzh_work_buffer,LZH_WORK_BUFFER_SIZE);

// Write VERSion chunk
//
	cksize=sizeof(SavegameInfoText);
	write(handle,"VERS",4);
	write(handle,&cksize,4);
	IO_FarWrite(handle,SavegameInfoText,cksize);

// Write DESC chunk
//
	_fmemcpy(nbuff,description,sizeof(nbuff));
	cksize=strlen(nbuff)+1;
	write(handle,"DESC",4);
	write(handle,&cksize,4);
	write(handle,nbuff,cksize);

// Write HEAD chunk
//
	cksize=0;
	write(handle,"HEAD",4);
	lseek(handle,4,SEEK_CUR);		// leave four bytes for chunk size

	WriteIt(true, &gamestate, sizeof(gamestate));
	WriteIt(true, &gamestuff, sizeof(gamestuff));

	lseek(handle,-(cksize+4),SEEK_CUR);
	write(handle,&cksize,4);
	lseek(handle,cksize,SEEK_CUR);

// Append PLAYTEMP file to savegame file
//
	MakeDestPath(PLAYTEMP_FILE);
	if (findfirst(tempPath,&finfo,0))
		goto cleanup;

	if ((shandle=open(tempPath,O_RDONLY|O_BINARY))==-1)
		goto cleanup;

	IO_CopyHandle(shandle,handle,-1);

	close(shandle);
	rt_value=true;

// Clean-up LZH compression
//
cleanup:;
	MM_FreePtr(&lzh_work_buffer);
	LZH_Shutdown();
	NewViewSize(viewsize);

//
// Return PLAYTEMP to original state!
//
//	remove(PLAYTEMP_FILE);
//	rename(OLD_PLAYTEMP_FILE,PLAYTEMP_FILE);
//

	return(rt_value);
}
#endif // 0

boolean SaveTheGame(int handle, const char* description)
{
    Uint32 cksize;
    int shandle;
    char nbuff[GAME_DESCRIPTION_LEN + 1];
    boolean rt_value = false;

    // Save current level -- saves it into PLAYTEMP.
    //
    SaveLevel(0xff);

    // Write VERSion chunk
    //
    cksize = sizeof(SavegameInfoText);
    write(handle, "VERS", 4);
    write(handle, &cksize, 4);
    IO_FarWrite(handle, SavegameInfoText, cksize);

    // Write DESC chunk
    //
    memcpy(nbuff, description, sizeof(nbuff));
    cksize = strlen(nbuff) + 1;
    write(handle, "DESC", 4);
    write(handle, &cksize, 4);
    write(handle, nbuff, cksize);

    // Write HEAD chunk
    //
    cksize = 0;
    write(handle, "HEAD", 4);

     // leave four bytes for chunk size
    lseek(handle, 4, SEEK_CUR);

    WriteIt(false, &gamestate, sizeof(gamestate));
    WriteIt(false, &gamestuff, sizeof(gamestuff));

    lseek(handle, -(static_cast<long>(cksize)+4), SEEK_CUR);
    write(handle, &cksize, 4);
    lseek(handle,cksize,SEEK_CUR);

    // Append PLAYTEMP file to savegame file
    //
    MakeDestPath(PLAYTEMP_FILE);

    shandle = open(tempPath, O_RDONLY | O_BINARY);

    if (shandle == -1)
        goto cleanup;

    IO_CopyHandle(shandle, handle, -1);

    close(shandle);
    rt_value = true;

cleanup:

    NewViewSize(viewsize);

    return rt_value;
}

//--------------------------------------------------------------------------
// LevelInPlaytemp()
//--------------------------------------------------------------------------
boolean LevelInPlaytemp(char levelnum)
{
	Sint16 handle;
	char chunk[5]="LVxx";
	boolean rt_value;

// Open PLAYTEMP file
//
	MakeDestPath(PLAYTEMP_FILE);
	handle=open(tempPath,O_RDONLY|O_BINARY);

// See if level exists in PLAYTEMP file...
//
	sprintf(&chunk[2],"%02x",levelnum);
	rt_value=FindChunk(handle,chunk);

// Close PLAYTEMP file
//
	close(handle);

	return(rt_value);
}

//--------------------------------------------------------------------------
// CheckDiskSpace()
//--------------------------------------------------------------------------
boolean CheckDiskSpace(Sint32 needed,const char *text,cds_io_type io_type)
{
// FIXME
#if 0
	struct ffblk finfo;
	struct diskfree_t dfree;
	Sint32 avail;

// Figure amount of space free on hard disk and let the gamer know if
// disk space is too low.
//
	if (_dos_getdiskfree(0,&dfree))
		MAIN_ERROR(CHECKDISK_GDFREE);

	avail = (Sint32)dfree.avail_clusters *
					  dfree.bytes_per_sector *
					  dfree.sectors_per_cluster;

	if (avail < needed)
	{
		Uint16 old_DS=_DS;

		switch (io_type)
		{
			case cds_dos_print:
				_DS=FP_SEG(text);
				printf("%s",text);
				_DS=old_DS;
				exit(0);
			break;

			case cds_menu_print:
			case cds_id_print:
				WindowX=0; WindowY=16; WindowW=320; WindowH=168;
				SD_PlaySound (NOWAYSND);
				Message(text);
				IN_ClearKeysDown();
				IN_Ack();
				if (io_type==cds_menu_print)
					MenuFadeOut();
			break;
		}

		return(false);
	}
#endif // 0

	return(true);
}



//--------------------------------------------------------------------------
// CleanUpDoors_N_Actors()
//--------------------------------------------------------------------------

// FIXME
#if 0
void CleanUpDoors_N_Actors(void)
{
	char x,y;
   objtype *obj;
   objtype **actor_ptr;
   Uint8 *tile_ptr;
	Uint16 door;

   actor_ptr = (objtype **)actorat;
	tile_ptr = (Uint8 *)tilemap;

   for (y=0;y<mapheight;y++)
	   for (x=0;x<mapwidth;x++)
      {
      	if (*tile_ptr & 0x80)
         {
         	// Found a door
            //

            obj = *actor_ptr;
            if ((obj >= objlist) && (obj < &objlist[MAXACTORS]))
            {
             	// Found an actor

            	// Determine door number...

	         	door = *tile_ptr & 0x3F;

					if ((obj->flags & (FL_SOLID|FL_DEADGUY)) == (FL_SOLID|FL_DEADGUY))
   	         	obj->flags &= ~(FL_SHOOTABLE | FL_SOLID | FL_FAKE_STATIC);

					// Make sure door is open

					doorobjlist[door].ticcount = 0;
					doorobjlist[door].action = dr_open;
					doorposition[door] = 0xffff;
            }
         }

         tile_ptr++;
      	actor_ptr++;
      }
}
#endif // 0

void CleanUpDoors_N_Actors()
{
    int x;
    int y;
    objtype* actor;
    Uint8 tile;
    Uint16 door;

    for (y = 0; y < mapheight; ++y) {
        for (x = 0; x < mapwidth; ++x) {
            tile = tilemap[y][x];

            if ((tile & 0x80) != 0) {
                // Found a door

                actor = actorat[y][x];

                if (ui16_to_actor((Uint16)actor) != NULL) {
                    // Found an actor

                    door = tile & 0x3F;

                    if ((actor->flags & (FL_SOLID | FL_DEADGUY)) == (FL_SOLID | FL_DEADGUY))
                        actor->flags &= ~(FL_SHOOTABLE | FL_SOLID | FL_FAKE_STATIC);

                    // Make sure door is open

                    doorobjlist[door].ticcount = 0;
                    doorobjlist[door].action = dr_open;
                    doorposition[door] = 0xFFFF;
                }
            }
        }
    }
}


//--------------------------------------------------------------------------
// ClearNClose() - Use when changing levels via standard elevator.
//
//               - This code doesn't CLEAR the elevator door as originally
//                 planned because, actors were coded to stay out of the
//                 elevator doorway.
//
//--------------------------------------------------------------------------
void ClearNClose()
{
	char x,y,tx=0,ty=0,px=player->x>>TILESHIFT,py=player->y>>TILESHIFT;

	// Locate the door.
	//
	for (x=-1; x<2 && !tx; x+=2)
		for (y=-1; y<2; y+=2)
			if (tilemap[px+x][py+y] & 0x80)
			{
				tx=px+x;
				ty=py+y;
				break;
			}

	// Close the door!
	//
	if (tx)
	{
		char doornum=tilemap[tx][ty]&63;

		doorobjlist[doornum].action = dr_closed;		// this door is closed!
		doorposition[doornum]=0;							// draw it closed!

// FIXME
#if 0
		(Uint16)actorat[tx][ty] = doornum | 0x80;	// make it solid!
#endif // 0

        // make it solid!
        actorat[tx][ty] = (objtype*)(doornum | 0x80);
	}
}

//--------------------------------------------------------------------------
// CycleColors()
//--------------------------------------------------------------------------
void CycleColors()
{
	#define NUM_RANGES 	5
	#define CRNG_LOW		0xf0
	#define CRNG_HIGH		0xfe
	#define CRNG_SIZE		(CRNG_HIGH-CRNG_LOW+1)

	static CycleInfo crng[NUM_RANGES] = {{7,0,0xf0,0xf1},
													 {15,0,0xf2,0xf3},
													 {30,0,0xf4,0xf5},
													 {10,0,0xf6,0xf9},
													 {12,0,0xfa,0xfe},
													};

	Uint8 loop,cbuffer[CRNG_SIZE][3];
	boolean changes=false;

	for (loop=0; loop<NUM_RANGES; loop++)
	{
		CycleInfo *c=&crng[loop];

		if (tics >= c->delay_count)
		{
			Uint8 temp[3],first,last,numregs;

			if (!changes)
			{
				VL_GetPalette(CRNG_LOW,CRNG_SIZE,(Uint8 *)cbuffer);
				changes=true;
			}

			first = c->firstreg-CRNG_LOW;
			numregs = c->lastreg-c->firstreg;	// is one less than in range
			last = first+numregs;

			memcpy(temp,cbuffer[last],3);
			memmove(cbuffer[first+1],cbuffer[first],numregs*3);
			memcpy(cbuffer[first],temp,3);

			c->delay_count = c->init_delay;
		}
		else
			c->delay_count -= tics;
	}

	if (changes)
		VL_SetPalette(CRNG_LOW,CRNG_SIZE,(Uint8 *)cbuffer);
	else
		VW_WaitVBL(1);
}


//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ();
}


//===========================================================================


/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/

void CalcProjection (Sint32 focal)
{
	Sint16             i;
	Sint32            intang;
	float   angle;
	double  tang;
	Sint16             halfview;
	double  facedist;


	focallength = focal;
	facedist = focal+MINDIST;
	halfview = viewwidth/2;                                 // half view in pixels

//
// calculate scale value for vertical height calculations
// and sprite x calculations
//
	scale = halfview*facedist/(VIEWGLOBAL/2);

//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>2
//
	heightnumerator = (TILEGLOBAL*scale)>>6;
	minheightdiv = heightnumerator/0x7fff +1;

//
// calculate the angle offset from view angle of each pixel's ray
//

	for (i=0;i<halfview;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		tang = (Sint32)i*VIEWGLOBAL/viewwidth/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}

//
// if a point's abs(y/x) is greater than maxslope, the point is outside
// the view area
//
	maxslope = finetangent[pixelangle[0]];
	maxslope >>= 8;
}



//===========================================================================

//--------------------------------------------------------------------------
// DoMovie()
//--------------------------------------------------------------------------
boolean DoMovie(movie_t movie, void* palette)
{
	boolean  ReturnVal;
//	StopMusic();
	SD_StopSound();

	ClearMemory();
	UnCacheLump(STARTFONT,STARTFONT+NUMFONT);
	CA_LoadAllSounds();

   if (palette)
   	Movies[movie].palette = palette;
   else
   	Movies[movie].palette = vgapal;

	ReturnVal = MOVIE_Play(&Movies[movie]);

	SD_StopSound();
	ClearMemory();
	LoadFonts();

	return(ReturnVal);
}

//===========================================================================

/*
=================
=
= MS_CheckParm
=
=================
*/

boolean MS_CheckParm (char *check)
{
	Sint16             i;
	char    *parm;

	for (i = 1;i<_argc;i++)
	{
		parm = _argv[i];

		while ( !isalpha(*parm) )       // skip - / \ etc.. in front of parm
			if (!*parm++)
				break;                          // hit end of string without an alphanum

		if ( !stricmp(check,parm) )
			return true;
	}

	return false;
}

//===========================================================================

//--------------------------------------------------------------------------
// LoadFonts()
//--------------------------------------------------------------------------
void LoadFonts(void)
{
	CA_CacheGrChunk(STARTFONT+4);
	CA_CacheGrChunk(STARTFONT+2);
}

//===========================================================================

/*
==========================
=
= SetViewSize
=
==========================
*/

boolean SetViewSize (Uint16 width, Uint16 height)
{
	viewwidth = width&~15;                  // must be divisable by 16
	viewheight = height&~1;                 // must be even
	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;
	screenofs = ((200-STATUSLINES-viewheight+TOP_STRIP_HEIGHT)/2*SCREENWIDTH+(320-viewwidth)/8);

//
// calculate trace angles and projection constants
//
	CalcProjection (FOCALLENGTH);

//
// build all needed compiled scalers
//
	SetupScaling (viewwidth*1.5);

	view_xl=0;
	view_xh=view_xl+viewwidth-1;
	view_yl=0;
	view_yh=view_yl+viewheight-1;

	return true;
}


void ShowViewSize (Sint16 width)
{
	Sint16     oldwidth,oldheight;

	oldwidth = viewwidth;
	oldheight = viewheight;

	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;
	VWB_Bar (0,TOP_STRIP_HEIGHT,320,200-STATUSLINES-TOP_STRIP_HEIGHT,BORDER_MED_COLOR);
//	VWB_Bar (0,0,320,200-STATUSLINES,BORDER_MED_COLOR);
	DrawPlayBorder ();

	viewheight = oldheight;
	viewwidth = oldwidth;
}


void NewViewSize (Sint16 width)
{
	CA_UpLevel ();

	viewsize = width;
	while (1)
	{
		if (SetViewSize (width*16,width*16*HEIGHTRATIO))
			break;
		width--;
	};
	CA_DownLevel ();
}


//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/


// FIXME
#if 0
void Quit (const char *error,...)
{
	Uint16        finscreen;
	void*			diz;
	char *screen;
	Uint16 unit,err;
	va_list ap;

	va_start(ap,error);

// FIXME
#if 0
	MakeDestPath(PLAYTEMP_FILE);
	remove(tempPath);
	ClearMemory ();

	if (!*error)
	{
#if GAME_VERSION != SHAREWARE_VERSION
		if (gamestate.flags & GS_BAD_DIZ_FILE)
		{
			char *end;

			CA_CacheGrChunk(DIZ_ERR_TEXT);
			diz = grsegs[DIZ_ERR_TEXT];
			end=strstr(diz,"^XX");
			*end=0;
		}
		else
		if (!IsA386)
		{
			CA_CacheGrChunk (NO386SCREEN);
			screen = MK_FP(grsegs[NO386SCREEN],7);
		}
//		else
#endif

#if 0
		{
			CA_CacheGrChunk (ORDERSCREEN);
			screen = MK_FP(grsegs[ORDERSCREEN],0);
		}
#endif
	}
	else
	{
		CA_CacheGrChunk (ERRORSCREEN);
		screen = MK_FP(grsegs[ERRORSCREEN],7);
	}

	WriteConfig ();
	ShutdownId ();

	if (error && *error)
	{
		FILE *fp;

		unit=va_arg(ap,Uint16);
		err=va_arg(ap,Uint16);
//		movedata ((unsigned)screen,7,0xb800,0,7*160);
		_fmemcpy(MK_FP(0xB800,0), screen, 7*160);

		textcolor(14);
		textbackground(4);
		gotoxy (10,4);
		cprintf(error,unit,err);

		gotoxy (65-strlen(__VERSION__),2);
		cprintf(" Ver:%s ",__VERSION__);

		gotoxy (1,8);

		MakeDestPath("BS_VSI.ERR");
		fp = fopen(tempPath,"wb");
		fprintf(fp,"$%02x%02x",unit,err);
		if (fp)
			fclose(fp);

		exit(1);
	}

#if 0
	if (!error || !(*error))
	{
		Uint16 *clear = MK_FP(0xb800,23*80*2);
		Uint16 len = 0;

		clrscr();
#if GAME_VERSION != SHAREWARE_VERSION
		if (gamestate.flags & GS_BAD_DIZ_FILE)
			fprint(diz);
		else
#endif
		{
//			movedata ((unsigned)screen,0,0xb800,0,4000);
			_fmemcpy(MK_FP(0xB800,0),screen,4000);

			// Far mem set (WORD)! - This is STUPID! Borland SUCKS!

			while (len != 80*2)
			{
				*clear++ = 0x700;
				len++;
			}
			gotoxy (1,24);
		}
	}
#endif
#endif // 0

	va_end(ap);
	exit(0);
}
#endif // 0

void Quit(const char* error, ...)
{
    va_list ap;

    va_start(ap, error);

    MakeDestPath(PLAYTEMP_FILE);
    remove(tempPath);
    ClearMemory();

    WriteConfig();
    ShutdownId();

    if (error != NULL && *error != '\0') {
        char dummy;

        SDL_LogMessageV(
            SDL_LOG_CATEGORY_APPLICATION,
            SDL_LOG_PRIORITY_CRITICAL,
            error, ap);

        scanf("%c", &dummy);
    }

    va_end(ap);
    exit(1);
}


//===========================================================================

/*
=====================
=
= DemoLoop
=
=====================
*/

void    DemoLoop (void)
{
	Sint16 	LastDemo=0;
	boolean breakit;
	Uint16 old_bufferofs;

    // FIXME Just for debugging
    gamestate.flags |= GS_NOWAIT;

	while (1)
	{
		playstate = ex_title;
		if (!screenfaded)
			VW_FadeOut();
		VL_SetPaletteIntensity(0,255,vgapal,0);

		while (!(gamestate.flags & GS_NOWAIT))
		{
			extern boolean sqActive;

		// Start music when coming from menu...
		//
			if (!sqActive)
			{
			// Load and start music
			//
				CA_CacheAudioChunk(STARTMUSIC+TITLE_LOOP_MUSIC);
				SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]);
			}

//
// title page
//
#if !SKIP_TITLE_AND_CREDITS
			breakit = false;

			CA_CacheScreen(TITLE1PIC);
			CA_CacheGrChunk(TITLEPALETTE);
			old_bufferofs = bufferofs;
			bufferofs=displayofs;
			VW_Bar(0,0,320,200,0);
			bufferofs=old_bufferofs;
			VL_SetPalette (0,256,reinterpret_cast<const Uint8*>(grsegs[TITLEPALETTE]));
			VL_SetPaletteIntensity(0,255,reinterpret_cast<const Uint8*>(grsegs[TITLEPALETTE]),0);

			fontnumber = 2;
			PrintX = WindowX = 270;
			PrintY = WindowY = 179;
			WindowW = 29;
			WindowH = 8;
			VWB_Bar(WindowX,WindowY-1,WindowW,WindowH,VERSION_TEXT_BKCOLOR);
			SETFONTCOLOR(VERSION_TEXT_COLOR, VERSION_TEXT_BKCOLOR);
			US_Print(__VERSION__);

			VW_UpdateScreen();
			VL_FadeIn(0,255,reinterpret_cast<Uint8*>(grsegs[TITLEPALETTE]),30);
			UNCACHEGRCHUNK(TITLEPALETTE);
			if (IN_UserInput(TickBase*6))
				breakit= true;

		// Cache screen 2 with Warnings and Copyrights

			CA_CacheScreen(TITLE2PIC);
			fontnumber = 2;
			PrintX = WindowX = 270;
			PrintY = WindowY = 179;
			WindowW = 29;
			WindowH = 8;
			VWB_Bar(WindowX,WindowY-1,WindowW,WindowH,VERSION_TEXT_BKCOLOR);
			SETFONTCOLOR(VERSION_TEXT_COLOR, VERSION_TEXT_BKCOLOR);
			US_Print(__VERSION__);

			// Fizzle whole screen incase of any last minute changes needed
			// on title intro.

// BBi Made abortable.
#if 0
			FizzleFade(bufferofs,displayofs,320,200,70,false);
#endif
            FizzleFade(bufferofs, displayofs, 320, 200, 70, true);
// BBi

			IN_UserInput(TickBase*2);
			if (breakit || IN_UserInput(TickBase*6))
				break;
			VW_FadeOut();

//
// credits page
//
			DrawCreditsPage();
			VW_UpdateScreen();
			VW_FadeIn();
			if (IN_UserInput(TickBase*6))
				break;
			VW_FadeOut();

#endif

//
// demo
//

#if DEMOS_ENABLED
#if IN_DEVELOPMENT
		if (!MS_CheckParm("recdemo"))
#endif
			PlayDemo(LastDemo++%6);

			if (playstate == ex_abort)
				break;
			else
			{
			// Start music when coming from menu...
			//
				if (!sqActive)
//				if (!SD_MusicPlaying())
				{
				// Load and start music
				//
					CA_CacheAudioChunk(STARTMUSIC+TITLE_LOOP_MUSIC);
					SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]);
				}
			}
#endif

//
// high scores
//
#if !SKIP_TITLE_AND_CREDITS
			CA_CacheScreen (BACKGROUND_SCREENPIC);
			DrawHighScores ();
			VW_UpdateScreen ();
			VW_FadeIn ();

			if (IN_UserInput(TickBase*9))
				break;
			VW_FadeOut();
#endif
		}


		if (audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]) {
            free(audiosegs[STARTMUSIC + TITLE_LOOP_MUSIC]);
            audiosegs[STARTMUSIC + TITLE_LOOP_MUSIC] = NULL;
        }

		if (!screenfaded)
			VW_FadeOut();

#ifdef DEMOS_EXTERN
		if (MS_CheckParm("recdemo"))
			RecordDemo ();
		else
#endif
		{
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
			if (gamestate.flags & GS_QUICKRUN)
			{
				ReadGameNames();
				CA_LoadAllSounds();
				NewGame(2,gamestate.episode);
				startgame = true;
			}
			else
#endif													 
				US_ControlPanel (0);
		}
		if (startgame || loadedgame)
			GameLoop ();
	}
}

//-------------------------------------------------------------------------
// DrawCreditsPage()
//-------------------------------------------------------------------------
void DrawCreditsPage()
{
	PresenterInfo pi;

	CA_CacheScreen(BACKGROUND_SCREENPIC);

	memset(&pi,0,sizeof(pi));
	pi.flags = TPF_CACHE_NO_GFX;
	pi.xl=38;
	pi.yl=28;
	pi.xh=281;
	pi.yh=170;
	pi.bgcolor = 2;
	pi.ltcolor = BORDER_HI_COLOR;
	fontcolor = BORDER_TEXT_COLOR;
	pi.shcolor = pi.dkcolor = 0;
	pi.fontnumber=fontnumber;

#ifdef ID_CACHE_CREDITS
	TP_LoadScript(NULL,&pi,CREDITSTEXT);
#else
	TP_LoadScript("CREDITS.TXT",&pi,0);
#endif

	TP_Presenter(&pi);
}


//===========================================================================


extern void JM_FREE_START();
extern void JM_FREE_END();

/*
==========================
=
= main
=
==========================
*/

//char    *nosprtxt[] = {"nospr",nil};
#if IN_DEVELOPMENT || TECH_SUPPORT_VERSION
Sint16 starting_episode=0,starting_level=0,starting_difficulty=2;
#endif
Sint16 debug_value=0;

int main(int argc, char* argv[])
{
    int sdl_result = 0;

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    sdl_result = SDL_Init(0);

    if (sdl_result != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
        exit(1);
    }

    _argc = argc;
    _argv = argv;

#if IN_DEVELOPMENT
	MakeDestPath(ERROR_LOG);
	remove(tempPath);
#endif

	MakeDestPath(PLAYTEMP_FILE);
	remove(tempPath);

	freed_main();

#if FREE_FUNCTIONS
	UseFunc((char *)JM_FREE_START,(char *)JM_FREE_END);
	UseFunc((char *)JM_FREE_DATA_START,(char *)JM_FREE_DATA_END);
#endif

	DemoLoop();

	Quit("");

    return 0;
}

#if FREE_FUNCTIONS

//-------------------------------------------------------------------------
// UseFunc()
//-------------------------------------------------------------------------
unsigned UseFunc(char *first, char *next)
{
	unsigned start,end;
	unsigned pars;

	first += 15;
	next++;
	next--;

	start = FP_SEG(first);
	end = FP_SEG(next);
	if (!FP_OFF(next))
		end--;
	pars = end - start - 1;
	_fmemset(MK_FP(start,0),0,pars*16);
	MML_UseSpace(start,pars);

	return(pars);
}

#endif


//-------------------------------------------------------------------------
// fprint()
//-------------------------------------------------------------------------
void fprint(char *text)
{
	while (*text)
		printf("%c",*text++);
}

// FIXME Make cross-platform
void InitDestPath()
{
    char* env_value;

    env_value = getenv("APOGEECD");

    if (env_value != NULL) {
        size_t len;
        struct _finddata_t fd;
        intptr_t fd_handle;
        Sint16 fd_result;
        boolean fd_found = false;

        len = strlen(env_value);

        if (len > MAX_DEST_PATH_LEN) {
            printf("\nAPOGEECD path too long.\n");
            exit(0);
        }

        strcpy(destPath, env_value);

        if (destPath[len-1] == '\\')
            destPath[len-1] = '\0';

        fd_handle = _findfirst(destPath, &fd);
        fd_result = (fd_handle != -1) ? 0 : -1;

        while ((fd_result == 0) && (!fd_found)) {
            fd_found = ((fd.attrib & _A_SUBDIR) != 0);
            fd_result = _findnext(fd_handle, &fd);
        }

        _findclose(fd_handle);

        if (!fd_found) {
            printf("\nAPOGEECD directory not found.\n");
            exit(0);
        }

        strcat(destPath, "\\");
    } else
        strcpy(destPath, "");
}

//-------------------------------------------------------------------------
// MakeDestPath()
//-------------------------------------------------------------------------
void MakeDestPath(const char *file)
{
	strcpy(tempPath,destPath);
	strcat(tempPath,file);
}

#if IN_DEVELOPMENT

//-------------------------------------------------------------------------
// ShowMemory()
//-------------------------------------------------------------------------
void ShowMemory(void)
{
	Sint32 psize,size;

	size = MM_TotalFree();
	psize = MM_LargestAvail();
	mprintf("Mem free: %ld   %ld\n",size,psize);
}

#endif
