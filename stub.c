#include "3d_def.h"


const Uint16 UI16_ACTOR_MASK = 0x8000;
const Uint16 UI16_STATIC_OBJECT_MASK = 0x4000;
const Uint16 UI16_DOOR_OBJECT_MASK = 0x2000;


void ogl_update_screen();


void VL_WaitVBL (Uint32 vbls)
{
    SDL_Delay(8 * vbls);
}

void VH_UpdateScreen()
{
    ogl_update_screen();
}

void VL_ScreenToScreen (Uint16 source, Uint16 dest, Sint16 width, Sint16 height)
{
}

void SDL_SetDS()
{
}

void SDL_t0FastAsmService()
{
}

void SDL_t0SlowAsmService()
{
}

Sint16 VL_VideoID ()
{
    return 5;
}


objtype* ui16_to_actor(Uint16 value)
{
    if ((value & UI16_ACTOR_MASK) != 0) {
        Uint16 index = value ^ UI16_ACTOR_MASK;

        if (index < MAXACTORS)
            return &objlist[index];
    }

    return NULL;
}

Uint16 actor_to_ui16(const objtype* actor)
{
    intptr_t index = actor - objlist;

    if (index >= 0 && index < MAXACTORS)
        return index | UI16_ACTOR_MASK;

    return 0;
}

statobj_t* ui16_to_static_object(Uint16 value)
{
    if ((value & UI16_STATIC_OBJECT_MASK) != 0) {
        Uint16 index = value ^ UI16_STATIC_OBJECT_MASK;

        if (index < MAXSTATS)
            return &statobjlist[index];
    }

    return NULL;
}

Uint16 static_object_to_ui16(const statobj_t* static_object)
{
    intptr_t index = static_object - statobjlist;

    if (index >= 0 && index < MAXSTATS);
        return index | UI16_STATIC_OBJECT_MASK;

    return 0;
}

doorobj_t* ui16_to_door_object(Uint16 value)
{
    if ((value & UI16_DOOR_OBJECT_MASK) != 0) {
        Uint16 index = value ^ UI16_DOOR_OBJECT_MASK;

        if (index < MAXDOORS)
            return &doorobjlist[index];
    }

    return NULL;
}

Uint16 door_object_to_ui16(const doorobj_t* door_object)
{
    intptr_t index = door_object - doorobjlist;

    if (index >= 0 && index < MAXDOORS)
        return index | UI16_DOOR_OBJECT_MASK;

    return 0;
}
