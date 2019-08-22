
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
#include "../3d_def.h"

extern int in_mouse_dx;
extern int in_mouse_dy;
extern int control2x;
extern bool vid_is_ui_stretched;
extern gametype gamestate;
int GetPrevWeaponNum();
int GetNextWeaponNum();
void DepressGivenKey(int whichkey, int upordown);
int lastautopress = 0;

void TranslateControllerEvent(SDL_Event *ev)
{
    int upordown = 0;
    int btn;
    SDL_Event ev_new;
    static const struct 
    {
        SDL_Keycode sym;
        SDL_Scancode scan;
    } v_keymap[] = 
    {
        { SDLK_y, SDL_SCANCODE_Y },                 // Triangle
#ifdef VITATEST
        { SDLK_BACKSPACE, SDL_SCANCODE_BACKSPACE},
#else
        { SDLK_RALT, SDL_SCANCODE_RALT },             // Circle
#endif
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
	    upordown = 1;
    }
    else if (ev->type == SDL_JOYBUTTONUP)
    {
    	ev_new.type = ev_new.key.type = SDL_KEYUP;
		ev_new.key.state = SDL_RELEASED;
	    upordown = 0;
    }

    SDL_PushEvent(&ev_new);
#ifndef VITATEST
	if (btn == 1)
	{
		if ((lastautopress == 0) and (upordown == 1))
		{
			lastautopress = GetNextWeaponNum();
			DepressGivenKey(lastautopress, upordown);
		}
		else
		{
			DepressGivenKey(lastautopress, 0);
			lastautopress = 0;
		}
	}
#endif
}

void TranslateTouchEvent(SDL_Event *ev)
{
    int upordown = 0;

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
	if (ev->type == SDL_FINGERDOWN)
	{
	    upordown = 1;
	}
	else if (ev->type == SDL_FINGERUP)
	{
	   upordown = 0;
	}

	if (fingerx > 660.0F / w && fingerx < 860.0F / w)
        //column containing elevator buttons
        {
            if (fingery > 50.0F / h  && fingery <= 139.0F / h)
            //9,10  50-140 
            {
                if (fingerx < m/w )
                {
			DepressGivenKey(9, upordown);
                }
                else
                {
			DepressGivenKey(0, upordown);
                }
            }
            //7,8   140-194
            if (fingery > 139.0F / h  && fingery <= 194.0F / h)
            {
                if (fingerx < m/w )
                {
			DepressGivenKey(7, upordown);
                }
                else
                {
			DepressGivenKey(8, upordown);
                }
            }
            //5,6   194-249
            if (fingery > 194.0F / h  && fingery <= 249.0F / h)
            {
                if (fingerx < m/w )
                {
			DepressGivenKey(5, upordown);
                }
                else
                {
			DepressGivenKey(6, upordown);
                }
            }
            //3,4   249-303
            if (fingery > 249.0F / h  && fingery <= 304.0F / h)
            {
                if (fingerx < m/w )
                {
			DepressGivenKey(3, upordown);
                }
                else
                {
			DepressGivenKey(4, upordown);
                }
            }
            //1,2   303-410
            if (fingery > 304.0F / h  && fingery <= 410.0F / h)
            {
                if (fingerx < m/w )
                {
			DepressGivenKey(1, upordown);
                }
                else
                {
			DepressGivenKey(2, upordown);
                }
            }
        }
        else
        //outside of the column
        {
        }
        if (fingery > 410.0F / h)
        {
            if (fingerx > 480.0F / w)
            {
		        DepressGivenKey(11, upordown);
            }
            else
            {
		        DepressGivenKey(12, upordown);
            }
        }
        if (fingery < 50.0F / h)
        {

            if (fingerx > 480.0F / w)
            {
		        if ((lastautopress == 0) and (upordown == 1))
		        {
			        lastautopress = GetNextWeaponNum();
			        DepressGivenKey(lastautopress, upordown);
		        }
		        else
		        {
			        DepressGivenKey(lastautopress, 0);
			        lastautopress = 0;
		        }
            }
            else //left side
	        {
		        if ((lastautopress == 0) and (upordown == 1))
		        {
			        lastautopress = GetPrevWeaponNum();
			        DepressGivenKey(lastautopress, upordown);
		        }
		        else
		        {
			        DepressGivenKey(lastautopress, 0);
			        lastautopress = 0;
		        }
	        }
	    }
    }
    else
    {
        // back touch
    }

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
    if (ev->jaxis.axis == 0)  //side-to-side
    {
        control2x = static_cast<int> (delta * (1.0F + (::mouseadjustment / 6.0F)) / 1777.0F);
    }
    else if (ev->jaxis.axis == 2) //turn
    {
        in_mouse_dx = delta / 1677 ;
    }
    else if (ev->jaxis.axis == 1) //forward
    {
        in_mouse_dy = delta / 6655 ;
    }
}

