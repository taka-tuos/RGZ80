#include "bios.h"

void io_hlt(void)
{
	__asm
		HALT
	__endasm;
}

void io_out(char port, char data)
{
	__asm
		ld hl,#2
		add hl,sp
		ld bc,#0
		ld c,(hl)
		ld hl,#3
		add hl,sp
		ld a,(hl)
		out (c),a
	__endasm;
}

int io_in(char port)
{
	__asm
		ld hl,#2
		add hl,sp
		ld bc,#0
		ld c,(hl)
		in l,(c)
		ld h,#0
	__endasm;
}