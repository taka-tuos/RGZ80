;---------------
;sdcc Z80 I/O function
;---------------
	.area	_CODE
;
; ret=inp(port addr)
;
_io_in::
	ld	hl,#2
	add	hl,sp
;
	ld	c,(hl)
	in	a,(c)
;
	ld	l,a
	ld	h,#0
;
	ret
;
; outp(port addr,param)
;
_io_out::
	push bc
	ld	hl,#4
	add	hl,sp
;
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	out	(c),b
;
	pop 	bc
	ret