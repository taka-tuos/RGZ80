#include <stdint.h>

#ifndef __FIFO__
#define __FIFO__

typedef struct {
	uint8_t *buf;
	int p, q, size, free, flags;
} fifo_t;

void fifo_init(fifo_t *fifo, int size, uint8_t *buf);
int fifo_put(fifo_t *fifo, uint8_t data);
uint8_t fifo_get(fifo_t *fifo);
int fifo_status(fifo_t *fifo);

#endif
