#include "RGZ80.h"
#include "a3d.h"
#include "fifo.h"

extern byte *z80_memory;
extern fifo32_t *gpu_fifo;

int rgz_vsync=0;

byte rgz_vram[2][256*768];
int rgz_vram_disp=0;

Uint8 colorpalette[16*3] = {
	0x00, 0x00, 0x00,
	0xAA, 0x00, 0x00,
	0x00, 0xAA, 0x00,
	0xAA, 0xAA, 0x00,
	0x00, 0x00, 0xAA,
	0xAA, 0x00, 0xAA,
	0x00, 0xAA, 0xAA,
	0xAA, 0xAA, 0xAA,
	0x55, 0x55, 0x55,
	0xFF, 0x55, 0x55,
	0x55, 0xFF, 0x55,
	0xFF, 0xFF, 0x55,
	0x55, 0x55, 0xFF,
	0xFF, 0x55, 0xFF,
	0x55, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF,
};

void PixelShader(int x, int y, int u, int v, int r, int g, int b)
{
	//printf("%d,%d\n",x,y);
	byte *p = &rgz_vram[1-rgz_vram_disp][y*256+(x/2)];
	int sft = (x & 1) ? 0 : 4;
	
	int dot = *p & (0xf0 >> sft);
	dot |= b << sft;
	
	*p = dot;
}

void init_vdp(void)
{
	gpu_fifo = (fifo32_t *)malloc(sizeof(fifo32_t));
	fifo32_init(gpu_fifo, 4096, (void **)malloc(4096*sizeof(void *)));
}

void rgz_grefresh(SDL_Surface *sdl_screen)
{
	for(int y = 0; y < 384; y+=2) {
		for(int x = 0; x < 512; x++) {
			Uint8 *q = &rgz_vram[rgz_vram_disp][(y/2)*256+(x/2)];
			Uint32 *p = &((Uint32 *)sdl_screen->pixels)[y*(sdl_screen->pitch/4)+x];
			
			int dot = (*q >> ((x & 1) ? 0 : 4)) & 15;
			
			*p = 0xff000000;
			
			*p |= colorpalette[dot*3+0] << 16;
			*p |= colorpalette[dot*3+1] << 8;
			*p |= colorpalette[dot*3+2] << 0;
			
			p[sdl_screen->pitch/4] = *p;
		}
	}
	rgz_vsync = 1;
}
