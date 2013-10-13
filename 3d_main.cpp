// 3D_MAIN.C

#include "3d_def.h"
#ifdef MSVC
#pragma hdrstop
#endif

#include "jm_lzh.h"

#include "bstone_binary_reader.h"
#include "bstone_binary_writer.h"


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


// FIXME
#if 0
int _argc;
char** _argv;
#endif // 0

int g_argc;
char** g_argv;


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

void InitPlaytemp();


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

// FIXME
#if 0
char	configname[13]="CONFIG.";
#endif // 0

const std::string g_config_file_name = "bstone_ps_config";

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
    if (state == NULL)
        return 0;

    for (int i = 1; states_list[i] != NULL; ++i) {
        if (states_list[i] == state)
            return i;
    }

    return -1;
}


// ========================================================================
// ArchiveException

ArchiveException::ArchiveException(
    const char* what) throw() :
    what_(what)
{
}

ArchiveException::ArchiveException(
    const ArchiveException& that) throw() :
        what_(that.what_)
{
}

// (virtual)
ArchiveException::~ArchiveException() throw()
{
}

ArchiveException& ArchiveException::operator=(
    const ArchiveException& that) throw()
{
    what_ = that.what_;
    return *this;
}

// (virtual)
const char* ArchiveException::what() const throw()
{
    return what_;
}

// ArchiveException
// ========================================================================

bstone::MemoryStream g_playtemp;

static bool is_config_loaded = false;
// BBi

/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig()
{
    is_config_loaded = true;

    SDMode sd = sdm_Off;
    SMMode sm = smm_Off;
    SDSMode sds = sds_Off;

    bool is_succeed = true;
    Uint16 flags = gamestate.flags;
    MakeDestPath(g_config_file_name.c_str());

    bstone::FileStream stream(tempPath);

    if (stream.is_open()) {
        Uint32 checksum = 0;
        bstone::BinaryReader reader(&stream);

        try {
            for (int i = 0; i < MaxScores; ++i) {
                HighScore* score = &Scores[i];

                ::deserialize_field(score->name, reader, checksum);
                ::deserialize_field(score->score, reader, checksum);
                ::deserialize_field(score->completed, reader, checksum);
                ::deserialize_field(score->episode, reader, checksum);
                ::deserialize_field(score->ratio, reader, checksum);
            }

            ::deserialize_field(sd, reader, checksum);
            ::deserialize_field(sm, reader, checksum);
            ::deserialize_field(sds, reader, checksum);

            ::deserialize_field(mouseenabled, reader, checksum);
            ::deserialize_field(joystickenabled, reader, checksum);
            ::deserialize_field(joypadenabled, reader, checksum);
            ::deserialize_field(joystickprogressive, reader, checksum);
            ::deserialize_field(joystickport, reader, checksum);

            ::deserialize_field(dirscan, reader, checksum);
            ::deserialize_field(buttonscan, reader, checksum);
            ::deserialize_field(buttonmouse, reader, checksum);
            ::deserialize_field(buttonjoy, reader, checksum);

            ::deserialize_field(viewsize, reader, checksum);
            ::deserialize_field(mouseadjustment, reader, checksum);

            // Use temp so we don't destroy pre-sets.
            ::deserialize_field(flags, reader, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }

        if (is_succeed) {
            Uint32 saved_checksum = 0;
            reader.read(saved_checksum);
            bstone::Endian::lei(saved_checksum);

            is_succeed = (saved_checksum == checksum);
        }
    }

    if (is_succeed) {
        flags &=
            GS_HEARTB_SOUND |
            GS_ATTACK_INFOAREA |
            GS_LIGHTING |
            GS_DRAW_CEILING |
            GS_DRAW_FLOOR; // Mask out the useful flags!

        gamestate.flags |= flags; // Must "OR", some flags are already set.

        if (sd == sdm_AdLib &&
            (!AdLibPresent || !SoundBlasterPresent))
        {
            sd = sdm_PC;
            sd = sdm_Off;
        }

        if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
            (sds == sds_SoundSource && !SoundSourcePresent))
            sds = sds_Off;

        if (!MousePresent)
            mouseenabled = false;

        if (!JoysPresent[joystickport])
            joystickenabled = false;

        MainMenu[6].active = AT_ENABLED;
        MainItems.curpos = 0;
    }

    if (!is_succeed || viewsize == 0) {
        //
        // no config file, so select by hardware
        //
        if (SoundBlasterPresent || AdLibPresent) {
            sd = sdm_AdLib;
            sm = smm_AdLib;
        } else {
            sd = sdm_PC;
            sm = smm_Off;
        }

        if (SoundBlasterPresent)
            sds = sds_SoundBlaster;
        else if (SoundSourcePresent)
            sds = sds_SoundSource;
        else
            sds = sds_Off;

        if (MousePresent)
            mouseenabled = true;

        joystickenabled = false;
        joypadenabled = false;
        joystickport = 0;
        joystickprogressive = false;

        viewsize = 20;
        mouseadjustment = 5;
        gamestate.flags |= GS_HEARTB_SOUND | GS_ATTACK_INFOAREA;

#ifdef CEILING_FLOOR_COLORS
        gamestate.flags |= GS_DRAW_CEILING | GS_DRAW_FLOOR | GS_LIGHTING;
#else
        gamestate.flags |= GS_LIGHTING;
#endif
    }

    ::SD_SetMusicMode(sm);
    ::SD_SetSoundMode(sd);
    ::SD_SetDigiDevice(sds);
}

/*
====================
=
= WriteConfig
=
====================
*/

// FIXME
#if 0
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
#endif // 0

