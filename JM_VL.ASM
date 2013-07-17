;===========================================================================
;
; JM_VL.ASM
;
;===========================================================================

	IDEAL
	MODEL	MEDIUM,C

SCREENSEG	=	0a000h

	DATASEG

	EXTRN	bufferofs:WORD

	CODESEG

;===========================================================================


;--------------------------------------------------------------------------
; JM_VGALinearFill() - Clears a linear section of latched VGA memory
;--------------------------------------------------------------------------
PROC	JM_VGALinearFill    start:WORD, len:WORD, fill:BYTE
PUBLIC	JM_VGALinearFill
USES SI,DI,DS



;
;	ES = screen memory segment						(dest)

     	mov	ax,SCREENSEG
     	mov	es,ax

;
; Store off the staring address for later reference
;

     	mov	ds,[WORD PTR start]

; Init our direction flag once for our STOS

     	cld

;
; PRE INIT 'bx' for a VGA MAP MASK  (BH will be shifted per plane loop)
;
     	mov bx,0102h
     	mov dx,03c4h			; Init our VGA register num..
     	mov si,4

;
; VGA 'PLANE' LOOP (4 times)
;
; Loop for 4 Planes of VGA Latch memory
;
@@loop:

;
;	DI	-> screen memory offset            (dest)
;
     	mov	di,ds

;
; Set the VGAMAPMASK
;
     	cli
     	mov ax,bx
     	out dx,ax
     	sti

     	shl bh,1


;
; INDIVIDUAL PLANE LOOP
;
; Get ready to move length/2 WORDS for this MAPMASK
;

		mov	cx,[WORD PTR len]
      shr	cx,1
   	mov	al,[BYTE PTR fill]
   	mov	ah,al

		rep 	stosw

;
;	 Check to see if we have done all planes
;
     	dec	si
     	jnz   @@loop

		ret
ENDP


END