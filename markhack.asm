.386C
IDEAL
MODEL MEDIUM,C

 SC_INDEX   =           03C4h
 SC_RESET   =           0
 SC_CLOCK   =           1
 SC_MAPMASK =           2
 CRTC_INDEX =           03D4h
 SCREENBWIDE=           80
 SCREENSEG  =           0a000h
 MISC_OUTPUT=           03c2h

 MASK_COLOR =				254	  

DATASEG

EXTRN    viewwidth:WORD
EXTRN    viewheight:WORD
EXTRN    bufferofs:WORD
EXTRN    ylookup:WORD
EXTRN    centery:WORD
EXTRN    postsource:DWORD
EXTRN    bufx:WORD
EXTRN    postheight:WORD
EXTRN    shadingtable:DWORD


CODESEG

;----------------------------------------------------------------------------
;
; DrawPost - Draws an unmasked post centered in the viewport
;
;            ecx - height to scale post to
;            esi - source pointer to proper column (x offsetted)
;            edi - destination pointer to proper screen column (xoffsetted)
;
;----------------------------------------------------------------------------

;ALIGN 16

PROC    DrawPost
PUBLIC  DrawPost

; ECX - loop variable (height and counter)
; EDI - top offset + bufferofs
; EDX - destination offset
; EBX - high word contains bottom offset, low word contains source data
; EBP - fraction

  push si
  push di
  push bp
  mov   ax,[WORD postsource + 2]
  mov   gs,ax
  mov   si,[WORD postsource]
  mov   ecx,0
  mov   cx,[postheight]
  SETFLAG cx,cx
  jz    donedraw
  mov   edx,0
  mov   eax,32*65536
  idiv   ecx
  mov   edx,eax

; edx holds fractional step

  mov    [DWORD PTR cs:patch1+3],edx					; JTR Self modifing cod
  shr   edx,1
  mov   ax,SCREENSEG
  mov   es,ax
  mov   di,OFFSET ylookup
  mov   ax,[centery]
  shl   ax,1
  add   di,ax
  mov   di,[di]
  add   di,[bufferofs]
  add   di,[bufx]
  mov   bp,SCREENBWIDE
  sub   di,SCREENBWIDE
  shr   ax,1
  cmp   cx,ax
  jle   heightok
  mov   cx,ax
heightok:        ; height is ok.. < viewheigth

drawloop:
  mov   eax,edx
  shr   eax,16
  mov   ebx,31
patch1:
  add   edx,12345678h					; JTR Self Modifing Code
  sub   ebx,eax
  mov   bl,[BYTE PTR gs:si+bx]
  mov   [es:di],bl
  mov   ebx,32
  add   ebx,eax
  mov   bl,[BYTE PTR gs:si+bx]
  mov   [es:di+bp],bl
  sub   di,SCREENBWIDE
  add   bp,SCREENBWIDE*2
  dec   cx
  jnz   drawloop

donedraw:
  pop bp
  pop di
  pop si
  ret

ENDP DrawPost



;----------------------------------------------------------------------------
;
; DrawLSPost - Draws an unmasked light sourced post centered in the viewport
;
;            ecx - height to scale post to
;            esi - source pointer to proper column (x offsetted)
;            edi - destination pointer to proper screen column (xoffsetted)
;
;----------------------------------------------------------------------------

;ALIGN 16

PROC    DrawLSPost
PUBLIC  DrawLSPost

; ECX - loop variable (height and counter)
; EDI - top offset + bufferofs
; EDX - destination offset
; EBX - high word contains bottom offset, low word contains source data
; EBP - fraction

  push si
  push di
  push bp
  mov   ax,[WORD postsource + 2]
  mov   gs,ax
  mov   si,[WORD postsource]
  mov   ecx,0
  mov   ax,[WORD shadingtable + 2]
  mov   fs,ax
  mov   cx,[postheight]
  SETFLAG cx,cx
  jz    donedraw
  mov   edx,0
  mov   eax,32*65536
  idiv   ecx
  mov   edx,eax

; edx holds fractional step

  mov    [DWORD PTR cs:patch1ls+3],edx				; JTR: Self Modifing Code!
  shr   edx,1
  mov   ax,SCREENSEG
  mov   es,ax
  mov   di,OFFSET ylookup
  mov   ax,[centery]
  shl   ax,1
  add   di,ax
  mov   di,[di]
  add   di,[bufferofs]
  add   di,[bufx]
  mov   bp,SCREENBWIDE
  sub   di,SCREENBWIDE
  shr   ax,1
  cmp   cx,ax
  jle   heightokls
  mov   cx,ax
heightokls:        ; height is ok.. < viewheigth

drawloopls:
  mov   eax,edx
  shr   eax,16
  mov   ebx,31

