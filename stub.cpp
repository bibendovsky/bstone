#include "3d_def.h"


const int OLD_OBJTYPE_SIZE = 70;
const int OLD_STATOBJ_T_SIZE = 12;
const int OLD_DOOROBJ_T_SIZE = 16;


//
// Offsets of variables in data segment.
//

const int OBJLIST_OFFSET = 0xFFFF - (MAXACTORS * OLD_OBJTYPE_SIZE);
const int STATOBJLIST_OFFSET = 0xFFFF - (MAXSTATS * OLD_STATOBJ_T_SIZE);
const int DOOROBJLIST_OFFSET = 0xFFFF - (MAXDOORS * OLD_DOOROBJ_T_SIZE);


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


Sint8 read_si8(const Uint8** data_ptr)
{
    Sint8 result = *(*data_ptr);
    ++(*data_ptr);
    return result;
}

Uint8 read_ui8(const Uint8** data_ptr)
{
    Uint8 result = *(*data_ptr);
    ++(*data_ptr);
    return result;
}

Sint16 read_si16(const Uint8** data_ptr)
{
    const Sint16* data = (const Sint16*)(*data_ptr);
    Sint16 result = SDL_SwapLE16(*data);
    *data_ptr = (const Uint8*)(data + 1);
    return result;
}

Uint16 read_ui16(const Uint8** data_ptr)
{
    const Uint16* data = (const Uint16*)(*data_ptr);
    Uint16 result = SDL_SwapLE16(*data);
    *data_ptr = (const Uint8*)(data + 1);
    return result;
}

Sint32 read_si32(const Uint8** data_ptr)
{
    const Sint32* data = (const Sint32*)(*data_ptr);
    Sint32 result = SDL_SwapLE32(*data);
    *data_ptr = (const Uint8*)(data + 1);
    return result;
}

Uint32 read_ui32(const Uint8** data_ptr)
{
    const Uint32* data = (const Uint32*)(*data_ptr);
    Uint32 result = SDL_SwapLE32(*data);
    *data_ptr = (const Uint8*)(data + 1);
    return result;
}

void skip_xi8(const Uint8** data_ptr)
{
    *data_ptr += 1;
}

void skip_xi32(const Uint8** data_ptr)
{
    *data_ptr += 4;
}

objtype* ui16_to_actor(Uint16 value)
{
    int offset = value - OBJLIST_OFFSET;

    if (offset >= 0) {
        int index = offset / OLD_OBJTYPE_SIZE;
        assert((offset % OLD_OBJTYPE_SIZE) == 0);
        return &objlist[index];
    }

    return NULL;
}

Uint16 actor_to_ui16(const objtype* actor)
{
    ptrdiff_t index = actor - objlist;

    if (index >= 0) {
        ptrdiff_t offset = OBJLIST_OFFSET + (index * OLD_OBJTYPE_SIZE);
        assert(offset <= 0xFFFF);
        return static_cast<Uint16>(offset);
    }

    return 0;
}

statobj_t* ui16_to_static_object(Uint16 value)
{
    int offset = value - STATOBJLIST_OFFSET;

    if (offset >= 0) {
        int index = offset / OLD_STATOBJ_T_SIZE;
        assert((offset % OLD_STATOBJ_T_SIZE) == 0);
        return &statobjlist[index];
    }

    return NULL;
}

Uint16 static_object_to_ui16(const statobj_t* static_object)
{
    ptrdiff_t index = static_object - statobjlist;

    if (index >= 0) {
        ptrdiff_t offset = STATOBJLIST_OFFSET + (index * OLD_STATOBJ_T_SIZE);
        assert(offset <= 0xFFFF);
        return static_cast<Uint16>(offset);
    }

    return 0;
}

doorobj_t* ui16_to_door_object(Uint16 value)
{
    int offset = value - DOOROBJLIST_OFFSET;

    if (offset >= 0) {
        int index = offset / OLD_DOOROBJ_T_SIZE;
        assert((offset % OLD_DOOROBJ_T_SIZE) == 0);
        return &doorobjlist[index];
    }

    return NULL;
}

Uint16 door_object_to_ui16(const doorobj_t* door_object)
{
    ptrdiff_t index = door_object - doorobjlist;

    if (index >= 0) {
        ptrdiff_t offset = DOOROBJLIST_OFFSET + (index * OLD_DOOROBJ_T_SIZE);
        assert(offset <= 0xFFFF);
        return static_cast<Uint16>(offset);
    }

    return 0;
}
