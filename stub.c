#include "SDL.h"


void ogl_update_screen();


void VL_WaitVBL (Sint16 vbls)
{
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

void JM_VGALinearFill(Uint16 start, Uint16 length, char fill)
{
}
