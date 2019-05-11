#include "SDL.h"
#include "z80.h"
#include <dirent.h>

byte io80_readm(int param, ushort address);
void io80_writem(int param, ushort address, byte data);
byte io80_readp(int param, ushort address);
void io80_writep(int param, ushort address, byte data);

void rgz_grefresh(SDL_Surface *sdl_screen);

void init_psg(void);
void reset_psg(void);

enum {
	RG_TRIG1,
	RG_TRIG2,
	RG_UP,
	RG_LEFT,
	RG_RIGHT,
	RG_DOWN,
	RG_START,
	RG_SELECT,
};
