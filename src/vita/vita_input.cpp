
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
//  modified versions of controller routines from i_video.c and txt_sdl.c in vita chocolate doom
//

#include "SDL.h"

extern int in_mouse_dx;
extern int in_mouse_dy;
extern int control2x; 

void TranslateControllerEvent(SDL_Event *ev)
{
    int btn;
    SDL_Event ev_new;
    int in_prompt;
    static const struct 
    {
        SDL_Keycode sym;
        SDL_Scancode scan;
    } v_keymap[] = 
    {
        { SDLK_y, SDL_SCANCODE_Y },                 // Triangle
        { SDLK_LALT, SDL_SCANCODE_LALT },           // Circle
        { SDLK_RETURN, SDL_SCANCODE_RETURN },       // Cross
        { SDLK_SPACE, SDL_SCANCODE_SPACE },         // Square
        { SDLK_TAB, SDL_SCANCODE_TAB },             // L Trigger
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
    in_prompt = 0; // TODO

    if (in_prompt)
    {
        if (btn == 1 || btn == 10)
        {
            ev_new.key.keysym.sym = SDLK_n;
            ev_new.key.keysym.scancode = SDL_SCANCODE_N;
        }
        else if (btn == 2 || btn == 11)
        {
            ev_new.key.keysym.sym = SDLK_y;
            ev_new.key.keysym.scancode = SDL_SCANCODE_Y;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (btn < 0 || btn > 11)
            return;
        ev_new.key.keysym.sym = v_keymap[btn].sym;
        ev_new.key.keysym.scancode = v_keymap[btn].scan;
    }

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

    float w = 960.0; //screen width
    float m = 760.0; //midpoint between rows
    float h = 544.0; //screen height
    if (ev->tfinger.touchId == 0)
    {
        // front touch

        if (ev->tfinger.x > 660 / w && ev->tfinger.x < 860 / w)    
        //column containing elevator buttons
        {
            if (ev->tfinger.y > 50 / h  && ev->tfinger.y <= 140 / h)    
            // 9,10 50-140
            {
                if (ev->tfinger.x < m/w )
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
            //7,8 140-194
            if (ev->tfinger.y > 140 / h  && ev->tfinger.y <= 194 / h)    
            {
                if (ev->tfinger.x < m/w )
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
            //5,6  194-249
            if (ev->tfinger.y > 194 / h  && ev->tfinger.y <= 249 / h)    
            {
                if (ev->tfinger.x < m/w )
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
            //3,4  249-303
            if (ev->tfinger.y > 249 / h  && ev->tfinger.y <= 303 / h)    
            {
                if (ev->tfinger.x < m/w )
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
            //1,2  303-410
            if (ev->tfinger.y > 303 / h  && ev->tfinger.y <= 410 / h)    
            {
                if (ev->tfinger.x < m/w )
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
        ev_new.key.keysym.sym = SDLK_BACKQUOTE;
        ev_new.key.keysym.scancode = SDL_SCANCODE_GRAVE;
        }
    }
    else
    {
        // back touch
        if (ev->tfinger.x > .5)
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
    // denominaors in the below expressiona estimated empirically
    if (ev->jaxis.axis == 0)
    {
        control2x = delta / 400;
    }
    else if (ev->jaxis.axis == 2)
    {
        in_mouse_dx = delta / 600 ;
    }
    else if (ev->jaxis.axis == 1)
    {
        in_mouse_dy = delta / 1900 ;
    }
}