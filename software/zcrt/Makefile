MAKE		= make -r
SDASZ80		= sdasz80

default :
	$(MAKE) crt

crt : zcrt0.s Makefile
	$(SDASZ80) -o zcrt0.o zcrt0.s