void WriteConfig()
{
    MakeDestPath(g_config_file_name.c_str());

    bstone::FileStream stream(tempPath, bstone::STREAM_OPEN_WRITE);

    if (!stream.is_open()) {
        ::SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Failed to open a config file for writing: %s.",
            tempPath);

        return;
    }

    Uint32 checksum = 0;
    bstone::BinaryWriter writer(&stream);

    for (int i = 0; i < MaxScores; ++i) {
        HighScore* score = &Scores[i];

        ::serialize_field(score->name, writer, checksum);
        ::serialize_field(score->score, writer, checksum);
        ::serialize_field(score->completed, writer, checksum);
        ::serialize_field(score->episode, writer, checksum);
        ::serialize_field(score->ratio, writer, checksum);
    }

    ::serialize_field(SoundMode, writer, checksum);
    ::serialize_field(MusicMode, writer, checksum);
    ::serialize_field(DigiMode, writer, checksum);

    ::serialize_field(mouseenabled, writer, checksum);
    ::serialize_field(joystickenabled, writer, checksum);
    ::serialize_field(joypadenabled, writer, checksum);
    ::serialize_field(joystickprogressive, writer, checksum);
    ::serialize_field(joystickport, writer, checksum);

    ::serialize_field(dirscan, writer, checksum);
    ::serialize_field(buttonscan, writer, checksum);
    ::serialize_field(buttonmouse, writer, checksum);
    ::serialize_field(buttonjoy, writer, checksum);

    ::serialize_field(viewsize, writer, checksum);
    ::serialize_field(mouseadjustment, writer, checksum);
    ::serialize_field(gamestate.flags, writer, checksum);

    writer.write(bstone::Endian::le(checksum));
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

	gamestate.key_floor = static_cast<char>(gamestate.mapon+1);
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

    // BBi
    g_playtemp.set_size(0);
    g_playtemp.set_position(0);
    // BBi
}

//===========================================================================

//==========================================================================
//
//             'LOAD/SAVE game' and 'LOAD/SAVE level' code
//
//==========================================================================

// FIXME
#if 0
boolean LevelInPlaytemp(char levelnum);
#endif // 0

bool LevelInPlaytemp(
    int level_index);

#define WriteIt(c,p,s)	cksize+=WriteInfo(c,(char *)p,s,handle)
#define ReadIt(d,p,s)	ReadInfo(d,(char *)p,s,handle)

#define LZH_WORK_BUFFER_SIZE	8192		

void* lzh_work_buffer;

// FIXME
#if 0
Sint32 checksum;
#endif // 0

//--------------------------------------------------------------------------
// InitPlaytemp()
//--------------------------------------------------------------------------

