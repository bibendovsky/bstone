
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2018 fgsfds
// Copyright(C) 2018 01y
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
// modified versions of controller routines from i_video.c and txt_sdl.c in vita chocolate doom
//

#include "SDL.h"

extern int in_mouse_dx;
extern int in_mouse_dy;
extern int control2x;
extern bool vid_is_ui_stretched;

void TranslateControllerEvent(SDL_Event *ev)
{
    int btn;
    SDL_Event ev_new;
    static const struct 
    {
        SDL_Keycode sym;
        SDL_Scancode scan;
    } v_keymap[] = 
    {
        { SDLK_y, SDL_SCANCODE_Y },                 // Triangle
//        { SDLK_BACKSPACE, SDL_SCANCODE_BACKSPACE},
        { SDLK_TAB, SDL_SCANCODE_TAB },             // Circle
        { SDLK_RETURN, SDL_SCANCODE_RETURN },       // Cross
        { SDLK_SPACE, SDL_SCANCODE_SPACE },         // Square
        { SDLK_SPACE, SDL_SCANCODE_SPACE },         // L Trigger
        { SDLK_y, SDL_SCANCODE_Y },                 // R Trigger
        { SDLK_DOWN, SDL_SCANCODE_DOWN },           // D-Down
        { SDLK_LEFT, SDL_SCANCODE_LEFT },           // D-Left
        { SDLK_UP, SDL_SCANCODE_UP },               // D-Up
        { SDLK_RIGHT, SDL_SCANCODE_RIGHT },         // D-Right
        { SDLK_TAB, SDL_SCANCODE_TAB },             // Select
        { SDLK_ESCAPE, SDL_SCANCODE_ESCAPE },       // Start
    };
    
    memset(&ev_new, 0, sizeof(SDL_Event));
    btn = ev->jbutton.button;

    if (btn < 0 || btn > 11)
        return;
    ev_new.key.keysym.sym = v_keymap[btn].sym;
    ev_new.key.keysym.scancode = v_keymap[btn].scan;

    if (ev->type == SDL_JOYBUTTONDOWN)
    {
        ev_new.type = ev_new.key.type = SDL_KEYDOWN;
        ev_new.key.state = SDL_PRESSED;
    }
    else if (ev->type == SDL_JOYBUTTONUP)
    {
        ev_new.type = ev_new.key.type = SDL_KEYUP;
        ev_new.key.state = SDL_RELEASED;
    }

    SDL_PushEvent(&ev_new);
}

void TranslateTouchEvent(SDL_Event *ev)
{
    SDL_Event ev_new;

    memset(&ev_new, 0, sizeof(SDL_Event));

    float w = 960.0F; //screen width
    float m = 760.0F; //midpoint between rows
    float h = 544.0F; //screen height
    float fingerx = ev->tfinger.x;
    float fingery = ev->tfinger.y;
    if (vid_is_ui_stretched) // adjusts for stretched screen, for expressions of the form (c/w)
    {
        w *=  0.75F ; // (4/3) / (16/9)
        fingerx += 0.166667F; // (1/6), compensates for 4:3 mode being centered on the vita's screen, as opposed to left flushed
    }
    // front touch
    if (ev->tfinger.touchId == 0)
    {
        if (fingerx > 660.0F / w && fingerx < 860.0F / w)
        //column containing elevator buttons
        {
            if (fingery > 50.0F / h  && fingery <= 139.0F / h)
            //9,10  50-140 
            {
                if (fingerx < m/w )
                {
                        ev_new.key.keysym.sym = SDLK_9;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_9;
                }
                else
                {
                        ev_new.key.keysym.sym = SDLK_0;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_0;
                }
            }
            //7,8   140-194
            if (fingery > 139.0F / h  && fingery <= 194.0F / h)
            {
                if (fingerx < m/w )
                {
                        ev_new.key.keysym.sym = SDLK_7;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_7;
                }
                else
                {
                        ev_new.key.keysym.sym = SDLK_8;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_8;
                }
            }
            //5,6   194-249
            if (fingery > 194.0F / h  && fingery <= 249.0F / h)
            {
                if (fingerx < m/w )
                {
                        ev_new.key.keysym.sym = SDLK_5;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_5;
                }
                else
                {
                        ev_new.key.keysym.sym = SDLK_6;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_6;
                }
            }
            //3,4   249-303
            if (fingery > 249.0F / h  && fingery <= 304.0F / h)
            {
                if (fingerx < m/w )
                {
                        ev_new.key.keysym.sym = SDLK_3;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_3;
                }
                else
                {
                        ev_new.key.keysym.sym = SDLK_4;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_4;
                }
            }
            //1,2   303-410
            if (fingery > 304.0F / h  && fingery <= 410.0F / h)
            {
                if (fingerx < m/w )
                {
                        ev_new.key.keysym.sym = SDLK_1;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_1;
                }
                else
                {
                        ev_new.key.keysym.sym = SDLK_2;
                        ev_new.key.keysym.scancode = SDL_SCANCODE_2;
                }
            }
        }
        else
        //outside of the column
        {
            /*ev_new.key.keysym.sym = SDLK_w;
            ev_new.key.keysym.scancode = SDL_SCANCODE_W;
            */
        }
        if (fingery > 410.0F / h)
        {
            if (fingerx > 480.0F / w)
            {
                ev_new.key.keysym.sym = SDLK_EQUALS;
                ev_new.key.keysym.scancode = SDL_SCANCODE_EQUALS;
            }
            else
            {
                ev_new.key.keysym.sym = SDLK_MINUS;
                ev_new.key.keysym.scancode = SDL_SCANCODE_MINUS;
            }
        }

    }
    else
    {
        // back touch
    }

    if (ev->type == SDL_FINGERDOWN)
    {
        ev_new.type = ev_new.key.type = SDL_KEYDOWN;
        ev_new.key.state = SDL_PRESSED;
    }
    else if (ev->type == SDL_FINGERUP)
    {
        ev_new.type = ev_new.key.type = SDL_KEYUP;
        ev_new.key.state = SDL_RELEASED;
    }

    SDL_PushEvent(&ev_new);
}

void TranslateAnalogEvent(SDL_Event *ev)
{
    static const int deadzone = 512;
    int delta;
    delta = ev->jaxis.value;
    if (delta >= -deadzone && delta <= deadzone)
    {   
        delta = 0;
    }
    // denominators in the below expressions estimated empirically //todo
    if (ev->jaxis.axis == 0)  //side-to-side
    {
        control2x = delta / 360;
    }
    else if (ev->jaxis.axis == 2) //turn
    {
        in_mouse_dx = delta / 500 ;
    }
    else if (ev->jaxis.axis == 1) //forward
    {
        in_mouse_dy = delta / 1860 ; //2100 slower than key /2050 faster /2070 a bit slower?
    }
}