int GetNextWeaponNum()
{
	std::int8_t cw = gamestate.weapon;
	std::int8_t uw = gamestate.useable_weapons;
	if ((cw < wp_pistol) && (uw & (1 << wp_pistol)))
	{
        	return 2;
	}
	else if ((cw < wp_burst_rifle) && (uw & (1 << wp_burst_rifle)))
	{
        	return 3;
	}
	else if ((cw < wp_ion_cannon) && (uw & (1 << wp_ion_cannon)))
	{
        	return 4;
	}
	else if ((cw < wp_grenade) && (uw & (1 << wp_grenade)))
	{
        	return 5;
	}
	if ((cw < wp_bfg_cannon) && (uw & (1 << wp_bfg_cannon)))
	{
        	return 6;
	}
	else
	{
        	return 1;
	}
}

int GetPrevWeaponNum()
{
	std::int8_t cw = gamestate.weapon;
	std::int8_t uw = gamestate.useable_weapons;
	if (cw == wp_autocharge) cw = wp_SPACER;
	if ((cw > wp_bfg_cannon) && (uw & (1 << wp_bfg_cannon)))
	{
        	return 6;
	}
	else if ((cw > wp_grenade) && (uw & (1 << wp_grenade)))
	{
        	return 5;
	}
	else if ((cw > wp_ion_cannon) && (uw & (1 << wp_ion_cannon)))
	{
        	return 4;
	}
	else if ((cw > wp_burst_rifle) && (uw & (1 << wp_burst_rifle)))
	{
        	return 3;
	}
	else if ((cw > wp_pistol) && (uw & (1 << wp_pistol)))
	{
        	return 2;
	}
	else
	{
        	return 1;
	}
}

void DepressGivenKey(int whichkey, int upordown)
{
	// 0 is up, nonzero is down
	SDL_Event ev_new;
	memset(&ev_new, 0, sizeof(SDL_Event));

	switch (whichkey)
	{
		case 1:
		{
			ev_new.key.keysym.sym = SDLK_1;
            ev_new.key.keysym.scancode = SDL_SCANCODE_1;
			break;
		}
		case 2:
		{
			ev_new.key.keysym.sym = SDLK_2;
            ev_new.key.keysym.scancode = SDL_SCANCODE_2;
			break;
		}
		case 3:
		{
			ev_new.key.keysym.sym = SDLK_3;
            ev_new.key.keysym.scancode = SDL_SCANCODE_3;
			break;
		}
		case 4:
		{
			ev_new.key.keysym.sym = SDLK_4;
            ev_new.key.keysym.scancode = SDL_SCANCODE_4;
			break;
		}
		case 5:
		{
			ev_new.key.keysym.sym = SDLK_5;
            ev_new.key.keysym.scancode = SDL_SCANCODE_5;
			break;
		}
		case 6:
		{
			ev_new.key.keysym.sym = SDLK_6;
            ev_new.key.keysym.scancode = SDL_SCANCODE_6;
			break;
		}
		case 7:
		{
			ev_new.key.keysym.sym = SDLK_7;
            ev_new.key.keysym.scancode = SDL_SCANCODE_7;
			break;
		}
		case 8:
		{
			ev_new.key.keysym.sym = SDLK_8;
            ev_new.key.keysym.scancode = SDL_SCANCODE_8;
			break;
		}
		case 9:
		{
			ev_new.key.keysym.sym = SDLK_9;
            ev_new.key.keysym.scancode = SDL_SCANCODE_9;
			break;
		}
		case 0:
		{
			ev_new.key.keysym.sym = SDLK_0;
            ev_new.key.keysym.scancode = SDL_SCANCODE_0;
			break;
		}
		case 11:
		{
			ev_new.key.keysym.sym = SDLK_EQUALS;
            ev_new.key.keysym.scancode = SDL_SCANCODE_EQUALS;
			break;
		}
		case 12:
		{
			ev_new.key.keysym.sym = SDLK_MINUS;
            ev_new.key.keysym.scancode = SDL_SCANCODE_MINUS;
			break;
		}
		default:
        {
            ev_new.key.keysym.sym = SDLK_0;
            ev_new.key.keysym.scancode = SDL_SCANCODE_0;
        }
	}
	if (upordown != 0)
	{
		ev_new.type = ev_new.key.type = SDL_KEYDOWN;
		ev_new.key.state = SDL_PRESSED;
	}
	else
	{
		ev_new.type = ev_new.key.type = SDL_KEYUP;
		ev_new.key.state = SDL_RELEASED;
	}

	SDL_PushEvent(&ev_new);
}

