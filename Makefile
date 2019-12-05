TARGET		= RGZ80
OBJS_TARGET	= RGZ80.o io80.o psg.o vdp.o z80.o libgpu.o fifo.c
ADDITIONAL_BUILD = dither

CFLAGS += -O3 `sdl-config --cflags`
LIBS += `sdl-config --libs` -lm -lc

include Makefile.in

dither : dither.c Makefile Makefile.in
	$(GCC) dither.c --std=c99 -lm -o dither
