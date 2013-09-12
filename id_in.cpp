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

#include "id_heads.h"

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

// FIXME
#if 0
#define	Mouse(x)	_AX = x,geninterrupt(MouseInt)
#endif // 0

#define Mouse(x)

//
// joystick constants
//
#define	JoyScaleMax		32768
#define	JoyScaleShift	8
//#define	MaxJoyValue		5000


extern int _argc;
extern char** _argv;


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

		KeyboardDef	KbdDefs = {0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51};
		JoystickDef	JoyDefs[MaxJoys];
		ControlType	Controls[MaxPlayers];

		Uint32	MouseDownCount;

#if DEMOS_ENABLED
		Uint8 *DemoBuffer;
		Uint16		DemoOffset,DemoSize;
#endif

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/
static	Uint8        ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					SpecialNames[] =	// ASCII for 0xe0 prefixed codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					};


boolean		IN_Started;
static	boolean		CapsLock;
static	ScanCode	CurCode,LastCode;

static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};

static	void			(*INL_KeyHook)(void);
static	void (*OldKeyVect)(void);

const char			* IN_ParmStrings[] = {"nojoys","nomouse","enablegp",nil};

//	Internal routines

// FIXME
#if 0
///////////////////////////////////////////////////////////////////////////
//
//	INL_KeyService() - Handles a keyboard interrupt (key up/down)
//
///////////////////////////////////////////////////////////////////////////
static void
INL_KeyService(void)
{
static	boolean	special;
		Uint8	k,c,
				temp;
		int		i;

	k = inportb(0x60);	// Get the scan code

	// Tell the XT keyboard controller to clear the key
	outportb(0x61,(temp = inportb(0x61)) | 0x80);
	outportb(0x61,temp);

	if (k == 0xe0)		// Special key prefix
		special = true;
	else if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (k & 0x80)	// Break code
		{
			k &= 0x7f;

// DEBUG - handle special keys: ctl-alt-delete, print scrn

			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

			if (special)
				c = SpecialNames[k];
			else
			{
				if (k == sc_CapsLock)
				{
					CapsLock ^= true;
					// DEBUG - make caps lock light work
				}

				if (Keyboard[sc_LShift] || Keyboard[sc_RShift])	// If shifted
				{
					c = ShiftNames[k];
					if ((c >= 'A') && (c <= 'Z') && CapsLock)
						c += 'a' - 'A';
				}
				else
				{
					c = ASCIINames[k];
					if ((c >= 'a') && (c <= 'z') && CapsLock)
						c -= 'a' - 'A';
				}
			}
			if (c)
				LastASCII = c;

			if (c)
				if (allcaps)
					LastASCII = toupper(c);
				else
					LastASCII = c;
		}

		special = false;
	}

	if (INL_KeyHook && !special)
		INL_KeyHook();
	outportb(0x20,0x20);
}
#endif // 0

// BBi
static int in_keyboard_map_to_bstone(SDL_Keycode key_code)
{
    switch (key_code) {
    case SDLK_RETURN:
        return sc_Return;

    case SDLK_ESCAPE:
        return sc_Escape;

    case SDLK_SPACE:
        return sc_Space;

    case SDLK_MINUS:
        return sc_Minus;

    case SDLK_EQUALS:
        return sc_Plus;

    case SDLK_BACKSPACE:
        return sc_BackSpace;

    case SDLK_TAB:
        return sc_Tab;

    case SDLK_LALT:
    case SDLK_RALT:
        return sc_Alt;

    case SDLK_LEFTBRACKET:
        return sc_LBrace;

    case SDLK_RIGHTBRACKET:
        return sc_RBrace;

    case SDLK_LCTRL:
    case SDLK_RCTRL:
        return sc_Control;

    case SDLK_CAPSLOCK:
        return sc_CapsLock;

    case SDLK_LSHIFT:
        return sc_LShift;

    case SDLK_RSHIFT:
        return sc_RShift;

    case SDLK_UP:
        return sc_UpArrow;

    case SDLK_DOWN:
        return sc_DownArrow;

    case SDLK_LEFT:
        return sc_LeftArrow;

    case SDLK_RIGHT:
        return sc_RightArrow;

    case SDLK_INSERT:
        return sc_Insert;

    case SDLK_DELETE:
        return sc_Delete;

    case SDLK_HOME:
        return sc_Home;

    case SDLK_END:
        return sc_End;

    case SDLK_PAGEUP:
        return sc_PgUp;

    case SDLK_PAGEDOWN:
        return sc_PgDn;

    case SDLK_SLASH:
        return sc_Slash;

    case SDLK_F1:
        return sc_F1;

    case SDLK_F2:
        return sc_F2;

    case SDLK_F3:
        return sc_F3;

    case SDLK_F4:
        return sc_F4;

    case SDLK_F5:
        return sc_F5;

    case SDLK_F6:
        return sc_F6;

    case SDLK_F7:
        return sc_F7;

    case SDLK_F8:
        return sc_F8;

    case SDLK_F9:
        return sc_F9;

    case SDLK_F10:
        return sc_F10;

    case SDLK_F11:
        return sc_F11;

    case SDLK_F12:
        return sc_F12;

    case SDLK_BACKQUOTE:
        return sc_Tilde;

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
        return sc_A;

    case SDLK_b:
        return sc_B;

    case SDLK_c:
        return sc_C;

    case SDLK_d:
        return sc_D;

    case SDLK_e:
        return sc_E;

    case SDLK_f:
        return sc_F;

    case SDLK_g:
        return sc_G;

    case SDLK_h:
        return sc_H;

    case SDLK_i:
        return sc_I;

    case SDLK_j:
        return sc_J;

    case SDLK_k:
        return sc_K;

    case SDLK_l:
        return sc_L;

    case SDLK_m:
        return sc_M;

    case SDLK_n:
        return sc_N;

    case SDLK_o:
        return sc_O;

    case SDLK_p:
        return sc_P;

    case SDLK_q:
        return sc_Q;

    case SDLK_r:
        return sc_R;

    case SDLK_s:
        return sc_S;

    case SDLK_t:
        return sc_T;

    case SDLK_u:
        return sc_U;

    case SDLK_v:
        return sc_V;

    case SDLK_w:
        return sc_W;

    case SDLK_x:
        return sc_X;

    case SDLK_y:
        return sc_Y;

    case SDLK_z:
        return sc_Z;

    case SDLK_KP_MINUS:
        return sc_kpMinus;

    case SDLK_KP_PLUS:
        return sc_kpPlus;

    default:
        return sc_None;
    }
}