// FIXME
#if 0
void InitPlaytemp()
{
	Sint16 handle;

	MakeDestPath(PLAYTEMP_FILE);
	if ((handle=open(tempPath,O_CREAT|O_TRUNC|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		MAIN_ERROR(INITPLAYTEMP_OPEN_ERR);

	close(handle);
}
#endif // 0

void InitPlaytemp()
{
    g_playtemp.open(1 * 1024 * 1024);
    g_playtemp.set_size(0);
    g_playtemp.set_position(0);
}

//--------------------------------------------------------------------------
// DoChecksum()
//--------------------------------------------------------------------------
Sint32 DoChecksum(
    const Uint8* source,
    int size,
    Sint32 checksum)
{
    for (int i = 0; i < size - 1; ++i)
        checksum += source[i] ^ source[i + 1];

    return checksum;
}

//--------------------------------------------------------------------------
// FindChunk()
//--------------------------------------------------------------------------

// FIXME
#if 0
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
#endif // 0

int FindChunk(
    bstone::IStream* stream,
    const std::string& chunk_name)
{
    char name_buffer[5];
    name_buffer[4] = '\0';
    std::string name;

    while (true) {
        if (stream->read(name_buffer, 4) != 4)
            break;

        Sint32 chunk_size = 0;

        if (stream->read(&chunk_size, 4) != 4)
            break;

        chunk_size = SDL_SwapLE32(chunk_size);

        name = name_buffer;

        if (name.find(chunk_name) != std::string::npos)
            return chunk_size;

        stream->skip(chunk_size);
    }

    stream->seek(0, bstone::STREAM_SEEK_END);
    return 0;
}

//--------------------------------------------------------------------------
// NextChunk()
//--------------------------------------------------------------------------

// FIXME
#if 0
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
#endif // 0

int NextChunk(
    bstone::IStream* stream)
{
    bool is_succeed = true;

    if (is_succeed) {
        char name_buffer[4];
        is_succeed = (stream->read(name_buffer, 4) == 4);
    }

    Sint32 chunk_size = 0;

    if (is_succeed)
        is_succeed = (stream->read(&chunk_size, 4) == 4);

    if (is_succeed)
        return chunk_size;

    stream->seek(0, bstone::STREAM_SEEK_END);
    return 0;
}

char LS_current=-1,LS_total=-1;

// FIXME
#if 0
//--------------------------------------------------------------------------
// ReadInfo()
//--------------------------------------------------------------------------

void ReadInfo(
    boolean decompress,
    char* dst,
    int size,
    bstone::IStream* stream)
{
    ::PreloadUpdate(LS_current++, LS_total);

    if (decompress) {
        Uint16 csize = 0;
        stream->read(&csize, 2);
        csize = SDL_SwapLE16(csize);
        stream->read(lzh_work_buffer, csize);
        checksum = ::DoChecksum(reinterpret_cast<Uint8*>(lzh_work_buffer),
            csize, checksum);
        int dsize = ::LZH_Decompress(lzh_work_buffer, dst, size, csize);

        if (dsize != size)
            MAIN_ERROR(READINFO_BAD_DECOMP);
    } else {
        stream->read(dst, size);
        checksum = ::DoChecksum(reinterpret_cast<Uint8*>(dst),
            static_cast<Uint16>(size), checksum);
    }
}

//--------------------------------------------------------------------------
// WriteInfo()
//--------------------------------------------------------------------------

int WriteInfo(
    bool compress,
    const char* src,
    int size,
    bstone::IStream* stream)
{
    ::PreloadUpdate(LS_current++, LS_total);

    Uint16 csize;

    if (compress) {
        csize = static_cast<Uint16>(::LZH_Compress(src, lzh_work_buffer, size));

        if (csize > LZH_WORK_BUFFER_SIZE)
            MAIN_ERROR(WRITEINFO_BIGGER_BUF);

        stream->write(&csize, sizeof(csize));
        stream->write(lzh_work_buffer, csize);

        checksum = ::DoChecksum(reinterpret_cast<Uint8*>(lzh_work_buffer),
            csize, checksum);

        csize += sizeof(csize);
    } else {
        stream->write(src, size);

        checksum = ::DoChecksum(
            const_cast<Uint8*>(reinterpret_cast<const Uint8*>(src)),
            static_cast<Uint16>(size), checksum);

        csize = static_cast<Uint16>(size);
    }

    return csize;
}
#endif // 0

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

bool LoadLevel(
    int level_index)
{
    extern boolean ForceLoadDefault;

    boolean oldloaded = loadedgame;

    extern Sint16 nsd_table[];
    extern Sint16 sm_table[];

    WindowY = 181;

    int real_level_index =
        level_index != 0xFF ? level_index : gamestate.mapon;

    gamestuff.level[real_level_index].locked = false;
    int mod = real_level_index % 6;
    normalshade_div = nsd_table[mod];
    shade_max = sm_table[mod];
    normalshade = (3 * (maxscale >> 2)) / normalshade_div;

    std::string chunk_name = "LV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << level_index).to_string();

    g_playtemp.set_position(0);

    if ((::FindChunk(&g_playtemp, chunk_name) == 0) || ForceLoadDefault) {
        ::SetupGameLevel();

        gamestate.flags |= GS_VIRGIN_LEVEL;
        gamestate.turn_around = 0;

        ::PreloadUpdate(1, 1);
        ForceLoadDefault = false;
        return true;
    }

    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Read all sorts of stuff...
    //

    bool is_succeed = true;
    Uint32 checksum = 0;

    loadedgame = true;
    ::SetupGameLevel();
    loadedgame = oldloaded;

    bstone::BinaryReader reader(&g_playtemp);

    try {
        ::deserialize_field(tilemap, reader, checksum);

        for (int i = 0; i < MAPSIZE; ++i) {
            for (int j = 0; j < MAPSIZE; ++j) {
                Sint32 value = 0;
                ::deserialize_field(value, reader, checksum);

                if (value < 0)
                    actorat[i][j] = &objlist[-value];
                else
                    actorat[i][j] = reinterpret_cast<objtype*>(value);
            }
        }

        ::deserialize_field(areaconnect, reader, checksum);
        ::deserialize_field(areabyplayer, reader, checksum);

        // Restore 'save game' actors
        //

        Sint32 actor_count = 0;
        ::deserialize_field(actor_count, reader, checksum);

        if (actor_count < 1 || actor_count >= MAXACTORS)
            throw ArchiveException("actor_count");

        ::InitActorList();
        new_actor->deserialize(reader, checksum);

        for (Sint32 i = 1; i < actor_count; ++i) {
            ::GetNewActor();
            new_actor->deserialize(reader, checksum);
            actorat[new_actor->tilex][new_actor->tiley] = new_actor;

        #if LOOK_FOR_DEAD_GUYS
            if ((new_actor->flags & FL_DEADGUY) != 0)
                DeadGuys[NumDeadGuys++] = new_actor;
        #endif
        }

        //
        //  Re-Establish links to barrier switches
        //

        for (objtype* actor = objlist; actor != NULL;
            actor = actor->next)
        {
            switch (actor->obclass) {
            case arc_barrierobj:
            case post_barrierobj:
            case vspike_barrierobj:
            case vpost_barrierobj:
                actor->temp2 = ::ScanBarrierTable(
                    actor->tilex, actor->tiley);
                break;

            default:
                break;
            }
        }

        ::ConnectBarriers();

        // Read all sorts of stuff...
        //

        Sint32 laststatobj_index = 0;
        ::deserialize_field(laststatobj_index, reader, checksum);

        if (laststatobj_index < 0)
            laststatobj = NULL;
        else
            laststatobj = &statobjlist[laststatobj_index];

        for (int i = 0; i < MAXSTATS; ++i)
            statobjlist[i].deserialize(reader, checksum);

        ::deserialize_field(doorposition, reader, checksum);

        for (int i = 0; i < MAXDOORS; ++i)
            doorobjlist[i].deserialize(reader, checksum);

        ::deserialize_field(pwallstate, reader, checksum);
        ::deserialize_field(pwallx, reader, checksum);
        ::deserialize_field(pwally, reader, checksum);
        ::deserialize_field(pwalldir, reader, checksum);
        ::deserialize_field(pwallpos, reader, checksum);
        ::deserialize_field(pwalldist, reader, checksum);
        ::deserialize_field(TravelTable, reader, checksum);
        ConHintList.deserialize(reader, checksum);

        for (int i = 0; i < MAXEAWALLS; ++i)
            eaList[i].deserialize(reader, checksum);

        GoldsternInfo.deserialize(reader, checksum);

        for (int i = 0; i < GOLDIE_MAX_SPAWNS; ++i)
            GoldieList[i].deserialize(reader, checksum);

        for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
            gamestate.barrier_table[i].deserialize(reader, checksum);

        ::deserialize_field(gamestate.plasma_detonators, reader, checksum);
    } catch (const ArchiveException&) {
        is_succeed = false;
    }

    // Read and evaluate checksum
    //
    if (is_succeed) {
        Uint32 saved_checksum = 0;
        reader.read(saved_checksum);
        bstone::Endian::lei(saved_checksum);

        is_succeed = (saved_checksum == checksum);
    }

    if (!is_succeed) {
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

        ::CacheMessage(BADINFO_TEXT);

        WindowX = old_wx;
        WindowY = old_wy;
        WindowW = old_ww;
        WindowH = old_wh;

        px = old_px;
        py = old_py;

        ::IN_ClearKeysDown();
        ::IN_Ack();

        gamestate.score = 0;
        gamestate.nextextra = EXTRAPOINTS;
        gamestate.lives = 1;

        gamestate.weapon = gamestate.chosenweapon = wp_autocharge;
        gamestate.weapons = 1 << wp_autocharge;

        gamestate.ammo = 8;
    }

    ::NewViewSize(viewsize);

    // Check for Strange Door and Actor combos
    //
    if (is_succeed)
        ::CleanUpDoors_N_Actors();

    return is_succeed;
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

bool SaveLevel(
    int level_index)
{
    WindowY = 181;

    // Make sure floor stats are saved!
    //
    Sint16 oldmapon = gamestate.mapon;
    gamestate.mapon = gamestate.lastmapon;
    ::ShowStats(0, 0, ss_justcalc,
        &gamestuff.level[gamestate.mapon].stats);
    gamestate.mapon = oldmapon;

    // Yeah! We're no longer a virgin!
    //
    gamestate.flags &= ~GS_VIRGIN_LEVEL;

    // Remove level chunk from file
    //
    std::string chunk_name = "LV" + (
        bstone::FormatString() << std::setw(2) << std::setfill('0') <<
        std::hex << std::uppercase << level_index).to_string();

    ::DeleteChunk(g_playtemp, chunk_name);

    g_playtemp.seek(0, bstone::STREAM_SEEK_END);

    // Write level chunk id
    //
    g_playtemp.write(chunk_name.c_str(), 4);

    // leave four bytes for chunk size
    g_playtemp.skip(4);

    Uint32 checksum = 0;
    Sint64 beg_offset = g_playtemp.get_position();

    bstone::BinaryWriter writer(&g_playtemp);

    ::serialize_field(tilemap, writer, checksum);

    //
    // actorat
    //

    for (int i = 0; i < MAPSIZE; ++i) {
        for (int j = 0; j < MAPSIZE; ++j) {
            Sint32 s_value;

            if (actorat[i][j] >= objlist) {
                s_value = -static_cast<Sint32>(
                    actorat[i][j] - objlist);
            } else {
                s_value = static_cast<Sint32>(
                    reinterpret_cast<size_t>(actorat[i][j]));
            }

            ::serialize_field(s_value, writer, checksum);
        }
    }

    ::serialize_field(areaconnect, writer, checksum);
    ::serialize_field(areabyplayer, writer, checksum);

    //
    // objlist
    //

    Sint32 actor_count = 0;
    const objtype* actor = NULL;

    for (actor = player; actor != NULL; actor = actor->next)
        ++actor_count;

    ::serialize_field(actor_count, writer, checksum);

    for (actor = player; actor != NULL; actor = actor->next)
        actor->serialize(writer, checksum);

    //
    // laststatobj
    //

    Sint32 laststatobj_index =
        static_cast<Sint32>(laststatobj - statobjlist);

    ::serialize_field(laststatobj_index, writer, checksum);


    //
    // statobjlist
    //
    for (int i = 0; i < MAXSTATS; ++i)
        statobjlist[i].serialize(writer, checksum);

    //

    ::serialize_field(doorposition, writer, checksum);

    for (int i = 0; i < MAXDOORS; ++i)
        doorobjlist[i].serialize(writer, checksum);

    ::serialize_field(pwallstate, writer, checksum);
    ::serialize_field(pwallx, writer, checksum);
    ::serialize_field(pwally, writer, checksum);
    ::serialize_field(pwalldir, writer, checksum);
    ::serialize_field(pwallpos, writer, checksum);
    ::serialize_field(pwalldist, writer, checksum);
    ::serialize_field(TravelTable, writer, checksum);
    ConHintList.serialize(writer, checksum);

    for (int i = 0; i < MAXEAWALLS; ++i)
        eaList[i].serialize(writer, checksum);

    GoldsternInfo.serialize(writer, checksum);

    for (int i = 0; i < GOLDIE_MAX_SPAWNS; ++i)
        GoldieList[i].serialize(writer, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
        gamestate.barrier_table[i].serialize(writer, checksum);

    ::serialize_field(gamestate.plasma_detonators, writer, checksum);

    // Write checksum and determine size of file
    //
    writer.write(bstone::Endian::le(checksum));

    Sint64 end_offset = g_playtemp.get_position();
    Sint32 chunk_size = static_cast<Sint32>(end_offset - beg_offset);

    // Write chunk size, set file size, and close file
    //
    g_playtemp.seek(-(chunk_size + 4), bstone::STREAM_SEEK_CURRENT);
    writer.write(bstone::Endian::le(chunk_size));
    g_playtemp.set_size(end_offset);

    ::NewViewSize(viewsize);

    return true;
}

//--------------------------------------------------------------------------
// DeleteChunk()
//--------------------------------------------------------------------------

// FIXME
#if 0
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
#endif // 0

int DeleteChunk(
    bstone::MemoryStream& stream,
    const std::string& chunk_name)
{
    stream.set_position(0);

    int chunk_size = ::FindChunk(&stream, chunk_name);

    if (chunk_size > 0) {
        Sint64 offset = stream.get_position() - 8;
        int count = chunk_size + 8;

        stream.remove_block(offset, count);
    }

    return chunk_size;
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
    "SAVEGAME file is from version: "__BLAKE_VERSION__"\n\r"
    " Compile Date :"__DATE__" : "__TIME__"\n\r"
    "-------------------------------------\n\r"
    "\x1a";
#endif // 0

static const std::string SavegameInfoText =
    "bstone (planet strike) save (v" BS_SAVE_VERSION ")";


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

bool LoadTheGame(
    bstone::IStream* stream)
{
    assert(stream != NULL);

    bool is_succeed = true;

    if (is_succeed) {
        is_succeed &= g_playtemp.set_size(0);
        is_succeed &= g_playtemp.set_position(0);
    }

    // Read in VERSion chunk
    //
    if (is_succeed)
        is_succeed = (::FindChunk(stream, "VERS") != 0);

    if (is_succeed) {
        int version_size = static_cast<int>(SavegameInfoText.size());
        std::vector<char> version_buffer;
        version_buffer.resize(version_size);
        stream->read(&version_buffer[0], version_size);
        std::string version(&version_buffer[0], version_size);
        is_succeed = (SavegameInfoText.compare(version) == 0);
    }

    // Read in HEAD chunk
    //
    if (is_succeed)
        is_succeed = (::FindChunk(stream, "HEAD") != 0);

    Uint32 checksum = 0;
    bstone::BinaryReader reader(stream);

    if (is_succeed) {
        try {
            gamestate.deserialize(reader, checksum);
            gamestuff.deserialize(reader, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }
    }

    if (is_succeed) {
        Uint32 saved_checksum = 0;
        reader.read(saved_checksum);
        bstone::Endian::lei(saved_checksum);
        is_succeed = (saved_checksum == checksum);
    }

    if (is_succeed)
        is_succeed = stream->copy_to(&g_playtemp);

    ::NewViewSize(viewsize);

    bool show_error_message = true;

    if (is_succeed) {
#if DUAL_SWAP_FILES
        // Reinitialize page manager
        //
        PM_Shutdown();
        PM_Startup ();
        PM_UnlockMainMem();
#endif

        // Start music for the starting level in this loaded game.
        //
        ::FreeMusic();
        ::StartMusic(false);

        is_succeed = ::LoadLevel(0xFF);

        // Already shown in LoadLevel
        show_error_message = false;
    }

    if (is_succeed) {
        ShowQuickMsg = false;
    } else {
        g_playtemp.set_size(0);
        g_playtemp.set_position(0);

        if (show_error_message) {
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

            ::CacheMessage(BADSAVEGAME_TEXT);

            ::sd_play_player_sound(NOWAYSND, bstone::AC_NO_WAY);

            WindowX = old_wx;
            WindowY = old_wy;
            WindowW = old_ww;
            WindowH = old_wh;

            px = old_px;
            py = old_py;

            ::IN_ClearKeysDown();
            ::IN_Ack();

            ::VW_FadeOut();
            screenfaded = true;
        }
    }

    return is_succeed;
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

bool SaveTheGame(
    bstone::IStream* stream,
    const std::string& description)
{
    bool is_succeed = true;

    Sint32 cksize;

    // Save current level -- saves it into PLAYTEMP.
    //
    ::SaveLevel(0xFF);

    // Write VERSion chunk
    //
    cksize = static_cast<Sint32>(SavegameInfoText.size());
    is_succeed &= stream->write("VERS", 4);
    is_succeed &= stream->write(&cksize, 4);
    is_succeed &= stream->write(SavegameInfoText.c_str(), cksize);

    // Write DESC chunk
    //
    std::vector<char> desc_buffer(description.begin(), description.end());
    desc_buffer.resize(GAME_DESCRIPTION_LEN + 1);
    cksize = static_cast<Sint32>(desc_buffer.size());
    is_succeed &= stream->write("DESC", 4);
    is_succeed &= stream->write(&cksize, 4);
    is_succeed &= stream->write(&desc_buffer[0], cksize);

    // Write HEAD chunk
    //
    cksize = 0;
    is_succeed &= stream->write("HEAD", 4);

    // leave four bytes for chunk size
    is_succeed &= (stream->skip(4) >= 0);

    Uint32 checksum = 0;
    bstone::BinaryWriter writer(stream);

    Sint64 beg_position = stream->get_position();

    is_succeed &= (beg_position >= 0);

    if (is_succeed) {
        try {
            gamestate.serialize(writer, checksum);
            gamestuff.serialize(writer, checksum);
        } catch (const ArchiveException&) {
            is_succeed = false;
        }
    }

    is_succeed &= writer.write(bstone::Endian::le(checksum));

    Sint64 end_position = stream->get_position();
    is_succeed &= (end_position >= 0);

    if (is_succeed) {
        cksize = static_cast<Sint32>(end_position - beg_position);

        stream->seek(-(cksize + 4), bstone::STREAM_SEEK_CURRENT);
        stream->write(&cksize, 4);
        stream->seek(cksize, bstone::STREAM_SEEK_CURRENT);

        g_playtemp.set_position(0);
        is_succeed = g_playtemp.copy_to(stream);
    }

    ::NewViewSize(viewsize);

    return is_succeed;
}

//--------------------------------------------------------------------------
// LevelInPlaytemp()
//--------------------------------------------------------------------------

// FIXME
#if 0
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
#endif // 0

bool LevelInPlaytemp(
    int level_index)
{
    char chunk[] = "LVxx";
    ::sprintf(&chunk[2], "%02x", level_index);
    return ::FindChunk(&g_playtemp, chunk) != 0;
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
                Uint16 actor_u16 = static_cast<Uint16>(
                    reinterpret_cast<size_t>(actor));

                if (ui16_to_actor(actor_u16) != NULL) {
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
	char x,y,tx=0,ty=0,px=static_cast<char>(player->x>>TILESHIFT),py=static_cast<char>(player->y>>TILESHIFT);

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
		char doornum=tilemap[static_cast<int>(tx)][static_cast<int>(ty)]&63;

		doorobjlist[static_cast<int>(doornum)].action = dr_closed;		// this door is closed!
		doorposition[static_cast<int>(doornum)]=0;							// draw it closed!

// FIXME
#if 0
		(Uint16)actorat[tx][ty] = doornum | 0x80;	// make it solid!
#endif // 0

        // make it solid!
        actorat[static_cast<int>(tx)][static_cast<int>(ty)] = reinterpret_cast<objtype*>(doornum | 0x80);
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
			c->delay_count -= static_cast<Uint8>(tics);
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
	scale = static_cast<fixed>(halfview*facedist/(VIEWGLOBAL/2));

//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>2
//
	heightnumerator = (TILEGLOBAL*scale)>>6;
	minheightdiv = static_cast<Sint16>(heightnumerator/0x7fff +1);

//
// calculate the angle offset from view angle of each pixel's ray
//

	for (i=0;i<halfview;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		tang = (Sint32)i*VIEWGLOBAL/viewwidth/facedist;
		angle = static_cast<float>(atan(tang));
		intang = static_cast<Sint32>(angle*radtoint);
		pixelangle[halfview-1-i] = static_cast<Sint16>(intang);
		pixelangle[halfview+i] = static_cast<Sint16>(-intang);
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

	for (i = 1;i<g_argc;i++)
	{
		parm = g_argv[i];

		while ( !isalpha(*parm) )       // skip - / \ etc.. in front of parm
			if (!*parm++)
				break;                          // hit end of string without an alphanum

		if (bstone::C::stricmp(check,parm) == 0)
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
	SetupScaling (static_cast<Sint16>(viewwidth*1.5));

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
	viewheight = static_cast<Sint16>(width*16*HEIGHTRATIO);
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
		if (SetViewSize (width*16,static_cast<Uint16>(width*16*HEIGHTRATIO)))
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

		gotoxy (65-strlen(__BLAKE_VERSION__),2);
		cprintf(" Ver:%s ",__BLAKE_VERSION__);

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

// FIXME
#if 0
    MakeDestPath(PLAYTEMP_FILE);
    remove(tempPath);
#endif // 0

    ClearMemory();

    if (is_config_loaded)
        ::WriteConfig();

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
#if DEMOS_ENABLED
	Sint16 	LastDemo=0;
#endif // DEMOS_ENABLED

	boolean breakit;
	Uint16 old_bufferofs;

    // FIXME Just for debugging
    gamestate.flags |= GS_NOWAIT;

	while (true)
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

// FIXME
#if 0
				SD_StartMusic((MusicGroup *)audiosegs[STARTMUSIC+TITLE_LOOP_MUSIC]);
#endif // 0

                ::SD_StartMusic(TITLE_LOOP_MUSIC);
			}

//
// title page
//
#if !SKIP_TITLE_AND_CREDITS
			breakit = false;

			CA_CacheScreen(TITLE1PIC);
			CA_CacheGrChunk(TITLEPALETTE);
			old_bufferofs = static_cast<Uint16>(bufferofs);
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
			US_Print(__BLAKE_VERSION__);

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
			US_Print(__BLAKE_VERSION__);

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
            delete [] audiosegs[STARTMUSIC + TITLE_LOOP_MUSIC];
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
	pi.fontnumber=static_cast<char>(fontnumber);

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

    g_argc = argc;
    g_argv = argv;

#if IN_DEVELOPMENT
	MakeDestPath(ERROR_LOG);
	remove(tempPath);
#endif

// FIXME
#if 0
	MakeDestPath(PLAYTEMP_FILE);
	remove(tempPath);
#endif // 0

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
#if 0
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
#endif // 0

void InitDestPath()
{
    destPath[0] = '\0';
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


// BBi
void objtype::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);
    ::serialize_field(active, writer, checksum);
    ::serialize_field(ticcount, writer, checksum);
    ::serialize_field(obclass, writer, checksum);

    Sint32 state_index = static_cast<Sint32>(::get_state_index(state));
    ::serialize_field(state_index, writer, checksum);

    ::serialize_field(flags, writer, checksum);
    ::serialize_field(flags2, writer, checksum);
    ::serialize_field(distance, writer, checksum);
    ::serialize_field(dir, writer, checksum);
    ::serialize_field(trydir, writer, checksum);
    ::serialize_field(x, writer, checksum);
    ::serialize_field(y, writer, checksum);
    ::serialize_field(s_tilex, writer, checksum);
    ::serialize_field(s_tiley, writer, checksum);
    ::serialize_field(viewx, writer, checksum);
    ::serialize_field(viewheight, writer, checksum);
    ::serialize_field(transx, writer, checksum);
    ::serialize_field(transy, writer, checksum);
    ::serialize_field(hitpoints, writer, checksum);
    ::serialize_field(ammo, writer, checksum);
    ::serialize_field(lighting, writer, checksum);
    ::serialize_field(linc, writer, checksum);
    ::serialize_field(angle, writer, checksum);
    ::serialize_field(speed, writer, checksum);
    ::serialize_field(temp1, writer, checksum);
    ::serialize_field(temp2, writer, checksum);
    ::serialize_field(temp3, writer, checksum);
}

void objtype::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);
    ::deserialize_field(active, reader, checksum);
    ::deserialize_field(ticcount, reader, checksum);
    ::deserialize_field(obclass, reader, checksum);

    Sint32 state_index = 0;
    ::deserialize_field(state_index, reader, checksum);
    state = states_list[state_index];

    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(flags2, reader, checksum);
    ::deserialize_field(distance, reader, checksum);
    ::deserialize_field(dir, reader, checksum);
    ::deserialize_field(trydir, reader, checksum);
    ::deserialize_field(x, reader, checksum);
    ::deserialize_field(y, reader, checksum);
    ::deserialize_field(s_tilex, reader, checksum);
    ::deserialize_field(s_tiley, reader, checksum);
    ::deserialize_field(viewx, reader, checksum);
    ::deserialize_field(viewheight, reader, checksum);
    ::deserialize_field(transx, reader, checksum);
    ::deserialize_field(transy, reader, checksum);
    ::deserialize_field(hitpoints, reader, checksum);
    ::deserialize_field(ammo, reader, checksum);
    ::deserialize_field(lighting, reader, checksum);
    ::deserialize_field(linc, reader, checksum);
    ::deserialize_field(angle, reader, checksum);
    ::deserialize_field(speed, reader, checksum);
    ::deserialize_field(temp1, reader, checksum);
    ::deserialize_field(temp2, reader, checksum);
    ::deserialize_field(temp3, reader, checksum);
}

void statobj_t::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);

    Sint32 vis_index = static_cast<Sint32>(visspot - &spotvis[0][0]);
    ::serialize_field(vis_index, writer, checksum);

    ::serialize_field(shapenum, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(itemnumber, writer, checksum);
    ::serialize_field(lighting, writer, checksum);
}

void statobj_t::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);

    Sint32 vis_index = 0;
    ::deserialize_field(vis_index, reader, checksum);

    if (vis_index < 0)
        visspot = NULL;
    else
        visspot = &(&spotvis[0][0])[vis_index];

    ::deserialize_field(shapenum, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(itemnumber, reader, checksum);
    ::deserialize_field(lighting, reader, checksum);
}

void doorobj_t::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(vertical, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(lock, writer, checksum);
    ::serialize_field(type, writer, checksum);
    ::serialize_field(action, writer, checksum);
    ::serialize_field(ticcount, writer, checksum);
    ::serialize_field(areanumber, writer, checksum);
}

void doorobj_t::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(vertical, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(lock, reader, checksum);
    ::deserialize_field(type, reader, checksum);
    ::deserialize_field(action, reader, checksum);
    ::deserialize_field(ticcount, reader, checksum);
    ::deserialize_field(areanumber, reader, checksum);
}

void mCacheInfo::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(local_val, writer, checksum);
    ::serialize_field(global_val, writer, checksum);
}

void mCacheInfo::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(local_val, reader, checksum);
    ::deserialize_field(global_val, reader, checksum);
    mSeg = NULL;
}

void con_mCacheInfo::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    mInfo.serialize(writer, checksum);
    ::serialize_field(type, writer, checksum);
    ::serialize_field(operate_cnt, writer, checksum);
}

void con_mCacheInfo::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    mInfo.deserialize(reader, checksum);
    ::deserialize_field(type, reader, checksum);
    ::deserialize_field(operate_cnt, reader, checksum);
}

void concession_t::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(NumMsgs, writer, checksum);

    for (int i = 0; i < NumMsgs; ++i)
        cmInfo[i].serialize(writer, checksum);
}

void concession_t::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(NumMsgs, reader, checksum);

    for (int i = 0; i < NumMsgs; ++i)
        cmInfo[i].deserialize(reader, checksum);
}

