/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
============================================================== */


//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "3d_def.h"

#ifdef MSVC
#pragma hdrstop
#endif

#define	KeyInt		9	// The keyboard ISR number

//
// mouse constants
//
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33

//
// joystick constants
//
#define	JoyScaleMax		32768
#define	JoyScaleShift	8
//#define	MaxJoyValue		5000


/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
boolean			MousePresent;
boolean			JoysPresent[MaxJoys];
boolean			JoyPadPresent;
boolean			NGinstalled=false;


// 	Global variables
		boolean JoystickCalibrated;				// JAM - added
		ControlType ControlTypeUsed;				// JAM - added
		boolean		Keyboard[NumCodes];
		boolean		Paused;
		char		LastASCII;
		ScanCode	LastScan;

KeyboardDef KbdDefs = {
    sc_control,
    sc_alt,
    sc_home,
    sc_up_arrow,
    sc_page_up,
    sc_left_arrow,
    sc_right_arrow,
    sc_end,
    sc_down_arrow,
    sc_page_down
}; // KeyboardDef KbdDefs

		JoystickDef	JoyDefs[MaxJoys];
		ControlType	Controls[MaxPlayers];

		Uint32	MouseDownCount;

#if DEMOS_ENABLED
		Uint8 *DemoBuffer;
		Uint16		DemoOffset,DemoSize;
#endif

bool in_use_modern_bindings = true;
Bindings in_bindings;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/

boolean		IN_Started;

static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};

const char			* IN_ParmStrings[] = {"nojoys","nomouse","enablegp",nil};

//	Internal routines

// BBi
static ScanCode in_keyboard_map_to_bstone(
    SDL_Keycode key_code,
    SDL_Keymod key_mod)
{
    // FIXME There is no reliable way to check for numlock state in SDL2.
    bool is_numlock_active = true;

    switch (key_code) {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return sc_return;

    case SDLK_ESCAPE:
        return sc_escape;

    case SDLK_SPACE:
    case SDLK_KP_SPACE:
        return sc_space;

    case SDLK_MINUS:
        return sc_minus;

    case SDLK_EQUALS:
        return sc_equals;

    case SDLK_BACKSPACE:
    case SDLK_KP_BACKSPACE:
        return sc_backspace;

    case SDLK_TAB:
    case SDLK_KP_TAB:
        return sc_tab;

    case SDLK_LALT:
    case SDLK_RALT:
        return sc_alt;

    case SDLK_LEFTBRACKET:
    case SDLK_KP_LEFTBRACE:
        return sc_left_bracket;

    case SDLK_RIGHTBRACKET:
    case SDLK_KP_RIGHTBRACE:
        return sc_right_bracket;

    case SDLK_LCTRL:
    case SDLK_RCTRL:
        return sc_control;

    case SDLK_CAPSLOCK:
        return sc_caps_lock;

    case SDLK_NUMLOCKCLEAR:
        return sc_num_lock;

    case SDLK_SCROLLLOCK:
        return sc_scroll_lock;

    case SDLK_LSHIFT:
        return sc_left_shift;

    case SDLK_RSHIFT:
        return sc_right_shift;

    case SDLK_UP:
        return sc_up_arrow;

    case SDLK_KP_8:
        if (is_numlock_active)
            return sc_up_arrow;
        else
            return sc_8;

    case SDLK_DOWN:
        return sc_down_arrow;

    case SDLK_KP_2:
        if (is_numlock_active)
            return sc_down_arrow;
        else
            return sc_2;

    case SDLK_LEFT:
        return sc_left_arrow;

    case SDLK_KP_4:
        if (is_numlock_active)
            return sc_left_arrow;
        else
            return sc_4;

    case SDLK_RIGHT:
        return sc_right_arrow;

    case SDLK_KP_6:
        if (is_numlock_active)
            return sc_right_arrow;
        else
            return sc_6;

    case SDLK_INSERT:
        return sc_insert;

    case SDLK_KP_0:
        if (is_numlock_active)
            return sc_insert;
        else
            return sc_0;

    case SDLK_DELETE:
        return sc_delete;

    case SDLK_KP_COMMA:
        if (is_numlock_active)
            return sc_delete;
        else
            return sc_comma;

    case SDLK_HOME:
        return sc_home;

    case SDLK_KP_7:
        if (is_numlock_active)
            return sc_home;
        else
            return sc_7;

    case SDLK_END:
        return sc_end;

    case SDLK_KP_1:
        if (is_numlock_active)
            return sc_end;
        else
            return sc_1;

    case SDLK_PAGEUP:
        return sc_page_up;

    case SDLK_KP_9:
        if (is_numlock_active)
            return sc_page_up;
        else
            return sc_9;

    case SDLK_PAGEDOWN:
        return sc_page_down;

    case SDLK_KP_3:
        if (is_numlock_active)
            return sc_page_down;
        else
            return sc_3;

    case SDLK_SLASH:
    case SDLK_KP_DIVIDE:
        return sc_slash;

    case SDLK_BACKSLASH:
        return sc_backslash;

    case SDLK_SEMICOLON:
        return sc_semicolon;

    case SDLK_QUOTE:
        return sc_quote;

    case SDLK_PERIOD:
        return sc_period;

    case SDLK_F1:
        return sc_f1;

    case SDLK_F2:
        return sc_f2;

    case SDLK_F3:
        return sc_f3;

    case SDLK_F4:
        return sc_f4;

    case SDLK_F5:
        return sc_f5;

    case SDLK_F6:
        return sc_f6;

    case SDLK_F7:
        return sc_f7;

    case SDLK_F8:
        return sc_f8;

    case SDLK_F9:
        return sc_f9;

    case SDLK_F10:
        return sc_f10;

    case SDLK_F11:
        return sc_f11;

    case SDLK_F12:
        return sc_f12;

    case SDLK_PRINTSCREEN:
        return sc_print_screen;

    case SDLK_PAUSE:
        return sc_pause;

    case SDLK_BACKQUOTE:
        return sc_back_quote;

    case SDLK_1:
        return sc_1;

    case SDLK_2:
        return sc_2;

    case SDLK_3:
        return sc_3;

    case SDLK_4:
        return sc_4;

    case SDLK_5:
        return sc_5;

    case SDLK_6:
        return sc_6;

    case SDLK_7:
        return sc_7;

    case SDLK_8:
        return sc_8;

    case SDLK_9:
        return sc_9;

    case SDLK_0:
        return sc_0;

    case SDLK_a:
    case SDLK_KP_A:
        return sc_a;

    case SDLK_b:
    case SDLK_KP_B:
        return sc_b;

    case SDLK_c:
    case SDLK_KP_C:
        return sc_c;

    case SDLK_d:
    case SDLK_KP_D:
        return sc_d;

    case SDLK_e:
    case SDLK_KP_E:
        return sc_e;

    case SDLK_f:
    case SDLK_KP_F:
        return sc_f;

    case SDLK_g:
        return sc_g;

    case SDLK_h:
        return sc_h;

    case SDLK_i:
        return sc_i;

    case SDLK_j:
        return sc_j;

    case SDLK_k:
        return sc_k;

    case SDLK_l:
        return sc_l;

    case SDLK_m:
        return sc_m;

    case SDLK_n:
        return sc_n;

    case SDLK_o:
        return sc_o;

    case SDLK_p:
        return sc_p;

    case SDLK_q:
        return sc_q;

    case SDLK_r:
        return sc_r;

    case SDLK_s:
        return sc_s;

    case SDLK_t:
        return sc_t;

    case SDLK_u:
        return sc_u;

    case SDLK_v:
        return sc_v;

    case SDLK_w:
        return sc_w;

    case SDLK_x:
        return sc_x;

    case SDLK_y:
        return sc_y;

    case SDLK_z:
        return sc_z;

    case SDLK_KP_MINUS:
        return sc_kp_minus;

    case SDLK_KP_PLUS:
        return sc_kp_plus;

    default:
        return sc_none;
    }
}