static char in_keyboard_map_to_char(const SDL_KeyboardEvent* e)
{
    Uint16 flags = e->keysym.mod;
    boolean is_caps = false;
    SDL_Keycode key_code = e->keysym.sym;

    if ((flags & (
        KMOD_LCTRL |
        KMOD_RCTRL |
        KMOD_LALT |
        KMOD_RALT |
        KMOD_LGUI |
        KMOD_RGUI |
        KMOD_MODE)) != 0)
    {
        return sc_None;
    }

    if ((flags & KMOD_CAPS) != 0)
        is_caps = !is_caps;

    if ((flags & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0)
        is_caps = !is_caps;

    switch (key_code) {
    case SDLK_ESCAPE:
    case SDLK_BACKSPACE:
    case SDLK_TAB:
    case SDLK_RETURN:
    case SDLK_SPACE:
    case SDLK_DELETE:
        return key_code;
    }

    if (!is_caps) {
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
            return key_code;
        }
    } else {
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
            return SDL_toupper(key_code);
        }
    }

    return sc_None;
}

static void in_handle_keyboard(const SDL_KeyboardEvent* e)
{
    SDL_Keycode key_code = e->keysym.sym;
    int key = in_keyboard_map_to_bstone(key_code);
    Uint16 key_mod;
    char key_char;
    boolean is_pressed;

    if (key_code == SDLK_PAUSE && e->state == SDL_PRESSED) {
        Paused = true;
        return;
    }

    if (key == sc_None)
        return;

    key_mod = SDL_GetModState();

    switch (key) {
    case sc_Alt:
        is_pressed = ((key_mod & KMOD_ALT) != 0);
        break;

    case sc_Control:
        is_pressed = ((key_mod & KMOD_CTRL) != 0);
        break;

    default:
        is_pressed = (e->state == SDL_PRESSED);
        break;
    }

    Keyboard[key] = is_pressed;

    if (is_pressed) {
        LastScan = key;

        key_char = in_keyboard_map_to_char(e);

        if (key_char != '\0')
            LastASCII = key_char;
    }
}
// BBi

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void
INL_GetMouseDelta(Sint16 *x,Sint16 *y)
{
	Mouse(MDelta);

// FIXME
#if 0
	*x = _CX;
	*y = _DX;
#endif // 0
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static Uint16
INL_GetMouseButtons(void)
{
	Uint16	buttons;

	Mouse(MButtons);

// FIXME
#if 0
	buttons = _BX;
#endif // 0

    buttons = 0;

	return(buttons);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void
IN_GetJoyAbs(Uint16 joy,Uint16 *xp,Uint16 *yp)
{
	Uint8	xb,yb,
			xs,ys;
	Uint16	x,y;

// FIXME
#if 0
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
#endif // 0

// Handle normal PC joystick.
//
	x = y = 0;
	xs = joy? 2 : 0;		// Find shift value for x axis
	xb = 1 << xs;			// Use shift value to get x bit mask
	ys = joy? 3 : 1;		// Do the same for y axis
	yb = 1 << ys;

// FIXME
#if 0
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
#endif // 0

	*xp = x;
	*yp = y;
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
static	Uint32	lasttime;

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

	lasttime = TimeCount;
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
boolean
INL_StartMouse(void)
{
// FIXME
#if 0
#if 0
	if (getvect(MouseInt))
	{
		Mouse(MReset);
		if (_AX == 0xffff)
			return(true);
	}
	return(false);
#endif
 union REGS regs;
 Uint8 *vector;


 if ((vector=MK_FP(peek(0,0x33*4+2),peek(0,0x33*4)))==NULL)
   return false;

 if (*vector == 207)
   return false;

 Mouse(MReset);
 return true;
#endif // 0

    return false;
}

#if 0
///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutMouse() - Cleans up after the mouse
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutMouse(void)
{
}
#endif


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

	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],IN_ParmStrings))
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
	LastScan = sc_None;
	LastASCII = key_None;
	memset (Keyboard,0,sizeof(Keyboard));
}

// BBi
void in_handle_events()
{
    SDL_Event e;

    SDL_PumpEvents();

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            in_handle_keyboard(&e.key);
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

    in_handle_events();

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
			buttons = INL_GetMouseButtons();
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
		dx = mx * 127;
		dy = my * 127;
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

	while (!(result = LastASCII))
		;
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

Uint8	IN_MouseButtons (void)
{
// FIXME
#if 0
	if (MousePresent)
	{
		Mouse(MButtons);
		return _BX;
	}
	else
#endif // 0

		return 0;
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

    for (i = 1; i < _argc; ++i) {
        switch (US_CheckParm(_argv[i], IN_ParmStrings)) {
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
    }

    INL_StartKbd();
    MousePresent = checkmouse? INL_StartMouse() : false;

    for (i = 0; i < MaxJoys; ++i)
        JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;

    IN_Started = true;
}
