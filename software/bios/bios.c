#include "bios.h"

#define ADDR_FREQ_A 0
#define ADDR_FREQ_B 1
#define ADDR_FREQ_C 2

#define pitchbend
#define programchange
#define tone(a,b,c) set_ch(a,b)

const int noteFreq[] = {
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

void main()
{
#include "song.h"
	
	while(1);
}