patch1ls:
  add   edx,12345678h								; JTR This value 1234... will change to contain the frac inc value.
  sub   ebx,eax
  shl   eax,16
  mov   al,[BYTE PTR gs:si+bx]
  mov   bx,[WORD shadingtable]
  mov   bl,al
  mov   al,[fs:bx]
  mov   [es:di],al

  mov   ebx,32
  shr   eax,16
  add   ebx,eax
  mov   al,[BYTE PTR gs:si+bx]
  mov   bx,[WORD shadingtable]
  mov   bl,al
  mov   al,[fs:bx]
  mov   [es:di+bp],al

  sub   di,SCREENBWIDE
  add   bp,SCREENBWIDE*2
  dec   cx
  jnz   drawloopls

donedrawls:
  pop bp
  pop di
  pop si
  ret

ENDP DrawLSPost



IF 0					; JIM Add line

;----------------------------------------------------------------------------
;
; DrawMPost - Draws an masked post centered in the viewport
;
;            ecx - height to scale post to
;            esi - source pointer to proper column (x offsetted)
;            edi - destination pointer to proper screen column (xoffsetted)
;
;----------------------------------------------------------------------------

;ALIGN 16


PROC    DrawMPost
PUBLIC  DrawMPost

; ECX - loop variable (height and counter)
; EDI - top offset + bufferofs
; EDX - destination offset
; EBX - high word contains bottom offset, low word contains source data
; EBP - fraction

  push si
  push di
  push bp


  ;
  ; 'postsource' is a ptr to the source image.
  ;
  ; Setup ptr to source image
  ; GS:SI will be address of veritacal post to draw.
  ;

  mov   ax,[WORD postsource + 2]
  mov   gs,ax
  mov   si,[WORD postsource]

  ;
  ; Clear out our high word of our loop counter
  ;

  mov   ecx,0

  ;
  ; Store height and test for 0
  ;


  mov   cx,[postheight]
  SETFLAG cx,cx
  jz    donedrawm

  ;
  ; Calc fractional step values to be stored in EDX
  ;
  ; (Height/Max_View??)
  ;
  ;

  mov   edx,0
  mov   eax,32*65536
  idiv  ecx
  mov   edx,eax

  ;
  ; Self modify the asm code to use the fractional step
  ; value.  Stored at patch1mls+3
  ;

  mov    [DWORD PTR cs:patch1m+3],edx					; Self modifing code

  ;
  ; Compute position on screen to draw pixel
  ;

  shr   edx,1
  mov   ax,SCREENSEG
  mov   es,ax
  mov   di,OFFSET ylookup
  mov   ax,[centery]
  shl   ax,1
  add   di,ax
  mov   di,[di]
  add   di,[bufferofs]
  add   di,[bufx]
  mov   bp,SCREENBWIDE
  sub   di,SCREENBWIDE
  shr   ax,1
  cmp   cx,ax
  jle   heightokm
  mov   cx,ax
heightokm:        ; height is ok.. < viewheigth

  ;
  ;  Get ready to draw post
  ;

drawloopm:
  mov   eax,edx
  shr   eax,16				; restore contents of AX from high word
  mov   ebx,31

patch1m:
  add   edx,12345678h					; Self Modifing Code
  sub   ebx,eax

  ;
  ;  move source pixel into BL
  ;

  mov   bl,[BYTE PTR gs:si+bx]

  ;
  ; BL now contains our source pixel to draw
  ;
  ; Test for mask color
  ;

  cmp		bl,MASK_COLOR
  je		skip1_maskcolor

  ;
  ; Draw source pixel to screen
  ;

  mov   [es:di],bl

skip1_maskcolor:
  mov   ebx,32
  add   ebx,eax

  ;
  ; Get pixel from source image.
  ;

  mov   bl,[BYTE PTR gs:si+bx]

  ;
  ; Test for mask color
  ;

  cmp		bl,MASK_COLOR
  je		skip2_maskcolor

  ;
  ; Draw Pixel
  ;

  mov   [es:di+bp],bl

skip2_maskcolor:
  sub   di,SCREENBWIDE
  add   bp,SCREENBWIDE*2
  dec   cx
  jnz   drawloopm

donedrawm:
  pop bp
  pop di
  pop si
  ret

ENDP DrawMPost



;----------------------------------------------------------------------------
;
; DrawMLSPost - Draws a masked light sourced post centered in the viewport
;
;            ecx - height to scale post to
;            esi - source pointer to proper column (x offsetted)
;            edi - destination pointer to proper screen column (xoffsetted)
;
;----------------------------------------------------------------------------

;ALIGN 16

PROC    DrawMLSPost
PUBLIC  DrawMLSPost

