#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <unistd.h>

struct ringbuffer;

struct ringbuffer* rbuf_create(int pages);
void rbuf_destroy(struct ringbuffer *rb);

size_t rbuf_free_bytes(struct ringbuffer *rb);
size_t rbuf_used_bytes(struct ringbuffer *rb);

char *rbuf_write_addr(struct ringbuffer *rb);
int rbuf_write_advance(struct ringbuffer *rb, size_t bytes);
char *rbuf_read_addr(struct ringbuffer *rb);
int rbuf_read_advance(struct ringbuffer *rb, size_t bytes);

/* r/w like fd */
size_t rbuf_read(struct ringbuffer *rb, char *p, size_t len);
size_t rbuf_write(struct ringbuffer *rb, const char *p, size_t len);

#endif //RINGBUFFER_H_
