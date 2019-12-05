#include "RGZ80.h"
#include "fifo.h"

#include <limits.h>

extern byte *z80_memory;
extern fifo_t *gpu_fifo;

int rgz_vsync=0;

int vdp_trans = 255;

byte rgz_vram[256*768];
int gpu_drawingarea[4] = {0,0,512,768}; // x0,y0,x1,y1

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
	if(c == vdp_trans) return;
	if(x >= 512 || x < 0) return;
	if(y >= 768 || y < 0) return;
	
	byte *p = &rgz_vram[y*256+(x/2)];
	int sft = (x & 1) ? 0 : 4;
	
	int dot = *p & (0xf0 >> sft);
	dot |= (c & 0x0f) << sft;
	
	*p = dot;
}

#define ABSI(n) ((n) > 0 ? (n) : -(n))

void vdp_extline(int x1, int y1, int x2, int y2, void (*ps)(int,int,void *), void *ext)
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
			ps(y,x,ext);
		} else {
			ps(x,y,ext);
		}
		
		err -= dy;
		if(err < 0) {
			y+=sy;
			err+=dx;
		}
	}
}

void ps_arrayline(int x, int y, void *ay)
{
	if(y >= 0 && y < 1024) ((int *)ay)[y] = x;
}

void ps_drawline(int x, int y, void *c)
{
	vdp_ps10(x,y,*((int *)c));
}

void vdp_arrayline(int *ay, int x1, int y1, int x2, int y2)
{
	vdp_extline(x1,y1,x2,y2,ps_arrayline,ay);
}

void vdp_drawline(int x1, int y1, int x2, int y2, int c)
{
	vdp_extline(x1,y1,x2,y2,ps_drawline,&c);
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
		printf("p[%d]=%d,%d\n",i,xv[i],yv[i]);
	}
	
	printf("\n");
	
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

void vdp_bitblt(int x, int y, int w, int h, byte *src)
{
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			byte *p = &src[i*(w/2)+(j/2)];
			int sft = (j & 1) ? 0 : 4;
			
			int dot = (*p >> sft) & 0x0f;
			
			vdp_ps10(x+j,y+i,dot);
		}
	}
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

uint16_t fifo_read16u(fifo_t *fifo)
{
	uint8_t lo = fifo_get(fifo);
	uint8_t hi = fifo_get(fifo);
	
	unsigned int n = lo | (hi << 8);
	
	uint16_t m;
	
	*((unsigned short *)&m) = n;
	
	return m;
}

float fifo_read32f(fifo_t *fifo)
{
	uint8_t llo = fifo_get(fifo);
	uint8_t lhi = fifo_get(fifo);
	uint8_t hlo = fifo_get(fifo);
	uint8_t hhi = fifo_get(fifo);
	
	unsigned int n = (unsigned int)llo | ((unsigned int)lhi << 8) | ((unsigned int)hlo << 16) | ((unsigned int)hhi << 24);
	
	float m;
	
	*((unsigned int *)&m) = n;
	
	return m;
}

void fifo_write32f(fifo_t *fifo, float n)
{
	unsigned int m;
	
	*((float *)&m) = n;
	
	fifo_put(fifo, m & 0xff);
	fifo_put(fifo, (m >> 8) & 0xff);
	fifo_put(fifo, (m >> 16) & 0xff);
	fifo_put(fifo, (m >> 24) & 0xff);
}

void vdp_flush(void)
{
	while(fifo_status(gpu_fifo)) {
		int cmd = fifo_get(gpu_fifo);
		
		if(cmd == 0x00) {
			int c = fifo_read8u(gpu_fifo);
			
			for(int i=0;i<192;i++) {
				for(int j=0;j<512;j++) {
					vdp_ps10(j,i,c);
				}
			}
		}
		
		if(cmd == 0x01) {
			int x = fifo_read16s(gpu_fifo);
			int y = fifo_read16s(gpu_fifo);
			int w = fifo_read16s(gpu_fifo);
			int h = fifo_read16s(gpu_fifo);
			int q = fifo_read16u(gpu_fifo);
			
			vdp_bitblt(x,y,w,h,z80_memory+q);
		}
		
		if(cmd == 0x02) {
			int x = fifo_read16s(gpu_fifo);
			int y = fifo_read16s(gpu_fifo);
			int w = fifo_read16s(gpu_fifo);
			int h = fifo_read16s(gpu_fifo);
			int o = fifo_read16u(gpu_fifo);
			
			for(int i=0;i<h;i++) {
				for(int j=0;j<w;j++) {
					byte *p = &rgz_vram[i*256+(j/2)];
					byte *q = &z80_memory[o+i*(w/2)+(j/2)];
					
					int sft = (x & 1) ? 0 : 4;
					
					int dot = *q & (0xf0 >> sft);
					dot |= (*p & 0x0f) << sft;
					
					*q = dot;
				}
			}
		}
		
		if(cmd == 0x03) {
			int x = fifo_read16s(gpu_fifo);
			int y = fifo_read16s(gpu_fifo);
			int w = fifo_read16s(gpu_fifo);
			int h = fifo_read16s(gpu_fifo);
			int x2 = fifo_read16u(gpu_fifo);
			int y2 = fifo_read16u(gpu_fifo);
			
			vdp_bitblt(x2,y2,w,h,z80_memory+(y*256+(x/2)));
		}
		
		if(cmd == 0x04) {
			int x[3], y[3];
			for(int i=0;i<3;i++) {
				x[i] = fifo_read16s(gpu_fifo);
				y[i] = fifo_read16s(gpu_fifo);
			}
			int c = fifo_read8u(gpu_fifo);
			
			vdp_drawtrigon(x,y,c);
		}
		
		if(cmd == 0x05) {
			int x1 = fifo_read16s(gpu_fifo);
			int y1 = fifo_read16s(gpu_fifo);
			int x2 = fifo_read16s(gpu_fifo);
			int y2 = fifo_read16s(gpu_fifo);
			int c = fifo_read8u(gpu_fifo);
			
			vdp_drawline(x1,y1,x2,y2,c);
		}
		
		if(cmd == 0x06) {
			int x = fifo_read16s(gpu_fifo);
			int y = fifo_read16s(gpu_fifo);
			int w = fifo_read16s(gpu_fifo);
			int h = fifo_read16s(gpu_fifo);
			int c = fifo_read8u(gpu_fifo);
			
			for(int i=y;i<y+h;i++) {
				for(int j=x;j<x+w;j++) {
					vdp_ps10(j,i,c);
				}
			}
		}
		
		if(cmd == 0x07) {
			int c = fifo_read8u(gpu_fifo);
			
			vdp_trans = c;
		}
		
		if(cmd == 0x80) {
			/*float x = fifo_read32f(gpu_fifo);
			printf("%f\n",x);
			fifo_write32f(gpu_fifo,1.0f);
			return;*/
			float x = fifo_read32f(gpu_fifo);
			float y = fifo_read32f(gpu_fifo);
			float cx = fifo_read32f(gpu_fifo);
			float cy = fifo_read32f(gpu_fifo);
			float r = fifo_read32f(gpu_fifo);
			
			x -= cx;
			y -= cy;
			
			float fx = (x * cos(r)) - (y * sin(r));
			float fy = (x * sin(r)) + (y * cos(r));
			
			fx += cx;
			fy += cy;
			
			fifo_write32f(gpu_fifo, fx);
			fifo_write32f(gpu_fifo, fy);
			return;
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