static char in_keyboard_map_to_char(
    const SDL_KeyboardEvent& e)
{
    Uint16 flags = e.keysym.mod;
    bool is_caps = false;
    bool is_shift = false;
    SDL_Keycode key_code = e.keysym.sym;

    if ((flags & (
        KMOD_LCTRL |
        KMOD_RCTRL |
        KMOD_LALT |
        KMOD_RALT |
        KMOD_LGUI |
        KMOD_RGUI |
        KMOD_MODE)) != 0)
    {
        return sc_none;
    }

    switch (key_code) {
    case SDLK_ESCAPE:
    case SDLK_BACKSPACE:
    case SDLK_TAB:
    case SDLK_RETURN:
    case SDLK_SPACE:
    case SDLK_DELETE:
        return static_cast<char>(key_code);
    }


    is_shift = ((flags & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0);

    if (allcaps)
        is_caps = true;
    else {
        if ((flags & KMOD_CAPS) != 0)
            is_caps = !is_caps;

        if (is_shift)
            is_caps = !is_caps;
    }

    //
    // Keys which depends on L/R Shift
    //

    if (is_shift) {
        switch (key_code) {
        case SDLK_1:
            return '!';

        case SDLK_2:
            return '@';

        case SDLK_3:
            return '#';

        case SDLK_4:
            return '$';

        case SDLK_5:
            return '%';

        case SDLK_6:
            return '^';

        case SDLK_7:
            return '&';

        case SDLK_8:
            return '*';

        case SDLK_9:
            return '(';

        case SDLK_0:
            return ')';

        case SDLK_MINUS:
            return '_';

        case SDLK_EQUALS:
            return '+';

        case SDLK_LEFTBRACKET:
            return '{';

        case SDLK_RIGHTBRACKET:
            return '}';

        case SDLK_SEMICOLON:
            return ':';

        case SDLK_QUOTE:
            return '"';

        case SDLK_BACKQUOTE:
            return '~';

        case SDLK_BACKSLASH:
            return '|';

        case SDLK_COMMA:
            return '<';

        case SDLK_PERIOD:
            return '>';

        case SDLK_SLASH:
            return '?';
        }
    } else {
        switch (key_code) {
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:
        case SDLK_6:
        case SDLK_7:
        case SDLK_8:
        case SDLK_9:
        case SDLK_0:
        case SDLK_MINUS:
        case SDLK_EQUALS:
        case SDLK_LEFTBRACKET:
        case SDLK_RIGHTBRACKET:
        case SDLK_SEMICOLON:
        case SDLK_QUOTE:
        case SDLK_BACKQUOTE:
        case SDLK_BACKSLASH:
        case SDLK_COMMA:
        case SDLK_PERIOD:
        case SDLK_SLASH:
            return static_cast<char>(key_code);
        }
    }


    //
    // Keys which depends on Caps Lock & L/R Shift
    //

    switch (key_code) {
    case SDLK_a:
    case SDLK_b:
    case SDLK_c:
    case SDLK_d:
    case SDLK_e:
    case SDLK_f:
    case SDLK_g:
    case SDLK_h:
    case SDLK_i:
    case SDLK_j:
    case SDLK_k:
    case SDLK_l:
    case SDLK_m:
    case SDLK_n:
    case SDLK_o:
    case SDLK_p:
    case SDLK_q:
    case SDLK_r:
    case SDLK_s:
    case SDLK_t:
    case SDLK_u:
    case SDLK_v:
    case SDLK_w:
    case SDLK_x:
    case SDLK_y:
    case SDLK_z:
        return is_caps ? static_cast<char>(::SDL_toupper(key_code)) :
            static_cast<char>(key_code);
    }

    return sc_none;
}

static void in_handle_keyboard(
    const SDL_KeyboardEvent& e)
{
    SDL_Keycode key_code = e.keysym.sym;
    SDL_Keymod key_mod = ::SDL_GetModState();
    ScanCode key = ::in_keyboard_map_to_bstone(key_code, key_mod);

    char key_char;
    boolean is_pressed;

    if (key == sc_none)
        return;

    switch (key) {
    case sc_alt:
        is_pressed = ((key_mod & KMOD_ALT) != 0);
        break;

    case sc_control:
        is_pressed = ((key_mod & KMOD_CTRL) != 0);
        break;

    default:
        is_pressed = (e.state == SDL_PRESSED);
        break;
    }

    Keyboard[key] = is_pressed;

    if (is_pressed) {
        LastScan = key;

        key_char = ::in_keyboard_map_to_char(e);

        if (key_char != '\0')
            LastASCII = key_char;
    }
}

static void in_handle_mouse_buttons(
    const SDL_MouseButtonEvent& e)
{
    bool is_pressed = (e.state == SDL_PRESSED);

    switch (e.button) {
    case SDL_BUTTON_LEFT:
        Keyboard[sc_mouse_left] = is_pressed;
        break;

    case SDL_BUTTON_MIDDLE:
        Keyboard[sc_mouse_middle] = is_pressed;
        break;

    case SDL_BUTTON_RIGHT:
        Keyboard[sc_mouse_right] = is_pressed;
        break;

    case SDL_BUTTON_X1:
        Keyboard[sc_mouse_x1] = is_pressed;
        break;

    case SDL_BUTTON_X2:
        Keyboard[sc_mouse_x2] = is_pressed;
        break;
    }
}

static int in_mouse_dx;
static int in_mouse_dy;

static void in_handle_mouse_motion(
    const SDL_MouseMotionEvent& e)
{
    in_mouse_dx += e.xrel;
    in_mouse_dy += e.yrel;
}

static void in_handle_mouse(
    const SDL_Event& e)
{
    switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        ::in_handle_mouse_buttons(e.button);
        break;

    case SDL_MOUSEMOTION:
        ::in_handle_mouse_motion(e.motion);
        break;
    }
}
// BBi

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void INL_GetMouseDelta(
    Sint16* x,
    Sint16* y)
{
    *x = static_cast<Sint16>(in_mouse_dx);
    *y = static_cast<Sint16>(in_mouse_dy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static int INL_GetMouseButtons()
{
    ::in_handle_events();

    int result = 0;

    if (Keyboard[sc_mouse_left])
        result |= 1;

    if (Keyboard[sc_mouse_middle])
        result |= 4;

    if (Keyboard[sc_mouse_right])
        result |= 2;

    if (Keyboard[sc_mouse_x1])
        result |= 8;

    if (Keyboard[sc_mouse_x2])
        result |= 16;

    return result;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void
IN_GetJoyAbs(Uint16 joy,Uint16 *xp,Uint16 *yp)
{
// FIXME
#if 0
	Uint8	xb,yb;
	Uint8	xs,ys;
	Uint16	x,y;

// Handle Notebook Gamepad's joystick.
//
	if (NGinstalled)
	{
		unsigned ax,bx;

		joy++;

		_AL=0x01;
		_BX=joy;
		NGjoy(0x01);

		ax=_AX;		bx=_BX;
		*xp=ax;		*yp=bx;

		return;
	}

// Handle normal PC joystick.
//
	x = y = 0;
	xs = joy? 2 : 0;		// Find shift value for x axis
	xb = 1 << xs;			// Use shift value to get x bit mask
	ys = joy? 3 : 1;		// Do the same for y axis
	yb = 1 << ys;

// Read the absolute joystick values
asm		pushf				// Save some registers
asm		push	si
asm		push	di
asm		cli					// Make sure an interrupt doesn't screw the timings


asm		mov		dx,0x201
asm		in		al,dx
asm		out		dx,al		// Clear the resistors

asm		mov		ah,[xb]		// Get masks into registers
asm		mov		ch,[yb]

asm		xor		si,si		// Clear count registers
asm		xor		di,di
asm		xor		bh,bh		// Clear high byte of bx for later

asm		push	bp			// Don't mess up stack frame
asm		mov		bp,MaxJoyValue

loop:
asm		in		al,dx		// Get bits indicating whether all are finished

asm		dec		bp			// Check bounding register
asm		jz		done		// We have a silly value - abort

asm		mov		bl,al		// Duplicate the bits
asm		and		bl,ah		// Mask off useless bits (in [xb])
asm		add		si,bx		// Possibly increment count register
asm		mov		cl,bl		// Save for testing later

asm		mov		bl,al
asm		and		bl,ch		// [yb]
asm		add		di,bx

asm		add		cl,bl
asm		jnz		loop 		// If both bits were 0, drop out

done:
asm     pop		bp

asm		mov		cl,[xs]		// Get the number of bits to shift
asm		shr		si,cl		//  and shift the count that many times

asm		mov		cl,[ys]
asm		shr		di,cl

asm		mov		[x],si		// Store the values into the variables
asm		mov		[y],di

asm		pop		di
asm		pop		si
asm		popf				// Restore the registers

	*xp = x;
	*yp = y;
#endif // 0

    *xp = 0;
    *yp = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void INL_GetJoyDelta(Uint16 joy,Sint16 *dx,Sint16 *dy)
{
	Uint16		x,y;
	JoystickDef	*def;

// FIXME
#if 0
static	Uint32	lasttime;
#endif // 0

	IN_GetJoyAbs(joy,&x,&y);
	def = JoyDefs + joy;

	if (x < def->threshMinX)
	{
		if (x < def->joyMinX)
			x = def->joyMinX;

		x = -(x - def->threshMinX);
		x *= def->joyMultXL;
		x >>= JoyScaleShift;
		*dx = (x > 127)? -127 : -x;
	}
	else if (x > def->threshMaxX)
	{
		if (x > def->joyMaxX)
			x = def->joyMaxX;

		x = x - def->threshMaxX;
		x *= def->joyMultXH;
		x >>= JoyScaleShift;
		*dx = (x > 127)? 127 : x;
	}
	else
		*dx = 0;

	if (y < def->threshMinY)
	{
		if (y < def->joyMinY)
			y = def->joyMinY;

		y = -(y - def->threshMinY);
		y *= def->joyMultYL;
		y >>= JoyScaleShift;
		*dy = (y > 127)? -127 : -y;
	}
	else if (y > def->threshMaxY)
	{
		if (y > def->joyMaxY)
			y = def->joyMaxY;

		y = y - def->threshMaxY;
		y *= def->joyMultYH;
		y >>= JoyScaleShift;
		*dy = (y > 127)? 127 : y;
	}
	else
		*dy = 0;

// FIXME
#if 0
	lasttime = TimeCount;
#endif // 0
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyButtons() - Returns the button status of the specified
//		joystick
//
///////////////////////////////////////////////////////////////////////////
static Uint16
INL_GetJoyButtons(Uint16 joy)
{
// FIXME
#if 0
register	Uint16	result;

// Handle Notebook Gamepad's joystick.
//
	if (NGinstalled)
	{
		unsigned ax,bx;

		joy++;

		_AL=0x01;
		_BX=joy;
		NGjoy(0x00);

		result=_AL;
		result >>= joy? 6 : 4;	// Shift into bits 0-1
		result &= 3;				// Mask off the useless bits
		result ^= 3;

		return(result);
	}

// Handle normal PC joystick.
//
	result = inportb(0x201);	// Get all the joystick buttons
	result >>= joy? 6 : 4;	// Shift into bits 0-1
	result &= 3;				// Mask off the useless bits
	result ^= 3;
	return(result);
#endif // 0

    return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyButtonsDB() - Returns the de-bounced button status of the
//		specified joystick
//
///////////////////////////////////////////////////////////////////////////
Uint16
IN_GetJoyButtonsDB(Uint16 joy)
{
	Uint32	lasttime;
	Uint16		result1,result2;

	do
	{
		result1 = INL_GetJoyButtons(joy);
		lasttime = TimeCount;
		while (TimeCount == lasttime)
			;
		result2 = INL_GetJoyButtons(joy);
	} while (result1 != result2);
	return(result1);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
void INL_StartKbd()
{
    IN_ClearKeysDown();
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void INL_ShutKbd()
{
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
boolean INL_StartMouse()
{
    return true;
}

// BBi
static void INL_ShutMouse()
{
    //::SDL_ShowCursor(SDL_TRUE);
}
// BBi


//
//	INL_SetJoyScale() - Sets up scaling values for the specified joystick
//
static void
INL_SetJoyScale(Uint16 joy)
{
	JoystickDef	*def;

	def = &JoyDefs[joy];
	def->joyMultXL = JoyScaleMax / (def->threshMinX - def->joyMinX);
	def->joyMultXH = JoyScaleMax / (def->joyMaxX - def->threshMaxX);
	def->joyMultYL = JoyScaleMax / (def->threshMinY - def->joyMinY);
	def->joyMultYH = JoyScaleMax / (def->joyMaxY - def->threshMaxY);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//		to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetupJoy(Uint16 joy,Uint16 minx,Uint16 maxx,Uint16 miny,Uint16 maxy)
{
	Uint16		d,r;
	JoystickDef	*def;

	def = &JoyDefs[joy];

	def->joyMinX = minx;
	def->joyMaxX = maxx;
	r = maxx - minx;
	d = r / 5;
	def->threshMinX = ((r / 2) - d) + minx;
	def->threshMaxX = ((r / 2) + d) + minx;

	def->joyMinY = miny;
	def->joyMaxY = maxy;
	r = maxy - miny;
	d = r / 5;
	def->threshMinY = ((r / 2) - d) + miny;
	def->threshMaxY = ((r / 2) + d) + miny;

	INL_SetJoyScale(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutJoy() - Cleans up the joystick stuff
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutJoy(Uint16 joy)
{
	JoysPresent[joy] = false;
}

#if 0
#if !FREE_FUNCTIONS || IN_DEVELOPMENT

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
boolean
INL_StartJoy(Uint16 joy)
{
	Uint16		x,y;

	IN_GetJoyAbs(joy,&x,&y);

	if
	(
		((x == 0) || (x > MaxJoyValue - 10))
	||	((y == 0) || (y > MaxJoyValue - 10))
	)
		return(false);
	else
	{
		IN_SetupJoy(joy,0,x * 2,0,y * 2);
		return(true);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	boolean	checkjoys,checkmouse,checkNG;
	Uint16	i;

	if (IN_Started)
		return;

	checkjoys = true;
	checkmouse = true;
	checkNG = false;

	for (i = 1;i < g_argc;i++)
	{
		switch (US_CheckParm(g_argv[i],IN_ParmStrings))
		{
			case 0:
				checkjoys = false;
			break;

			case 1:
				checkmouse = false;
			break;

			case 2:
				checkNG = true;
			break;
		}
	}

	if (checkNG)
	{
		#define WORD_CODE(c1,c2)	((c2)|(c1<<8))

		NGjoy(0xf0);
		if ((_AX==WORD_CODE('S','G')) && _BX)
			NGinstalled=true;
	}

	INL_StartKbd();
	MousePresent = checkmouse? INL_StartMouse() : false;

	for (i = 0;i < MaxJoys;i++)
		JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;

	IN_Started = true;
}

#endif
#endif

#if 0

///////////////////////////////////////////////////////////////////////////
//
//	IN_Default() - Sets up default conditions for the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Default(boolean gotit,ControlType in)
{
	if
	(
		(!gotit)
	|| 	((in == ctrl_Joystick1) && !JoysPresent[0])
	|| 	((in == ctrl_Joystick2) && !JoysPresent[1])
	|| 	((in == ctrl_Mouse) && !MousePresent)
	)
		in = ctrl_Keyboard1;
	IN_SetControlType(0,in);
}

#endif

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Shutdown(void)
{
	Uint16	i;

	if (!IN_Started)
		return;

	for (i = 0;i < MaxJoys;i++)
		INL_ShutJoy(i);
	INL_ShutKbd();

    // BBi
    ::INL_ShutMouse();

	IN_Started = false;
}

#if 0

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetKeyHook() - Sets the routine that gets called by INL_KeyService()
//			everytime a real make/break code gets hit
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetKeyHook(void (*hook)())
{
	INL_KeyHook = hook;
}

#endif


///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	LastScan = sc_none;
	LastASCII = key_None;
	memset (Keyboard,0,sizeof(Keyboard));
}

// BBi
static void in_handle_window(
    const SDL_WindowEvent& e)
{
    bool clear_state = false;

    switch (e.event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        clear_state = true;
        ::SDL_SetRelativeMouseMode(SDL_TRUE);

        break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
        clear_state = true;
        ::SDL_SetRelativeMouseMode(SDL_FALSE);
        break;
    }

    if (clear_state) {
        ::in_clear_mouse_deltas();

        Keyboard[sc_mouse_left] = false;
        Keyboard[sc_mouse_middle] = false;
        Keyboard[sc_mouse_right] = false;
        Keyboard[sc_mouse_x1] = false;
        Keyboard[sc_mouse_x2] = false;
    }
}

void in_handle_events()
{
    SDL_Event e;

    ::SDL_PumpEvents();

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            ::in_handle_keyboard(e.key);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
            ::in_handle_mouse(e);
            break;

        case SDL_WINDOWEVENT:
            ::in_handle_window(e.window);
            break;
        }
    }
}
// BBi

//
// IN_ReadControl() was modified to allow a single player to use any input
// device at any time without having to physically choose a different
// input device.
//
// PROBLEM: This technique will not work for multi-player games.
//
// Basically, this function should use "player" when in "multi-player"
// mode and ignore "player" when in single player mode.
//
// Anyway, it's not a major problem until we start working with
// with multi-player games.
//
//                                                       - Mike
//

///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(Sint16 player,ControlInfo *info)
{
			boolean		realdelta=false;
			Uint16		buttons;
			Sint16			dx,dy;
			Motion		mx,my;
register	KeyboardDef	*def;

    // BBi
    ::in_handle_events();

	player=player;					// shut up compiler!

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

#if 0												 
	if (DemoMode == demo_Playback)
	{
		dbyte = DemoBuffer[DemoOffset + 1];
		my = (dbyte & 3) - 1;
		mx = ((dbyte >> 2) & 3) - 1;
		buttons = (dbyte >> 4) & 3;

		if (!(--DemoBuffer[DemoOffset]))
		{
			DemoOffset += 2;
			if (DemoOffset >= DemoSize)
				DemoMode = demo_PlayDone;
		}

		realdelta = false;
	}
	else if (DemoMode == demo_PlayDone)
		IN_ERROR(IN_READCONTROL_PLAY_EXC);
	else
#endif
	{

// JAM begin
		ControlTypeUsed = ctrl_None;

		// Handle keyboard input...
		//
		if (ControlTypeUsed == ctrl_None)
		{
			def = &KbdDefs;

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;

			if (Keyboard[def->up])
				my = motion_Up;
			else if (Keyboard[def->down])
				my = motion_Down;

			if (Keyboard[def->left])
				mx = motion_Left;
			else if (Keyboard[def->right])
				mx = motion_Right;

			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
			if (mx || my || buttons)
				ControlTypeUsed = ctrl_Keyboard;
		}

		// Handle mouse input...
		//
		if (MousePresent && (ControlTypeUsed == ctrl_None))
		{
			INL_GetMouseDelta(&dx,&dy);
			buttons = static_cast<Uint16>(INL_GetMouseButtons());
			realdelta = true;
			if (dx || dy || buttons)
				ControlTypeUsed = ctrl_Mouse;
		}

		// Handle joystick input...
		//
		if (JoystickCalibrated && (ControlTypeUsed == ctrl_None))
		{
			INL_GetJoyDelta(ctrl_Joystick1 - ctrl_Joystick,&dx,&dy);
			buttons = INL_GetJoyButtons(ctrl_Joystick1 - ctrl_Joystick);
			realdelta = true;
			if (dx || dy || buttons)
				ControlTypeUsed = ctrl_Joystick;
		}

// JAM end


#if 0
		switch (type = Controls[player])
		{

		case ctrl_Keyboard:
			def = &KbdDefs;

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;

			if (Keyboard[def->up])
				my = motion_Up;
			else if (Keyboard[def->down])
				my = motion_Down;

			if (Keyboard[def->left])
				mx = motion_Left;
			else if (Keyboard[def->right])
				mx = motion_Right;

			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
		break;

		case ctrl_Joystick1:
		case ctrl_Joystick2:
			INL_GetJoyDelta(type - ctrl_Joystick,&dx,&dy);
			buttons = INL_GetJoyButtons(type - ctrl_Joystick);
			realdelta = true;
		break;

		case ctrl_Mouse:
			INL_GetMouseDelta(&dx,&dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;
		break;

		}
#endif
	}

	if (realdelta)
	{
		mx = (dx < 0)? motion_Left : ((dx > 0)? motion_Right : motion_None);
		my = (dy < 0)? motion_Up : ((dy > 0)? motion_Down : motion_None);
	}
	else
	{
		dx = static_cast<Sint16>(mx * 127);
		dy = static_cast<Sint16>(my * 127);
	}

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = buttons & (1 << 0);
	info->button1 = buttons & (1 << 1);
	info->button2 = buttons & (1 << 2);
	info->button3 = buttons & (1 << 3);
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];

#if 0
	if (DemoMode == demo_Record)
	{
		// Pack the control info into a byte
		dbyte = (buttons << 4) | ((mx + 1) << 2) | (my + 1);

		if
		(
			(DemoBuffer[DemoOffset + 1] == dbyte)
		&&	(DemoBuffer[DemoOffset] < 255)
		)
			(DemoBuffer[DemoOffset])++;
		else
		{
			if (DemoOffset || DemoBuffer[DemoOffset])
				DemoOffset += 2;

			if (DemoOffset >= DemoSize)
				IN_ERROR(IN_READCONTROL_BUF_OV);

			DemoBuffer[DemoOffset] = 1;
			DemoBuffer[DemoOffset + 1] = dbyte;
		}
	}
#endif
}

#if 0

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetControlType() - Sets the control type to be used by the specified
//		player
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetControlType(Sint16 player,ControlType type)
{
	// DEBUG - check that requested type is present?
	Controls[player] = type;
}

#endif


#if 0

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
ScanCode
IN_WaitForKey(void)
{
	ScanCode	result;

	while (!(result = LastScan))
		;
	LastScan = 0;
	return(result);
}

#endif

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForASCII() - Waits for an ASCII char, then clears LastASCII and
//		returns the ASCII value
//
///////////////////////////////////////////////////////////////////////////
char
IN_WaitForASCII(void)
{
	char		result;

	while ((result = LastASCII) == '\0')
		::in_handle_events();
	LastASCII = '\0';
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean	btnstate[8];

void IN_StartAck(void)
{
	Uint16	i,buttons;

//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset (btnstate,0,sizeof(btnstate));

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if (buttons&1)
			btnstate[i] = true;
}


boolean IN_CheckAck (void)
{
	Uint16	i,buttons;

    in_handle_events();
//
// see if something has been pressed
//
	if (LastScan)
		return true;

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if ( buttons&1 )
		{
			if (!btnstate[i])
				return true;
		}
		else
			btnstate[i]=false;

	return false;
}


void IN_Ack()
{
    IN_StartAck();

    while (!IN_CheckAck())
        ;
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(Uint32 delay)
{
	Uint32	lasttime;

	lasttime = TimeCount;
	IN_StartAck ();
	do
	{
		VL_WaitVBL(1);
		if (IN_CheckAck())
			return true;
	} while (TimeCount - lasttime < delay);
	return(false);
}
//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/

Uint8 IN_MouseButtons()
{
    return static_cast<Uint8>(::INL_GetMouseButtons());
}

/*
===================
=
= IN_JoyButtons
=
===================
*/

Uint8	IN_JoyButtons (void)
{
// FIXME
#if 0
	unsigned joybits;

	joybits = inportb(0x201);	// Get all the joystick buttons
	joybits >>= 4;				// only the high bits are useful
	joybits ^= 15;				// return with 1=pressed

	return joybits;
#endif // 0

    return 0;
}

boolean INL_StartJoy(Uint16 joy)
{
    Uint16 x;
    Uint16 y;

    IN_GetJoyAbs(joy, &x, &y);

    if ((x == 0 || x > MaxJoyValue - 10) ||
        (y == 0 || y > MaxJoyValue - 10))
    {
        return false;
    } else {
        IN_SetupJoy(joy, 0, x * 2, 0, y * 2);
        return true;
    }
}

void IN_Startup()
{
    int i;
    boolean checkjoys;
    boolean checkmouse;

    if (IN_Started)
        return;

    checkjoys = true;
    checkmouse = true;

    switch (::g_args.check_argument(IN_ParmStrings)) {
    case 0:
        checkjoys = false;
        break;

    case 1:
        checkmouse = false;
        break;

    case 2:
        // FIXME Print a warning?
        break;
    }

    INL_StartKbd();
    MousePresent = checkmouse? INL_StartMouse() : false;

    for (i = 0; i < MaxJoys; ++i)
        JoysPresent[i] = checkjoys? INL_StartJoy(static_cast<Uint16>(i)) : false;

    in_set_default_bindings();

    IN_Started = true;
}

// BBi
void in_get_mouse_deltas(
    int& dx,
    int& dy)
{
    Sint16 dx16;
    Sint16 dy16;

    ::INL_GetMouseDelta(&dx16, &dy16);

    dx = dx16;
    dy = dy16;
}

void in_clear_mouse_deltas()
{
    in_mouse_dx = 0;
    in_mouse_dy = 0;
}

void in_set_default_bindings()
{
    for (int b = 0; b < k_max_bindings; ++b) {
        for (int k = 0; k < k_max_binding_keys; ++k)
            in_bindings[b][k] = sc_none;
    }

    in_bindings[e_bi_forward][0] = sc_w;
    in_bindings[e_bi_backward][0] = sc_s;
    in_bindings[e_bi_left][0] = sc_left_arrow;
    in_bindings[e_bi_right][0] = sc_right_arrow;
    in_bindings[e_bi_strafe][0] = sc_alt;
    in_bindings[e_bi_strafe_left][0] = sc_a;
    in_bindings[e_bi_strafe_right][0] = sc_d;
    in_bindings[e_bi_quick_left][0] = sc_q;
    in_bindings[e_bi_quick_right][0] = sc_e;
    in_bindings[e_bi_turn_around][0] = sc_r;
    in_bindings[e_bi_run][0] = sc_left_shift;

    in_bindings[e_bi_attack][0] = sc_control;
    in_bindings[e_bi_attack][1] = sc_mouse_left;
    in_bindings[e_bi_weapon_1][0] = sc_1;
    in_bindings[e_bi_weapon_2][0] = sc_2;
    in_bindings[e_bi_weapon_3][0] = sc_3;
    in_bindings[e_bi_weapon_4][0] = sc_4;
    in_bindings[e_bi_weapon_5][0] = sc_5;
    in_bindings[e_bi_weapon_6][0] = sc_6;

    in_bindings[e_bi_use][0] = sc_space;
    in_bindings[e_bi_use][1] = sc_mouse_right;

    in_bindings[e_bi_stats][0] = sc_tab;
    in_bindings[e_bi_radar_magnify][0] = sc_equals;
    in_bindings[e_bi_radar_minify][0] = sc_minus;

    in_bindings[e_bi_help][0] = sc_f1;
    in_bindings[e_bi_save][0] = sc_f2;
    in_bindings[e_bi_load][0] = sc_f3;
    in_bindings[e_bi_sound][0] = sc_f4;
    in_bindings[e_bi_controls][0] = sc_f6;
    in_bindings[e_bi_end_game][0] = sc_f7;
    in_bindings[e_bi_quick_save][0] = sc_f8;
    in_bindings[e_bi_quick_load][0] = sc_f9;
    in_bindings[e_bi_quick_exit][0] = sc_f10;

    in_bindings[e_bi_attack_info][0] = sc_i;
    in_bindings[e_bi_lightning][0] = sc_l;
    in_bindings[e_bi_sfx][0] = sc_x;
    in_bindings[e_bi_music][0] = sc_m;
    in_bindings[e_bi_ceiling][0] = sc_c;
    in_bindings[e_bi_flooring][0] = sc_f;

    in_bindings[e_bi_pause][0] = sc_p;
    in_bindings[e_bi_pause][1] = sc_pause;
}

bool in_is_binding_pressed(
    BindingId binding_id)
{
    if (in_use_modern_bindings) {
        const Binding& binding = in_bindings[binding_id];

        return
            (binding[0] != sc_none && Keyboard[binding[0]]) ||
            (binding[1] != sc_none && Keyboard[binding[1]]);
    } else {
        switch (binding_id) {
        case e_bi_forward:
            return Keyboard[dirscan[di_north]];

        case e_bi_backward:
            return Keyboard[dirscan[di_south]];

        case e_bi_left:
            return Keyboard[dirscan[di_west]];

        case e_bi_right:
            return Keyboard[dirscan[di_east]];

        case e_bi_strafe:
            return Keyboard[buttonscan[bt_strafe]];

        case e_bi_quick_left:
            return Keyboard[sc_q];

        case e_bi_quick_right:
            return Keyboard[sc_e];

        case e_bi_turn_around:
            return Keyboard[sc_w] || Keyboard[sc_return];

        case e_bi_run:
            return Keyboard[buttonscan[bt_run]];

        case e_bi_attack:
            return Keyboard[buttonscan[bt_attack]];

        case e_bi_weapon_1:
            return Keyboard[sc_1];

        case e_bi_weapon_2:
            return Keyboard[sc_2];

        case e_bi_weapon_3:
            return Keyboard[sc_3];

        case e_bi_weapon_4:
            return Keyboard[sc_4];

        case e_bi_weapon_5:
            return Keyboard[sc_5];

        case e_bi_weapon_6:
            return Keyboard[sc_6];

        case e_bi_use:
            return Keyboard[sc_space];

        case e_bi_stats:
            return Keyboard[sc_tab];

        case e_bi_radar_magnify:
            return Keyboard[sc_equals];

        case e_bi_radar_minify:
            return Keyboard[sc_minus];

        case e_bi_help:
            return Keyboard[sc_f1];

        case e_bi_save:
            return Keyboard[sc_f2];

        case e_bi_load:
            return Keyboard[sc_f3];

        case e_bi_sound:
            return Keyboard[sc_f4];

        case e_bi_controls:
            return Keyboard[sc_f6];

        case e_bi_end_game:
            return Keyboard[sc_f7];

        case e_bi_quick_save:
            return Keyboard[sc_f8];

        case e_bi_quick_load:
            return Keyboard[sc_f9];

        case e_bi_quick_exit:
            return Keyboard[sc_f10];

        case e_bi_attack_info:
            return Keyboard[sc_i];

        case e_bi_lightning:
            return Keyboard[sc_l];

        case e_bi_sfx:
            return Keyboard[sc_s];

        case e_bi_music:
            return Keyboard[sc_m];

        case e_bi_ceiling:
            return Keyboard[sc_c];

        case e_bi_flooring:
            return Keyboard[sc_f];

        case e_bi_pause:
            return Keyboard[sc_p] || Keyboard[sc_pause];

        default:
            return false;
        }
    }

    return false;
}
// BBi
