TARGET = bios
OBJS_TARGET = bios.o io.o
CFLAGS = -mz80 --std-c99
LFLAGS = -b _CODE=0x0000 -b _DATA=0x2000 -m -l /usr/share/sdcc/lib/z80/z80.lib

include Makefile.in