void eaWallInfo::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
    ::serialize_field(aliens_out, writer, checksum);
    ::serialize_field(delay, writer, checksum);
}

void eaWallInfo::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
    ::deserialize_field(aliens_out, reader, checksum);
    ::deserialize_field(delay, reader, checksum);
}

void GoldsternInfo_t::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(LastIndex, writer, checksum);
    ::serialize_field(SpawnCnt, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(WaitTime, writer, checksum);
    ::serialize_field(GoldSpawned, writer, checksum);
}

void GoldsternInfo_t::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(LastIndex, reader, checksum);
    ::deserialize_field(SpawnCnt, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(WaitTime, reader, checksum);
    ::deserialize_field(GoldSpawned, reader, checksum);
}

void tilecoord_t::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(tilex, writer, checksum);
    ::serialize_field(tiley, writer, checksum);
}

void tilecoord_t::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(tilex, reader, checksum);
    ::deserialize_field(tiley, reader, checksum);
}

void barrier_type::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    coord.serialize(writer, checksum);
    ::serialize_field(on, writer, checksum);
}

void barrier_type::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    coord.deserialize(reader, checksum);
    ::deserialize_field(on, reader, checksum);
}

void statsInfoType::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(total_points, writer, checksum);
    ::serialize_field(accum_points, writer, checksum);
    ::serialize_field(total_enemy, writer, checksum);
    ::serialize_field(accum_enemy, writer, checksum);
    ::serialize_field(total_inf, writer, checksum);
    ::serialize_field(accum_inf, writer, checksum);
    ::serialize_field(overall_floor, writer, checksum);
}

