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
//	ID_IN.h - Header file for Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_IN__
#define	__ID_IN__

#ifdef	__DEBUG__
#define	__DEBUG_InputMgr__
#endif

#define	MaxPlayers	4
#define	MaxKbds		2
#define	MaxJoys		2
#define	NumCodes	128

typedef	Uint8		ScanCode;
#define	sc_None			0
#define	sc_Bad			0xff
#define	sc_Return		0x1c
#define	sc_Enter		sc_Return
#define	sc_Escape		0x01
#define	sc_Space			0x39
#define 	sc_Minus			0x0C			 
#define	sc_Plus			0x0D			 
#define	sc_BackSpace	0x0e
#define	sc_Tab			0x0f
#define	sc_Alt			0x38
#define	sc_LBrace		0x1A			
#define	sc_RBrace		0x1B
#define	sc_Control		0x1d
#define	sc_CapsLock		0x3a
#define	sc_LShift		0x2a
#define	sc_RShift		0x36
#define	sc_UpArrow		0x48
#define	sc_DownArrow	0x50
#define	sc_LeftArrow	0x4b
#define	sc_RightArrow	0x4d
#define	sc_Insert		0x52
#define	sc_Delete		0x53
#define	sc_Home			0x47
#define	sc_End			0x4f
#define	sc_PgUp			0x49
#define	sc_PgDn			0x51
#define  sc_Slash		0x35
#define	sc_F1			0x3b
#define	sc_F2			0x3c
#define	sc_F3			0x3d
#define	sc_F4			0x3e
#define	sc_F5			0x3f
#define	sc_F6			0x40
#define	sc_F7			0x41
#define	sc_F8			0x42
#define	sc_F9			0x43
#define	sc_F10			0x44
#define	sc_F11			0x57
#define	sc_F12			0x59
#define	sc_Tilde		0x29

#define	sc_1			0x02
#define	sc_2			0x03
#define	sc_3			0x04
#define	sc_4			0x05
#define	sc_5			0x06
#define	sc_6			0x07
#define	sc_7			0x08
#define	sc_8			0x09
#define	sc_9			0x0a
#define	sc_0			0x0b

#define	sc_A			0x1e
#define	sc_B			0x30
#define	sc_C			0x2e
#define	sc_D			0x20
#define	sc_E			0x12
#define	sc_F			0x21
#define	sc_G			0x22
#define	sc_H			0x23
#define	sc_I			0x17
#define	sc_J			0x24
#define	sc_K			0x25
#define	sc_L			0x26
#define	sc_M			0x32
#define	sc_N			0x31
#define	sc_O			0x18
#define	sc_P			0x19
#define	sc_Q			0x10
#define	sc_R			0x13
#define	sc_S			0x1f
#define	sc_T			0x14
#define	sc_U			0x16
#define	sc_V			0x2f
#define	sc_W			0x11
#define	sc_X			0x2d
#define	sc_Y			0x15
#define	sc_Z			0x2c

#define	sc_kpMinus	0x4a
#define	sc_kpPlus	0x4e

#define	key_None		0
#define	key_Return		0x0d
#define	key_Enter		key_Return
#define	key_Escape		0x1b
#define	key_Space		0x20
#define	key_BackSpace	0x08
#define	key_Tab			0x09
#define	key_Delete		0x7f
#define	key_UnderScore 0x0c

// 	Stuff for the mouse
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33

#define  NGint	0x15
#define  NGjoy(com)	_AH=0x84;_DX=com;geninterrupt(NGint);

#define	MaxJoyValue		5000			// JAM

enum Demo {
    demo_Off,
    demo_Record,
    demo_Playback,
    demo_PlayDone
}; // enum Demo

enum ControlType {
    ctrl_None, // JAM - added
    ctrl_Keyboard,
    ctrl_Keyboard1 = ctrl_Keyboard,
    ctrl_Keyboard2,
    ctrl_Joystick,
    ctrl_Joystick1 = ctrl_Joystick,
    ctrl_Joystick2,
    ctrl_Mouse
}; // enum ControlType

enum Motion {
    motion_Left = -1,
    motion_Up = -1,
    motion_None = 0,
    motion_Right = 1,
    motion_Down = 1
}; // enum Motion

enum Direction {
    dir_North,
    dir_NorthEast,
    dir_East,
    dir_SouthEast,
    dir_South,
    dir_SouthWest,
    dir_West,
    dir_NorthWest,
    dir_None
}; // enum Direction

struct CursorInfo {
    boolean		button0,button1,button2,button3;
    Sint16			x,y;
    Motion		xaxis,yaxis;
    Direction	dir;
}; // struct  CursorInfo

typedef CursorInfo ControlInfo;

struct KeyboardDef {
    ScanCode button0;
    ScanCode button1;
    ScanCode upleft;
    ScanCode up;
    ScanCode upright;
    ScanCode left;
    ScanCode right;
    ScanCode downleft;
    ScanCode down;
    ScanCode downright;
}; // struct KeyboardDef

struct JoystickDef {
    Uint16 joyMinX;
    Uint16 joyMinY;
    Uint16 threshMinX;
    Uint16 threshMinY;
    Uint16 threshMaxX;
    Uint16 threshMaxY;
    Uint16 joyMaxX;
    Uint16 joyMaxY;
    Uint16 joyMultXL;
    Uint16 joyMultYL;
    Uint16 joyMultXH;
    Uint16 joyMultYH;
}; // struct JoystickDef


// Global variables

extern boolean			NGinstalled; 				// JAM

extern boolean JoystickCalibrated;				// JAM - added
extern ControlType ControlTypeUsed;				// JAM - added
extern	boolean		Keyboard[],
					MousePresent,
					JoysPresent[];
extern	boolean		Paused;
extern	char		LastASCII;
extern	ScanCode	LastScan;
extern	KeyboardDef	KbdDefs;
extern	JoystickDef	JoyDefs[];
extern	ControlType	Controls[MaxPlayers];

extern	Uint8 *DemoBuffer;
extern	Uint16		DemoOffset,DemoSize;

// Function prototypes
#define	IN_KeyDown(code)	(Keyboard[(code)])
#define	IN_ClearKey(code)	{Keyboard[code] = false;\
							if (code == LastScan) LastScan = sc_None;}

// DEBUG - put names in prototypes
extern	void		IN_Startup(void),IN_Shutdown(void),
					IN_Default(boolean gotit,ControlType in),
					IN_SetKeyHook(void (*)()),
					IN_ClearKeysDown(void),
					IN_ReadCursor(CursorInfo *),
					IN_ReadControl(Sint16,ControlInfo *),
					IN_SetControlType(Sint16,ControlType),
					IN_GetJoyAbs(Uint16 joy,Uint16 *xp,Uint16 *yp),
					IN_SetupJoy(Uint16 joy,Uint16 minx,Uint16 maxx,
								Uint16 miny,Uint16 maxy),
					IN_StopDemo(void),IN_FreeDemoBuffer(void),
					IN_Ack(void),IN_AckBack(void);
extern	boolean		IN_UserInput(Uint32 delay);
extern	char		IN_WaitForASCII(void);
extern	ScanCode	IN_WaitForKey(void);
extern	Uint16		IN_GetJoyButtonsDB(Uint16 joy);
extern	Uint8		*IN_GetScanName(ScanCode);


Uint8	IN_MouseButtons (void);
Uint8	IN_JoyButtons (void);


// BBi
void in_handle_events();

void in_get_mouse_deltas(
    int& dx,
    int& dy);

void in_clear_mouse_deltas();
// BBi


#endif
