// ID_HEAD.H


#ifndef ID_HEAD_H
#define ID_HEAD_H


#include <cassert>
#include <cstdlib>
#include <malloc.h>

#include <memory>

#include "SDL.h"


#define WOLF
#define FREE_FUNCTIONS   (true)

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	VGAGR

#define false (0)
#define true (1)
typedef	Sint16	boolean;

typedef	struct
		{
			int	x,y;
		} Point;

typedef	struct
		{
			Point	ul,lr;
		} Rect;


void	Quit (char *error,...);		// defined in user program


#endif // ID_HEAD_H