void statsInfoType::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(total_points, reader, checksum);
    ::deserialize_field(accum_points, reader, checksum);
    ::deserialize_field(total_enemy, reader, checksum);
    ::deserialize_field(accum_enemy, reader, checksum);
    ::deserialize_field(total_inf, reader, checksum);
    ::deserialize_field(accum_inf, reader, checksum);
    ::deserialize_field(overall_floor, reader, checksum);
}

void levelinfo::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(bonus_queue, writer, checksum);
    ::serialize_field(bonus_shown, writer, checksum);
    ::serialize_field(locked, writer, checksum);
    stats.serialize(writer, checksum);
    ::serialize_field(ptilex, writer, checksum);
    ::serialize_field(ptiley, writer, checksum);
    ::serialize_field(pangle, writer, checksum);
}

void levelinfo::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(bonus_queue, reader, checksum);
    ::deserialize_field(bonus_shown, reader, checksum);
    ::deserialize_field(locked, reader, checksum);
    stats.deserialize(reader, checksum);
    ::deserialize_field(ptilex, reader, checksum);
    ::deserialize_field(ptiley, reader, checksum);
    ::deserialize_field(pangle, reader, checksum);
}

void fargametype::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    for (int i = 0; i < MAPS_PER_EPISODE; ++i)
        old_levelinfo[i].serialize(writer, checksum);

    for (int i = 0; i < MAPS_PER_EPISODE; ++i)
        level[i].serialize(writer, checksum);
}

