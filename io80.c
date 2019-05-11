#include "RGZ80.h"

extern byte *z80_memory;
extern int psg_period[3];
extern int psg_volume[3];
extern SDL_Joystick *joy;

int rgz_wait = 0;

byte io80_readm(int param, ushort address)
{
	rgz_wait++;
	return z80_memory[address];
}

void io80_writem(int param, ushort address, byte data)
{
	rgz_wait++;
	z80_memory[address] = data;
}

byte io80_readp(int param, ushort address)
{
	int ret = 0;

	int kmode[8];

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
	}

	return ret;
}

void io80_writep(int param, ushort address, byte data)
{
	int tmp;
	
	switch(address & 0xff) {
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5:
		rgz_wait += 16;
		
		tmp = psg_period[(address >> 1) & 3];
		
		if(address & 1) tmp &= 0x0f;
		else tmp &= 0xf0;
		
		tmp |= data << ((address & 0x0f) * 8);
		
		psg_period[(address >> 1) & 3] = tmp;
		
		break;
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5:
		rgz_wait += 16;
		
		tmp = psg_volume[(address >> 1) & 3];
		
		if(address & 1) tmp &= 0x0f;
		else tmp &= 0xf0;
		
		tmp |= data << ((address & 0x0f) * 8);
		
		psg_volume[(address >> 1) & 3] = tmp;
		
		break;
	}
}
