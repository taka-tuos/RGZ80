#include "a3d.h"
#include <math.h>
#include <stdint.h>

#define R_8(rgb) ((rgb >> 16) & 0xff)
#define G_8(rgb) ((rgb >>  8) & 0xff)
#define B_8(rgb) ((rgb >>  0) & 0xff)

int a3d_lnstep(a3d_liner *o)
{
	o->y1 += o->dy;
	o->y0 = (o->y1 + (QUOL2_NUM >> 1)) >> QUOL2;

	return 0;
}

void a3d_lninit(a3d_liner *o, int x0, int x1)
{
	int a = x0;
	int b = x1;
	int l = (b - a);

	o->x0 = a;
	o->x1 = b;

	o->y0 = 0;
	o->y1 = 0;

	if (l) o->dy = (QUOL_NUM << QUOL2) / l;
	else o->dy = 0;
}

#define SMOOTH 0
#define TEXTURE 0
#define VERTEX 3
void a3d_drawpoly_F3(a3d_vertex *v)
#include "a3d_sub.h"

#define SMOOTH 0
#define TEXTURE 1
#define VERTEX 3
void a3d_drawpoly_FT3(a3d_vertex *v)
#include "a3d_sub.h"

#define SMOOTH 1
#define TEXTURE 0
#define VERTEX 3
void a3d_drawpoly_G3(a3d_vertex *v)
#include "a3d_sub.h"

#define SMOOTH 1
#define TEXTURE 1
#define VERTEX 3
void a3d_drawpoly_GT3(a3d_vertex *v)
#include "a3d_sub.h"