void fargametype::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    for (int i = 0; i < MAPS_PER_EPISODE; ++i)
        old_levelinfo[i].deserialize(reader, checksum);

    for (int i = 0; i < MAPS_PER_EPISODE; ++i)
        level[i].deserialize(reader, checksum);
}

void gametype::serialize(
    bstone::BinaryWriter& writer,
    Uint32& checksum) const
{
    ::serialize_field(turn_around, writer, checksum);
    ::serialize_field(turn_angle, writer, checksum);
    ::serialize_field(flags, writer, checksum);
    ::serialize_field(lastmapon, writer, checksum);
    ::serialize_field(difficulty, writer, checksum);
    ::serialize_field(mapon, writer, checksum);
    ::serialize_field(status_refresh, writer, checksum);
    ::serialize_field(oldscore, writer, checksum);
    ::serialize_field(tic_score, writer, checksum);
    ::serialize_field(score, writer, checksum);
    ::serialize_field(nextextra, writer, checksum);
    ::serialize_field(score_roll_wait, writer, checksum);
    ::serialize_field(lives, writer, checksum);
    ::serialize_field(health, writer, checksum);
    ::serialize_field(health_delay, writer, checksum);
    ::serialize_field(health_str, writer, checksum);
    ::serialize_field(rpower, writer, checksum);
    ::serialize_field(old_rpower, writer, checksum);
    ::serialize_field(rzoom, writer, checksum);
    ::serialize_field(radar_leds, writer, checksum);
    ::serialize_field(lastradar_leds, writer, checksum);
    ::serialize_field(lastammo_leds, writer, checksum);
    ::serialize_field(ammo_leds, writer, checksum);
    ::serialize_field(ammo, writer, checksum);
    ::serialize_field(old_ammo, writer, checksum);
    ::serialize_field(plasma_detonators, writer, checksum);
    ::serialize_field(old_plasma_detonators, writer, checksum);
    ::serialize_field(useable_weapons, writer, checksum);
    ::serialize_field(weapons, writer, checksum);
    ::serialize_field(weapon, writer, checksum);
    ::serialize_field(chosenweapon, writer, checksum);
    ::serialize_field(old_weapons, writer, checksum);
    ::serialize_field(key_floor, writer, checksum);
    ::serialize_field(weapon_wait, writer, checksum);
    ::serialize_field(attackframe, writer, checksum);
    ::serialize_field(attackcount, writer, checksum);
    ::serialize_field(weaponframe, writer, checksum);
    ::serialize_field(episode, writer, checksum);

    Uint32 time_count = TimeCount;
    ::serialize_field(time_count, writer, checksum);

    ::serialize_field(killx, writer, checksum);
    ::serialize_field(killy, writer, checksum);
    // Skip "msg"
    ::serialize_field(numkeys, writer, checksum);
    ::serialize_field(old_numkeys, writer, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
        barrier_table[i].serialize(writer, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
        old_barrier_table[i].serialize(writer, checksum);

    ::serialize_field(tokens, writer, checksum);
    ::serialize_field(old_tokens, writer, checksum);
    ::serialize_field(boss_key_dropped, writer, checksum);
    ::serialize_field(old_boss_key_dropped, writer, checksum);
    ::serialize_field(wintilex, writer, checksum);
    ::serialize_field(wintiley, writer, checksum);
}

void gametype::deserialize(
    bstone::BinaryReader& reader,
    Uint32& checksum)
{
    ::deserialize_field(turn_around, reader, checksum);
    ::deserialize_field(turn_angle, reader, checksum);
    ::deserialize_field(flags, reader, checksum);
    ::deserialize_field(lastmapon, reader, checksum);
    ::deserialize_field(difficulty, reader, checksum);
    ::deserialize_field(mapon, reader, checksum);
    ::deserialize_field(status_refresh, reader, checksum);
    ::deserialize_field(oldscore, reader, checksum);
    ::deserialize_field(tic_score, reader, checksum);
    ::deserialize_field(score, reader, checksum);
    ::deserialize_field(nextextra, reader, checksum);
    ::deserialize_field(score_roll_wait, reader, checksum);
    ::deserialize_field(lives, reader, checksum);
    ::deserialize_field(health, reader, checksum);
    ::deserialize_field(health_delay, reader, checksum);
    ::deserialize_field(health_str, reader, checksum);
    ::deserialize_field(rpower, reader, checksum);
    ::deserialize_field(old_rpower, reader, checksum);
    ::deserialize_field(rzoom, reader, checksum);
    ::deserialize_field(radar_leds, reader, checksum);
    ::deserialize_field(lastradar_leds, reader, checksum);
    ::deserialize_field(lastammo_leds, reader, checksum);
    ::deserialize_field(ammo_leds, reader, checksum);
    ::deserialize_field(ammo, reader, checksum);
    ::deserialize_field(old_ammo, reader, checksum);
    ::deserialize_field(plasma_detonators, reader, checksum);
    ::deserialize_field(old_plasma_detonators, reader, checksum);
    ::deserialize_field(useable_weapons, reader, checksum);
    ::deserialize_field(weapons, reader, checksum);
    ::deserialize_field(weapon, reader, checksum);
    ::deserialize_field(chosenweapon, reader, checksum);
    ::deserialize_field(old_weapons, reader, checksum);
    ::deserialize_field(key_floor, reader, checksum);
    ::deserialize_field(weapon_wait, reader, checksum);
    ::deserialize_field(attackframe, reader, checksum);
    ::deserialize_field(attackcount, reader, checksum);
    ::deserialize_field(weaponframe, reader, checksum);
    ::deserialize_field(episode, reader, checksum);

    Uint32 time_count = 0;
    ::deserialize_field(time_count, reader, checksum);

    ::deserialize_field(killx, reader, checksum);
    ::deserialize_field(killy, reader, checksum);
    msg = NULL;
    ::deserialize_field(numkeys, reader, checksum);
    ::deserialize_field(old_numkeys, reader, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
        barrier_table[i].deserialize(reader, checksum);

    for (int i = 0; i < MAX_BARRIER_SWITCHES; ++i)
        old_barrier_table[i].deserialize(reader, checksum);

    ::deserialize_field(tokens, reader, checksum);
    ::deserialize_field(old_tokens, reader, checksum);
    ::deserialize_field(boss_key_dropped, reader, checksum);
    ::deserialize_field(old_boss_key_dropped, reader, checksum);
    ::deserialize_field(wintilex, reader, checksum);
    ::deserialize_field(wintiley, reader, checksum);

    TimeCount = time_count;
}
// BBi
