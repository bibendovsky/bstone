;
;	WaitVBL
;	Wait for the vertical retrace (returns before the actual vertical sync)
;	By Jason Blochowiak
;
	PROC	WaitVBL  num:WORD
	PUBLIC	WaitVBL

@@wait:
	mov	dx,status_Reg1
	mov	bl,8				; test VBL
	mov	bh,1				; test DE

@@waitno:
	in	al,dx
	test al,bl				; VBL?
	jnz	@@waitno			; Yep, keep waiting
	mov	cx,400				; for 320x200 mode (200 lines are double-scanned)
@@waityes:
	in al, dx
	test al,bl				; VBL?
	jnz	@@done				; Yep, we're done
	test al,bh				; No. Display enabled?
	jz	@@waityes			; Yes, keep checking
@@waitenable:
	in	al,dx				; No, wait for Display not enabled or VBL
	test al,bl				; VBL?
	jnz	@@done				; Yep, done
	test al,bh				; No. Display enabled?
	jnz	@@waitenable		; No, keep waiting
	dec	cx					; DE toggled state - decrement line abort count
	jnz	@@waityes			; If 0, we missed the VBL, so drop out,
							; otherwise, go back to waiting for the VBL

@@done:
	dec	[num]				; wait multiple VBLs?
	jnz	@@wait

	ret

	ENDP
