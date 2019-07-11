#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef __A3D__
#define __A3D__

#define QUOL 8
#define QUOL_NUM (1 << QUOL)

#define QUOL2 8
#define QUOL2_NUM (1 << QUOL2)

#define FIX_MIX_EX(a1,a2,f) (a2 * f + a1 * (QUOL_NUM - f))
#define FIX_MIX_NM(a1,a2,f) (a2 * f + a1 * (QUOL_NUM - f) + (QUOL_NUM >> 1)) >> QUOL
#define FIX_MIX(a1,a2,f) (a2 * f + a1 * (QUOL_NUM - f) + (1 << ((QUOL * 2)-1))) >> (QUOL * 2)

#define MIN_X 0
#define MAX_X 512
#define MIN_Y 0
#define MAX_Y 192

typedef struct {
	int x,y,u,v,c;
} a3d_vertex;

typedef struct {
	int dx,dy,err,a,e2,sx,sy;
	int x0,y0,x1,y1,x2,y2;
} a3d_liner;

void a3d_drawpoly_F3(a3d_vertex *v);
void a3d_drawpoly_FT3(a3d_vertex *v);
void a3d_drawpoly_G3(a3d_vertex *v);
void a3d_drawpoly_GT3(a3d_vertex *v);

extern void PixelShader(int x, int y, int u, int v, int r, int g, int b);

#endif
