#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>

#include "ringbuffer.h"

int main(int argc, const char *argv[])
{
    struct ringbuffer *rb = rbuf_create(1);
    if (rb == 0)
    {
        perror("rb_create");
        return -1;
    }

    char *s = "0123456789ABCDEFuiocapewebvmzalwqwui";
    size_t len = strlen(s);
    char buf[234];
    size_t nret = 0;
    int count = 100; //write time
    bool wr = true;

    srand(time(0));

    int wfd = open("wfile", O_CREAT|O_WRONLY, 0644);
    if (wfd < 0)
    {
        goto byebye;
    }
    int rfd = open("rfile", O_CREAT|O_WRONLY, 0644);
    if (rfd < 0)
    {
        goto byebye;
    }

    for (;;)
    {
        while (wr)
        {
            if (--count <= 0)
            {
                wr = false;
            }

            size_t wlen = rand()%len;
            nret = rbuf_write(rb, s, wlen);
            
            if (nret == 0) //full
            {
                break;
            }
            else
            {
                write(wfd, s, nret);
            }
        }

        int rcount = rand()%10 + 1;
        if (!wr)
        {
            rcount = 0x7FFFFFFF;
        }
        while (rcount--)
        {
            nret = rbuf_read(rb, buf, sizeof(buf));

            if (nret == 0) //empty
            {
                break;
            }
            else
            {
                write(rfd, buf, nret);
            }
        }

        if (!wr)
        {
            break;
        }
    }

byebye:
    rbuf_destroy(rb);
    return 0;
}