; ECX - loop variable (height and counter)
; EDI - top offset + bufferofs
; EDX - destination offset
; EBX - high word contains bottom offset, low word contains source data
; EBP - fraction

  push si
  push di
  push bp

  ;
  ; Setup ptr to source image
  ; GS:SI will be address of veritacal post to draw.
  ;

  mov   ax,[WORD postsource + 2]
  mov   gs,ax
  mov   si,[WORD postsource]

  ;
  ; Clear out our high word of our loop counter
  ;

  mov   ecx,0

  ;
  ; Init FS to hold SEGMENT for the shading table
  ;
  ;  (shading table is on WORD boundries?)
  ;

  mov   ax,[WORD shadingtable + 2]
  mov   fs,ax                                ; FS - Holds SEG to Far shading table

  ;
  ; Store height and test for 0
  ;

  mov   cx,[postheight]
  SETFLAG cx,cx
  jz    donedrawmls					; Nothing to draw...Exit this mofo.

  ;
  ; Calc fractional step values to be stored in EDX
  ;
  ; (Height/Max_View??)
  ;
  ;

  mov   edx,0
  mov   eax,32*65536
  idiv   ecx
  mov   edx,eax

  ;
  ; Self modify the asm code to use the fractional step
  ; value.  Stored at patch1mls+3
  ;

  mov    [DWORD PTR cs:patch1mls+3],edx

  ;
  ; Divide the fractional step in half
  ;

  shr   edx,1

  ;
  ; Calc screen position to draw vert post
  ;
  ; ES:DI - dest Screen ptr
  ;

  mov   ax,SCREENSEG
  mov   es,ax
  mov   di,OFFSET ylookup
  mov   ax,[centery]
  shl   ax,1
  add   di,ax
  mov   di,[di]
  add   di,[bufferofs]
  add   di,[bufx]

  mov   bp,SCREENBWIDE
  sub   di,SCREENBWIDE

  ;
  ; Test boundries of drawing....?
  ;

  shr   ax,1
  cmp   cx,ax
  jle   heightokmls
  mov   cx,ax
heightokmls:        ; height is ok.. < viewheigth

  ;
  ; Get ready to draw this Mofo!
  ;
  ;

drawloopmls:
  mov   eax,edx
  shr   eax,16
  mov   ebx,31

  ;
  ; NOTE: The value 12345678 will be Modified to hold the
  ;       fractional inc value.
  ;

patch1mls:
  add   edx,12345678h
  sub   ebx,eax
  shl   eax,16						; Store AX into the high word of EAX

  ;
  ; AL will contain the source pixel to draw
  ;

  mov   al,[BYTE PTR gs:si+bx]

  ;
  ; Now test AL for a mask color, if so, lets skip it.
  ;

  cmp		al,MASK_COLOR
  je		skip1_ls_maskcolor

  ;
  ; Nope, Not the mask color....
  ;
  ; Get the offset to the shading table
  ;
  ;  BX = offset to shading table
  ;
  ;

  mov   bx,[WORD shadingtable]					; Get offset for shading table

  ;
  ;  Adjust the offset into the shading table to index
  ;  using the source pixel color (currently being held in
  ;  AL.
  ;

  mov   bl,al

  ;
  ;  FS:BX points to the shaded color
  ;
  ; Move into AL
  ;

  mov   al,[fs:bx]

  ;
  ; Draw the shaded pixel to the screen.
  ;

  mov   [es:di],al

  ;
  ; Do some adjustments for ??????????
  ;
  ;

skip1_ls_maskcolor:
  mov   ebx,32
  shr   eax,16						; Restore the Old AX which was stored in the hi word
  add   ebx,eax

  ;
  ; Move the source pixel into AL
  ;
  ;

  mov   al,[BYTE PTR gs:si+bx]

  ;
  ; Test AL for a mask color
  ;


  cmp		al,MASK_COLOR
  je		skip2_ls_maskcolor


  ;
  ; Get the offset for the Shading table ... again
  ;
  ; BX = Offset into shading table
  ;

  mov   bx,[WORD shadingtable]

  ;
  ; Adjust BX to index with the source pixel color to
  ; get the shaded pixel color.
  ;

  mov   bl,al
  mov   al,[fs:bx]

  ;
  ; AL now contains the shaded pixel... Draw that mofo!
  ;

  mov   [es:di+bp],al

  ;
  ; Move to next pixel(s)
  ;


skip2_ls_maskcolor:
  sub   di,SCREENBWIDE
  add   bp,SCREENBWIDE*2
  dec   cx
  jnz   drawloopmls

donedrawmls:
  pop bp
  pop di
  pop si
  ret

ENDP DrawMLSPost

ENDIF							; JIM Add line

END








