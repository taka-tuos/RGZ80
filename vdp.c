#include "RGZ80.h"
#include "fifo.h"

#include <limits.h>

extern byte *z80_memory;
extern fifo_t *gpu_fifo;

int rgz_vsync=0;

byte rgz_vram[256*768];
int rgz_vram_disp=0;
int gpu_drawingarea[4] = {0,0,512,192}; // x0,y0,x1,y1

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

void vdp_ps10(int x, int y, int c)
{
	//printf("%d,%d\n",x,y);
	byte *p = &rgz_vram[y*256+(x/2)];
	int sft = (x & 1) ? 0 : 4;
	
	int dot = *p & (0xf0 >> sft);
	dot |= (c & 0x0f) << sft;
	
	*p = dot;
}

#define ABSI(n) ((n) > 0 ? (n) : -(n))

void vdp_arrayline(int *ay, int x1, int y1, int x2, int y2)
{
	int steep = ABSI(y2-y1) > ABSI(x2-x1);
	
	if(steep) {
		int tmp;
		tmp=x1;
		x1=y1;
		y1=tmp;
		
		tmp=x2;
		x2=y2;
		y2=tmp;
	}
	
	if(x1>x2) {
		int tmp;
		tmp=x1;
		x1=x2;
		x2=tmp;
		
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	
	int dx=ABSI(x2-x1);
	int dy=ABSI(y2-y1);
	
	int err=dx/2;
	int sy;
	
	sy = y1 < y2 ? 1 : -1;
	
	int y = y1;
	
	for(int x=x1;x<=x2;x++) {
		if(steep) {
			if(x >= 0 && x < 1024) ay[x] = y;
		} else {
			if(y >= 0 && y < 1024) ay[y] = x;
		}
		
		err -= dy;
		if(err < 0) {
			y+=sy;
			err+=dx;
		}
	}
}

#define INTERP(xi,xi1,yi,yi1,x) (yi + ((( yi1 - yi ) * ( x - xi )) / ( xi1 - xi )))

void vdp_drawtrigon(int *xv, int *yv, int c)
{
	int s[3][1024];
	int m[3][4]; // min x,max x,min y,max y
	
	int miny=INT_MAX,maxy=INT_MIN;
	
	int x[3],y[3];
	int mii,mai;
	
	int xmin = gpu_drawingarea[0];
	int ymin = gpu_drawingarea[1];
	
	int xmax = gpu_drawingarea[2];
	int ymax = gpu_drawingarea[3];
	
	for(int i=0;i<3;i++) {
		if(yv[i] > maxy) {
			maxy = yv[i];
			mai = i;
		}
		if(yv[i] < miny) {
			miny = yv[i];
			mii = i;
		}
	}
	
	if(miny == maxy) return;
	
	for(int i=0;i<3;i++) {
		if(i == mii) {
			x[0] = xv[i];
			y[0] = yv[i];
		} else if(i == mai) {
			x[2] = xv[i];
			y[2] = yv[i];
		} else {
			x[1] = xv[i];
			y[1] = yv[i];
		}
	}
	
	for(int i=0;i<3;i++) {
		int x1=x[i],y1=y[i],x2=x[(i+1)%3],y2=y[(i+1)%3];
		m[i][0] = x1 < x2 ? x1 : x2;
		m[i][1] = x1 > x2 ? x1 : x2;
		m[i][2] = y1 < y2 ? y1 : y2;
		m[i][3] = y1 > y2 ? y1 : y2;
	}
	
	for(int i=0;i<3;i++) {
		vdp_arrayline(s[i],x[i],y[i],x[(i+1)%3],y[(i+1)%3]);
	}
	
	if(miny >= ymax || maxy < ymin) return;
	if(miny < ymin) miny = ymin;
	if(maxy >= ymax) maxy = ymax - 1;
	
	for(int i=miny;i<maxy;i++) {
		int xa,xb;
		int tmp;
		
		if(m[0][2] <= i && m[0][3] > i) {
			xa = s[0][i];
		}
		if(m[1][2] <= i && m[1][3] > i) {
			xa = s[1][i];
		}
		if(m[2][2] <= i && m[2][3] > i) {
			xb = s[2][i];
		}
		
		if(xa > xb) {
			tmp=xa;
			xa=xb;
			xb=tmp;
		}
		
		if(xa >= xmax || xb < xmin) continue;
		if(xa < xmin) xa = xmin;
		if(xb >= xmax) xb = xmax - 1;
		
		for(int j=xa;j<xb;j++) {
			vdp_ps10(j,i,c);
		}
	}
	
	//printf("\n");
}

void init_vdp(void)
{
	gpu_fifo = (fifo_t *)malloc(sizeof(fifo_t));
	fifo_init(gpu_fifo, 8192, (uint8_t *)malloc(8192*sizeof(uint8_t)));
}

uint8_t fifo_read8u(fifo_t *fifo)
{
	return fifo_get(fifo);
}

int16_t fifo_read16s(fifo_t *fifo)
{
	uint8_t lo = fifo_get(fifo);
	uint8_t hi = fifo_get(fifo);
	
	unsigned int n = lo | (hi << 8);
	
	int16_t m;
	
	*((unsigned short *)&m) = n;
	
	return m;
}

void vdp_flush(void)
{
	while(fifo_status(gpu_fifo)) {
		int cmd = fifo_get(gpu_fifo);
		
		if(cmd == 0x04) {
			int x[3], y[3];
			for(int i=0;i<3;i++) {
				x[i] = fifo_read16s(gpu_fifo);
				y[i] = fifo_read16s(gpu_fifo);
			}
			int c = fifo_read8u(gpu_fifo);
			vdp_drawtrigon(x,y,c);
		}
	}
}

void rgz_grefresh(SDL_Surface *sdl_screen, int scan)
{
	int st = scan != -1 ? scan : 0;
	for(int y = st; y < 384; y+=2) {
		for(int x = 0; x < 512; x++) {
			Uint8 *q = &rgz_vram[(y/2)*256+(x/2)];
			Uint32 *p = &((Uint32 *)sdl_screen->pixels)[y*(sdl_screen->pitch/4)+x];
			
			int dot = (*q >> ((x & 1) ? 0 : 4)) & 15;
			
			*p = 0xff000000;
			
			*p |= colorpalette[dot*3+0] << 16;
			*p |= colorpalette[dot*3+1] << 8;
			*p |= colorpalette[dot*3+2] << 0;
			
			if(scan == -1) p[sdl_screen->pitch/4] = *p;
		}
	}
	rgz_vsync = 1;
}
