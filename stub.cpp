#include "3d_def.h"


//
// Offsets of variables in data segment.
//

const int OBJLIST_OFFSET = 0xFFFF - MAXACTORS;
const int STATOBJLIST_OFFSET = 0xFFFF - MAXSTATS;
const int DOOROBJLIST_OFFSET = 0xFFFF - MAXDOORS;


void ogl_update_screen();


void VL_WaitVBL (Uint32 vbls)
{
    SDL_Delay(8 * vbls);
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

objtype* ui16_to_actor(
    Uint16 value)
{
    int index = value - OBJLIST_OFFSET;

    if (index < 0)
        return NULL;

    if (index >= MAXACTORS)
        return NULL;

    return &objlist[index];
}

Uint16 actor_to_ui16(
    const objtype* actor)
{
    ptrdiff_t index = actor - objlist;

    if (index < 0)
        return 0;

    if (index >= MAXACTORS)
        return 0;

    return static_cast<Uint16>(index + OBJLIST_OFFSET);
}

statobj_t* ui16_to_static_object(
    Uint16 value)
{
    int index = value - STATOBJLIST_OFFSET;

    if (index < 0)
        return NULL;

    if (index >= MAXSTATS)
        return NULL;

    return &statobjlist[index];
}

Uint16 static_object_to_ui16(
    const statobj_t* static_object)
{
    ptrdiff_t index = static_object - statobjlist;

    if (index < 0)
        return 0;

    if (index >= MAXSTATS)
        return 0;

    return static_cast<Uint16>(index + STATOBJLIST_OFFSET);
}

doorobj_t* ui16_to_door_object(
    Uint16 value)
{
    int index = value - DOOROBJLIST_OFFSET;

    if (index < 0)
        return NULL;

    if (index >= MAXDOORS)
        return NULL;

    return &doorobjlist[index];
}

Uint16 door_object_to_ui16(
    const doorobj_t* door_object)
{
    ptrdiff_t index = door_object - doorobjlist;

    if (index < 0)
        return 0;

    if (index >= MAXDOORS)
        return 0;

    return static_cast<Uint16>(index + DOOROBJLIST_OFFSET);
}
