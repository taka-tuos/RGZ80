#include "RGZ80.h"
#include <stdint.h>
#include "z80user.h"
#include "fifo.h"

extern byte *z80_memory;
extern Z80Context *g_z80;
extern int psg_period[3];
extern int psg_volume[3];
extern int psg_count[3];
extern SDL_Joystick *joy;
extern int rgz_vsync;

fifo_t *gpu_fifo;

int rgz_wait = 0;

int rgz_time = 0;
int rgz_addr = 0;

#define TICKS (g_z80->tstates / (6000))

byte io80_readm(int param, ushort address)
{
	return z80_memory[address];
}

void io80_writem(int param, ushort address, byte data)
{
	z80_memory[address] = data;
}

byte io80_readp(int param, ushort address)
{
	int ret = 0;

	int kmode[8];
	
	//printf("READ %02x\n",address & 0xff);

	switch(address & 0xff) {
	case 0x80:
		{
			rgz_wait += 32;
			memset(kmode,0,sizeof(kmode));
			
			Uint8 *keystate = SDL_GetKeyState(NULL);

			if(SDL_JoystickGetButton(joy,1) || keystate[SDLK_z]) {
				kmode[RG_TRIG1] = 1;	// TRIG1
			}
			if(SDL_JoystickGetButton(joy,2) || keystate[SDLK_x]) {
				kmode[RG_TRIG2] = 1;	// TRIG2
			}
			if(SDL_JoystickGetButton(joy,10) || keystate[SDLK_c]) {
				kmode[RG_SELECT] = 1;	// select
			}
			if(SDL_JoystickGetButton(joy,11) || keystate[SDLK_v]) {
				kmode[RG_START] = 1;	// start
			}

			if(SDL_JoystickGetButton(joy,8) || keystate[SDLK_UP]) {
				kmode[RG_UP] = 1;		// up
			}
			if(SDL_JoystickGetButton(joy,6) || keystate[SDLK_DOWN]) {
				kmode[RG_DOWN] = 1;		// down
			}
			if(SDL_JoystickGetButton(joy,7) || keystate[SDLK_LEFT]) {
				kmode[RG_LEFT] = 1;		// left
			}
			if(SDL_JoystickGetButton(joy,9) || keystate[SDLK_RIGHT]) {
				kmode[RG_RIGHT] = 1;	// right
			}
		}
		ret |= kmode[RG_TRIG1];
		ret |= kmode[RG_TRIG2] << 1;
		ret |= kmode[RG_LEFT] << 2;
		ret |= kmode[RG_UP] << 3;
		ret |= kmode[RG_DOWN] << 4;
		ret |= kmode[RG_RIGHT] << 5;
		ret |= kmode[RG_SELECT] << 6;
		ret |= kmode[RG_START] << 7;
		break;
	case 0x81:
		if(TICKS >= rgz_time) ret |= 1;
		break;
	case 0x82:
		ret = rgz_vsync;
		rgz_vsync = 0;
		break;
	case 0xc1:
		ret = fifo_get(gpu_fifo);
		break;
	}

	return ret;
}

void io80_writep(int param, ushort address, byte data)
{
	int tmp;
	
	switch(address & 0xff) {
	case 0x81:
		rgz_time = TICKS + data + 1;
		break;
	case 0xb0: case 0xb1:
		tmp = rgz_addr;
		
		if(address & 1) tmp &= 0x00ff;
		else tmp &= 0xff00;
		
		tmp |= data << ((address & 0x01) * 8);
		rgz_addr = tmp;
		//printf("%04x\n",rgz_addr);
		break;
	case 0xc0:
		vdp_flush();
		break;
	case 0xc1:
		fifo_put(gpu_fifo,data);
		break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5:
		rgz_wait += 16;
		
		tmp = psg_period[(address >> 1) & 3];
		
		if(address & 1) tmp &= 0x00ff;
		else tmp &= 0xff00;
		
		tmp |= data << ((address & 0x01) * 8);
		
		psg_count[(address >> 1) & 3] = 0;
		
		psg_period[(address >> 1) & 3] = tmp;
		
		//printf("[%d]%d Hz(%04x,%02x,%02x)\n",(address >> 1) & 3,44100/tmp,tmp,address,data);
		
		break;
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5:
		rgz_wait += 16;
		
		tmp = psg_volume[(address >> 1) & 3];
		
		if(address & 1) tmp &= 0x00ff;
		else tmp &= 0xff00;
		
		tmp |= data << ((address & 1) * 8);
		
		psg_volume[(address >> 1) & 3] = tmp;
		
		//printf("[%d]%d %(%04x,%02x,%02x)\n",(address >> 1) & 3,tmp*100/8192,tmp,address,data);
		
		break;
	}
}
