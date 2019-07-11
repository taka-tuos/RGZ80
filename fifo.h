#ifndef __FIFO__
#define __FIFO__

typedef struct {
	void **buf;
	int p, q, size, free, flags;
} fifo32_t;
void fifo32_init(fifo32_t *fifo, int size, void **buf);
int fifo32_put(fifo32_t *fifo, void *data);
void *fifo32_get(fifo32_t *fifo);
int fifo32_status(fifo32_t *fifo);

#endif
