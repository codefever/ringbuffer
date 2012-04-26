#include "ringbuffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

#define PAGE_SIZE (4*1024)

struct ringbuffer
{
    char *addr;
    size_t count_bytes;
    size_t write_offset_bytes;
    size_t read_offset_bytes;
};

struct ringbuffer* rbuf_create(int pages)
{
    struct ringbuffer *rb = malloc(sizeof(*rb));
    if (!rb)
    {
        return 0;
    }

    rb->count_bytes = PAGE_SIZE*pages;
    rb->write_offset_bytes = rb->read_offset_bytes = 0;

    char path[] = "/var/tmp/ringbuffer-XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0)
    {
        free(rb);
        return 0;
    }
    unlink(path);

    if (ftruncate(fd, rb->count_bytes) < 0)
    {
        free(rb);
        return 0;
    }

    rb->addr = mmap(0, rb->count_bytes << 1, PROT_NONE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    if (rb->addr == MAP_FAILED)
    {
        free(rb);
        close(fd);
        return 0;
    }

    char *addr = mmap(rb->addr, rb->count_bytes, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd, 0);
    if (addr != rb->addr)
    {
        munmap(rb->addr, rb->count_bytes << 1);

        free(rb);
        close(fd);
        return 0;
    }

    addr = mmap(rb->addr + rb->count_bytes, rb->count_bytes, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd, 0);
    if (addr != (rb->addr + rb->count_bytes))
    {
        munmap(rb->addr, rb->count_bytes << 1);

        free(rb);
        close(fd);
        return 0;
    }

    close(fd);
    return rb;
}

void rbuf_destroy(struct ringbuffer *rb)
{
    if (rb)
    {
        if (munmap(rb->addr, rb->count_bytes * 2) < 0)
        {
            perror("munmap");
        }

        free(rb);
    }
}

size_t rbuf_free_bytes(struct ringbuffer *rb)
{
    assert(rb->write_offset_bytes >= rb->read_offset_bytes);

    return (rb->count_bytes - (rb->write_offset_bytes - rb->read_offset_bytes));
}

size_t rbuf_used_bytes(struct ringbuffer *rb)
{
    assert(rb->write_offset_bytes >= rb->read_offset_bytes);

    return (rb->write_offset_bytes - rb->read_offset_bytes);
}

char *rbuf_write_addr(struct ringbuffer *rb)
{
    return (rb->addr + rb->write_offset_bytes);
}

int rbuf_write_advance(struct ringbuffer *rb, size_t bytes)
{
    if (bytes <= rbuf_free_bytes(rb))
    {
        rb->write_offset_bytes += bytes;

        assert(rb->write_offset_bytes-rb->read_offset_bytes <= rb->count_bytes
            && rb->write_offset_bytes < rb->count_bytes << 1);
        return 0;
    }

    return -1;
}

char *rbuf_read_addr(struct ringbuffer *rb)
{
    return (rb->addr + rb->read_offset_bytes);
}

int rbuf_read_advance(struct ringbuffer *rb, size_t bytes)
{
    if (bytes <= rbuf_used_bytes(rb))
    {
        rb->read_offset_bytes += bytes;

        if (rb->read_offset_bytes >= rb->count_bytes)
        {
            rb->read_offset_bytes -= rb->count_bytes;
            rb->write_offset_bytes -= rb->count_bytes;
        }

        return 0;
    }

    return -1;
}

size_t rbuf_read(struct ringbuffer *rb, char *p, size_t len)
{
    assert(rb->write_offset_bytes >= rb->read_offset_bytes);

    size_t rlen = rbuf_used_bytes(rb);
    if (rlen <= 0)
    {
        errno = EAGAIN;
        return 0;
    }
    rlen = (rlen > len ? len : rlen);

    memcpy(p, rbuf_read_addr(rb), rlen);

    rbuf_read_advance(rb, rlen);

    return rlen;
}

size_t rbuf_write(struct ringbuffer *rb, const char *p, size_t len)
{
    assert(rb->write_offset_bytes >= rb->read_offset_bytes);

    size_t wlen = rbuf_free_bytes(rb);
    if (wlen <= 0)
    {
        errno = ENOSPC;
        return 0;
    }
    wlen = (wlen > len ? len : wlen);

    memcpy(rbuf_write_addr(rb), p, wlen);

    rbuf_write_advance(rb, wlen);

    return wlen;
}

#if 0
int main(int argc, const char *argv[])
{
    struct ringbuffer *rb = rbuf_create(1);
    if (rb == 0)
    {
        perror("rbuf_create");
        return -1;
    }

    char *page1 = rb->addr;
    char *page2 = rb->addr + rb->count_bytes;

    memset(page1, 0, rb->count_bytes);
    memset(page2, 0, rb->count_bytes);

    strcpy(page1, "hello wolrd!  goodbye world!");
    printf("%s\n", page2);

    rbuf_destroy(rb);
    return 0;
}
#endif

