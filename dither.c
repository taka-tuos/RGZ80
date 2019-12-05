#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdint.h>
#include <omp.h>

#define COLORS 16

int bayerpattern[16] = {
	  0,  8,  2, 10,
	 12,  4, 14,  6,
	  3, 11,  1,  9,
	 15,  7, 13,  5
};

int bayerpattern8[64] = {
	 1, 33,  9, 41,  3, 35, 11, 43,
	49, 17, 57, 25, 51, 19, 59, 27,
	13, 45,  5, 37, 15, 47,  7, 39,
	61, 29, 53, 21, 63, 31, 55, 23,
	 4, 36, 12, 44,  2, 34, 10, 42,
	52, 20, 60, 28, 50, 18, 58, 26,
	16, 48,  8, 40, 14, 46,  6, 38,
	64, 32, 56, 24, 62, 30, 54, 22,
};

uint8_t colorpalette[COLORS*3] = {
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

#define PI 3.14159265
#define RADIAN(d) ((d)*PI/180.0)

static double v25_7 = pow(25, 7);
static double d6 = RADIAN(6);
static double d25 = RADIAN(25);
static double d30 = RADIAN(30);
static double d60 = RADIAN(60);
static double d63 = RADIAN(63);
static double d275 = RADIAN(275);
static double kl = 1;
static double kc = 1;
static double kh = 1;
static double MAX = -1;

double dif(double l1, double a1, double b1, double l2, double a2, double b2){
	double dld = l2 - l1;
	double lb = (l1 + l2) / 2;

	double cs1 = hypot(a1, b1);
	double cs2 = hypot(a2, b2);
	double cb = (cs1 + cs2) / 2;
	double cb7 = pow(cb, 7);
	double ad1 = a1 + a1 / 2 * (1 - sqrt(cb7 / (cb7 + v25_7)));
	double ad2 = a2 + a2 / 2 * (1 - sqrt(cb7 / (cb7 + v25_7)));

	double cd1 = hypot(ad1, b1);
	double cd2 = hypot(ad2, b2);
	double cbd = (cd1 + cd2) / 2;
	double cbd7 = pow(cbd, 7);

	double dcd = (cd2 - cd1);
	double hd1 = b1 == 0 && ad1 == 0 ? 0 : atan2(b1, ad1);
	if(hd1 < 0){
		hd1 += PI * 2;
	}
	double hd2 = b2 == 0 && ad2 == 0 ? 0 : atan2(b2, ad2);
	if(hd2 < 0){
		hd2 += PI * 2;
	}

	double dhd = hd2 - hd1;
	if(cd1 * cd2 == 0){
		dhd = 0;
	} else if(abs(hd1 - hd2) > PI) {
		if(hd2 <= hd1){
			dhd += PI * 2;
		} else {
			dhd -= PI * 2;
		}
	}


	double dhhd = 2 * sqrt(cd1 * cd2) * sin(dhd / 2);
	double hhbd = 0;
	if(cd1 * cd2 != 0){
		hhbd = abs(hd1 - hd2) > PI ? ( hd1 + hd2 + PI * 2) / 2 : (hd1 + hd2) / 2;
	}

	double tt = 1
			- 0.17 * cos(hhbd - d30)
			+ 0.24 * cos(2 * hhbd)
			+ 0.32 * cos(3 * hhbd + d6)
			- 0.20 * cos(4 * hhbd - d63);
	double lb50_2 = pow(lb - 50, 2);
	double ssl = 1 + (0.015 * lb50_2) / sqrt(20 + lb50_2);
	double ssc = 1 + 0.045 * cbd;
	double ssh = 1 + 0.015 * cbd * tt;
	double rrt = -2.0 * sqrt(cbd7 / (cbd7 +v25_7)) * sin(d60 * exp(- pow((hhbd - d275)/ d25, 2)));
	double de = pow(dld / (kl * ssl), 2)
			+ pow(dcd / (kc * ssc), 2)
			+ pow(dhhd / (kh * ssh), 2)
			+ rrt * (dcd / (kc * ssc)) * (dhhd / (kh * ssh));

	return sqrt(de);
}

void rgb332_compatible(void)
{
	int r,g,b;

	for(r = 0; r < 8; r++) {
		for(g = 0; g < 8; g++) {
			for(b = 0; b < 4; b++) {
				int r8, g8, b8;

				r8 = (r << 5) | (r << 2) | (r >> 1);
				g8 = (g << 5) | (g << 2) | (g >> 1);
				b8 = (b << 6) | (b << 4) | (b << 2) | b;
			
				if(r8 > 255) r8 = 255;
				if(g8 > 255) g8 = 255;
				if(b8 > 255) b8 = 255;

				colorpalette[((r << 5) | (g << 2) | b) * 3 + 1] = r8;
				colorpalette[((r << 5) | (g << 2) | b) * 3 + 2] = g8;
				colorpalette[((r << 5) | (g << 2) | b) * 3 + 3] = b8;
			}
		}
	}
}


int nearcolor(int a, int b, int c)
{
	int mini = 0;
	double minl;
	
	minl = 32768 * 32768;
	
	if(MAX < 0) MAX = dif(1,1,1,0,0,0);
	
	for(int i = 0; i < COLORS; i++) {
		int pa = colorpalette[i*3+0];
		int pb = colorpalette[i*3+1];
		int pc = colorpalette[i*3+2];
		
		int al = pa - a;
		int bl = pb - b;
		int cl = pc - c;
		
		//int len = al*al + bl*bl + cl*cl;
		
		double len = dif(a/255.0f,b/255.0f,c/255.0f,pa/255.0f,pb/255.0f,pc/255.0f);
		
		if(len < minl) {
			minl = len;
			mini = i;
		}
	}
	
	//printf("%d,%d\n",mini,minl);
	
	return mini;
}

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("usage : %s <filename> <variable name>\n", argv[0]);
		return 1;
	}
	
	int w,h,bpp;
	stbi_uc *in = stbi_load(argv[1],&w,&h,&bpp,4);
	
	//printf("%dx%d %dbpp\n",w,h,bpp*8);
	
	stbi_uc *in2 = (stbi_uc *)malloc(w*(h/2)*4);
	stbi_uc *out = (stbi_uc *)malloc(w*h*4);
	
	//rgb332_compatible();
	
	for(int x = 0; x < w; x++) {
		for(int y = 0; y < h; y+=2) {
			uint32_t *p = ((uint32_t *) in) + (y*w+x);
			uint32_t *q = ((uint32_t *)in2) + ((y/2)*w+x);
			
			int a1 = *p & 0xff;
			int b1 = ((*p) >> 8) & 0xff;
			int c1 = ((*p) >> 16) & 0xff;
			
			int a2 = p[w] & 0xff;
			int b2 = ((p[w]) >> 8) & 0xff;
			int c2 = ((p[w]) >> 16) & 0xff;
			
			*q = ((a1+a2)/2) | (((b1+b2)/2) << 8) | (((c1+c2)/2) << 16) | 0xff000000;
		}
	}
	
	printf("unsigned char %s[] = \x7b\n", argv[2]);
	
	for(int y = 0; y < h; y+=2) {
		int b4 = 0;
		printf("\t");
		for(int x = 0; x < w; x++) {
			uint32_t *p = ((uint32_t *)in2) + ((y/2)*w+x);
			uint32_t *q = ((uint32_t *)out) + (y*w+x);
			
			*q = 0xff000000;
			
			int a = *p & 0xff;
			int b = ((*p) >> 8) & 0xff;
			int c = ((*p) >> 16) & 0xff;
			
			//int bias = bayerpattern[(y%4)*4 + (x%4)]*4;
			int bias = bayerpattern8[((y/2)%8)*8 + (x%8)];
			//int bias = 0;
			
			int pal = nearcolor(a+bias,b+bias,c+bias);
			
			int *r = (int *)(colorpalette + (pal * 3));
			
			if((x & 1) == 0) {
				b4 = pal << 4;
			} else {
				b4 |= pal;
				printf("0x%02x, ", b4);
			}
			
			*q |= *r;
			
			q[w] = *q;
		}
		printf("\n");
	}
	
	printf("\x7d\n");
	
	stbi_write_png("result.png",w,h,4,out,0);
	
	return 0;
}
