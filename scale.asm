.386C
IDEAL
MODEL MEDIUM,C

SCREENWIDTH = 80

DATASEG

EXTRN   dc_length:WORD
EXTRN   dc_iscale:DWORD
EXTRN   dc_frac:DWORD
EXTRN   dc_source:WORD
EXTRN   dc_dest:WORD
EXTRN   dc_seg:WORD
EXTRN   shadingtable:DWORD

codeseg

;================
;
; R_DrawColumn
;
;================

PROC   R_DrawColumn
PUBLIC   R_DrawColumn
		  push    ebp
		  push    esi
		  push    edi
		  mov     ax,[dc_seg]
		  mov     fs,ax
		  mov     ax,0a000h
		  mov     es,ax
		  mov     cx,[dc_length]
		  mov     si,[dc_source]
		  mov     di,[dc_dest]
		  mov     eax,[dc_iscale]
		  mov     [DWORD PTR cs:patch1+3],eax
		  mov     ebp,[dc_frac]

pixelloop:
		  mov     ebx,ebp                                 ; begin calculating first pixel
		  shr     ebx,16
patch1:
		  add     ebp,12345678h                           ; advance frac pointer
		  mov     al,[fs:si+bx]
		  mov     [es:di],al
		  add     di,SCREENWIDTH
		  dec     cx
		  jnz     pixelloop
done:
		  pop     edi
		  pop     esi
		  pop     ebp
		  ret

ENDP R_DrawColumn




;================
;
; R_DrawSLSColumn
;
;================

PROC   R_DrawSLSColumn
PUBLIC   R_DrawSLSColumn
		  push    ebp
		  push    esi
		  push    edi

		  mov     ax,[WORD shadingtable + 2]
		  mov     gs,ax

		  mov     ax,0a000h
		  mov     es,ax

		  mov     cx,[dc_length]
		  mov     di,[dc_dest]

spixelloopls:
        mov     al,[es:di]

		  mov     bx,[WORD shadingtable]
		  mov     bl,al
        add 	 bh,10
        cmp		 bh,62
        jle		 okshade
        mov		 bh,62
okshade:
		  mov     al,[gs:bx]

		  mov     [es:di],al
		  add     di,SCREENWIDTH
		  dec     cx
		  jnz     spixelloopls
sdonels:
		  pop     edi
		  pop     esi
		  pop     ebp
		  ret

ENDP R_DrawSLSColumn



;================
;
; R_DrawLSColumn
;
;================

PROC   R_DrawLSColumn
PUBLIC   R_DrawLSColumn
		  push    ebp
		  push    esi
		  push    edi
		  mov     ax,[dc_seg]
		  mov     fs,ax
		  mov     ax,[WORD shadingtable + 2]
		  mov     gs,ax
		  mov     ax,0a000h
		  mov     es,ax
		  mov     cx,[dc_length]
		  mov     si,[dc_source]
		  mov     di,[dc_dest]
		  mov     eax,[dc_iscale]
		  mov     [DWORD PTR cs:patch1ls+3],eax
		  mov     ebp,[dc_frac]

pixelloopls:
		  mov     ebx,ebp                                 ; begin calculating first pixel
		  shr     ebx,16
patch1ls:
		  add     ebp,12345678h                           ; advance frac pointer
		  mov     al,[fs:si+bx]
		  mov     bx,[WORD shadingtable]
		  mov     bl,al
		  mov     al,[gs:bx]
		  mov     [es:di],al
		  add     di,SCREENWIDTH
		  dec     cx
		  jnz     pixelloopls
donels:
		  pop     edi
		  pop     esi
		  pop     ebp
		  ret

ENDP R_DrawLSColumn


END

