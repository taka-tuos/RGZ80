#include "bios.h"

static const int costable[] = {
	64,
	63,
	63,
	63,
	63,
	63,
	63,
	63,
	62,
	62,
	62,
	61,
	61,
	60,
	60,
	59,
	59,
	58,
	57,
	57,
	56,
	55,
	54,
	54,
	53,
	52,
	51,
	50,
	49,
	48,
	47,
	46,
	45,
	44,
	42,
	41,
	40,
	39,
	38,
	36,
	35,
	34,
	32,
	31,
	30,
	28,
	27,
	25,
	24,
	23,
	21,
	20,
	18,
	17,
	15,
	14,
	12,
	10,
	9,
	7,
	6,
	4,
	3,
	1
};

int cos(int x)
{
	if(x < 0) {
		x = -x;
		x &= 255;
		x = 255 - x;
	} else {
		x &= 255;
	}
	unsigned char y = x & 63;
	if(x < 64) {
		return costable[y];
	} else if(x >= 64 && x < 128) {
		return -costable[63 - y];
	} else if(x >= 128 && x < 192) {
		return -costable[y];
	} else {
		return costable[63 - y];
	}
}

int sin(int x)
{
	return cos(63 + x);
}

#define ADDR_FREQ_A 0
#define ADDR_FREQ_B 1
#define ADDR_FREQ_C 2

#define pitchbend
#define programchange
#define tone(a,b,c) set_ch(a,b)

static const int noteFreq[] = {
	0, 2450, 2450, 2205, 2205, 2004, 1837, 1837, 1696, 1575, 1470, 1470, 
	1378, 1297, 1225, 1160, 1050, 1002, 958, 882, 848, 787, 760, 
	711, 668, 630, 595, 565, 537, 501, 479, 450, 424, 400, 
	380, 355, 339, 319, 302, 282, 268, 253, 237, 225, 212, 
	200, 188, 177, 168, 158, 150, 141, 133, 126, 119, 112, 
	106, 100, 94, 89, 84, 79, 75, 70, 66, 63, 59, 
	56, 53, 50, 47, 44, 42, 39, 37, 35, 33, 31, 
	29, 28, 26, 25, 23, 22, 21, 19, 18, 17, 16, 
	15, 14, 14, 13, 12, 11, 11, 10, 9, 9, 8, 
	8, 7, 7, 7, 6, 6, 5, 5, 5, 4, 4, 
	4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 
	2, 2, 2, 1, 1, 1, 
};

void delay(int n)
{
	int m = n >> 8;
	int o = n & 255;
	
	while(m) {
		io_out(0x81,255);
		while(!io_in(0x81));
		m--;
	}
	
	io_out(0x81,o);
	while(!io_in(0x81));
}

void set_ch(int ch, int no)
{
	int freq = noteFreq[no];
	int regofs = ch << 1;
	
	if(!no) {
		io_out(0xf0+regofs+0,0);
		io_out(0xf0+regofs+1,0);
		return;
	}
	
	io_out(0xe0+regofs+0,freq & 255);
	io_out(0xe0+regofs+1,freq >> 8);
	
	io_out(0xf0+regofs+0,255);
	io_out(0xf0+regofs+1,0);
}

void fifoput8(unsigned char b)
{
	io_out(0xc1,b);
}

void fifoput16s(short w)
{
	fifoput8(w & 0xff);
	fifoput8((w >> 8) & 0xff);
}

void drawpoly(int x1, int y1, int x2, int y2, int x3, int y3, int c)
{
	fifoput8(4);
	fifoput16s(x1);
	fifoput16s(y1);
	fifoput16s(x2);
	fifoput16s(y2);
	fifoput16s(x3);
	fifoput16s(y3);
	fifoput8(c);
	/*short v[2+2+2+1];
	
	v[0] = x1;
	v[1] = y1;
	
	v[2] = x2;
	v[3] = y2;
	
	v[4] = x3;
	v[5] = y3;
	
	v[6] = c;
	
	unsigned int vl = (((unsigned int)v)&0xff);
	unsigned int vh = (((unsigned int)v)>>0x8);
	
	io_out(0xb0,vl);
	io_out(0xb1,vh);
	
	io_out(0xc0,0);*/
}

static const int sbox[2*4] = {
	-12, -12,
	-12, 12,
	12, 12,
	12, -12
};

#define RX(x,y,r) (((x) * cos(i)) >> 6) - (((y) * sin(i)) >> 6)
#define RY(x,y,r) (((x) * sin(i)) >> 6) + (((y) * cos(i)) >> 6)

#define NUM 128

void main()
{
	static int x[NUM],y[NUM];
	static int u[NUM],v[NUM];
	static unsigned char c[NUM],o[NUM];
	
	srand();
	
	for(int i=0;i<NUM;i++) {
		x[i] = rand() % (256*16);
		y[i] = rand() % (192*16);
		
		u[i] = (rand() % 32) - 16;
		v[i] = (rand() % 32) - 16;
		
		c[i] = rand() % 15+1;
		o[i] = 0; 
	}
	
	while(1) {
		drawpoly(0,0,512,0,512,192,0);
		drawpoly(0,0,0,192,512,192,0);
		for(int i=0;i<NUM;i++) {
			int tbox[2*4];
			
			int q,p;
			int r = o[i];
			
			x[i] += u[i];
			y[i] += v[i];
			
			q = x[i] >> 4;
			p = y[i] >> 4;
			
			if(x[i] >= 256*16 || x[i] < 0) u[i] = -u[i];
			if(y[i] >= 192*16 || y[i] < 0) v[i] = -v[i];
			
			//q = 128;
			//p = -2;
			
#if 0
			tbox[0] = RX(sbox[0],sbox[1],r) + q;
			tbox[1] = RY(sbox[0],sbox[1],r) + p;
			
			tbox[2] = RX(sbox[2],sbox[3],r) + q;
			tbox[3] = RY(sbox[2],sbox[3],r) + p;
			
			tbox[4] = RX(sbox[4],sbox[5],r) + q;
			tbox[5] = RY(sbox[4],sbox[5],r) + p;
			
			tbox[6] = RX(sbox[6],sbox[7],r) + q;
			tbox[7] = RY(sbox[6],sbox[7],r) + p;
#endif
			
			tbox[0] = sbox[0] + q;
			tbox[1] = sbox[1] + p;
			
			tbox[2] = sbox[2] + q;
			tbox[3] = sbox[3] + p;
			
			tbox[4] = sbox[4] + q;
			tbox[5] = sbox[5] + p;
			
			tbox[6] = sbox[6] + q;
			tbox[7] = sbox[7] + p;
			
			drawpoly(tbox[0]*2,tbox[1],tbox[2]*2,tbox[3],tbox[4]*2,tbox[5],c[i]);
			drawpoly(tbox[0]*2,tbox[1],tbox[6]*2,tbox[7],tbox[4]*2,tbox[5],c[i]);
		}
		
		io_out(0xc0,0);
		io_out(0x82,0);
	}
}
