;**************************************************************************
;
; This code was taken from FRED FISH 20.        - Michael D. Maynard
;
; 11-Jan-92 - MDM - C entry points changed to use registers specified
;                   by Lattice's __asm keyword.
;
;**************************************************************************

		IDNT	"RND"

		XDEF	RandomSeed,Random	   ;assembler entry points
		XDEF	_RandomSeed,_Random   ;C entry points
;=============================================================================
; NAME
;    RandomSeed - seed random number generator, call once at beginning of
;		  your program.  CurrentTime provides useful values for this
;
; SYSNOPSIS
;    RandomSeed( SeedValue )
;		    D0
;
; FUNCTION
;    Seeds the random number generator
;
; INPUTS
;    SeedValue - a longword containing any value you like
;
; RESULT
;    Random number generator is initialised
;
; BUGS/LIMITATIONS
;    None that I know of
;
; SEE ALSO
;
;============================================================================


;_RandomSeed	MOVE.L	4(SP),D0	   ;entry point for C functions
_RandomSeed
RandomSeed	ADD.L	D0,D1		;user seed in d0 (d1 too)
		MOVEM.L	D0/D1,RND

; drops through to the main random function (not user callable)

LongRnd		MOVEM.L	D2-D3,-(SP)
		MOVEM.L	RND,D0/D1	   ;D0=LSB's, D1=MSB's of random number
		ANDI.B	#$0E,D0		;ensure upper 59 bits are an...
		ORI.B	#$20,D0		;...odd binary number
		MOVE.L	D0,D2
		MOVE.L	D1,D3
		ADD.L	D2,D2		;accounts for 1 of 17 left shifts
		ADDX.L	D3,D3		;[D2/D3] = RND*2
		ADD.L	D2,D0
		ADDX.L	D3,D1		;[D0/D1] = RND*3
		SWAP	D3		;shift [D2/D3] additional 16 times
		SWAP	D2
		MOVE.W	D2,D3
		CLR.W	D2
		ADD.L	D2,D0		;add to [D0/D1]
		ADDX.L	D3,D1
		MOVEM.L	D0/D1,RND	   ;save for next time through
		MOVE.L	D1,D0		;most random part to D0
		MOVEM.L	(SP)+,D2-D3
		RTS

;=============================================================================
; NAME
;    Random - returns a random integer in the specified range
;
; SYSNOPSIS
;    RndNum = Random( UpperLimit )
;      D0		  D0
;
; FUNCTION
;    returns a random integer in the range 0 to UpperLimit-1
;
; INPUTS
;     UpperLimit - a long(or short will do) in the range 1-65535
;
; RESULT
;    a random integer is returned to you, real quick!
;
; BUGS/LIMITATIONS
;    range was limited to 1-65535 to avoid problems with the DIVU instruction
;    which can return real wierd values if the result is larger than 16 bits.
;
; SEE ALSO
;
;============================================================================

;_Random		MOVE.L	4(SP),D0	   ;C entry point
_Random
Random		MOVE.W	D2,-(SP)
		MOVE.W	D0,D2		;save upper limit
		BEQ.S	10$		;range of 0 returns 0 always
		BSR.S	LongRnd		;get a longword random number
		CLR.W	D0		;use upper word (it's most random)
		SWAP	D0
		DIVU.W	D2,D0		;divide by range...
		CLR.W	D0		;...and use remainder for the value
		SWAP	D0		;result in D0.W
10$		MOVE.W	(SP)+,D2
		RTS

RND		DS.L	2		;random number


