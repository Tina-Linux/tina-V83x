
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "kfifo.h"


#define min(x, y) ((x) < (y) ? (x) : (y))
#define barrier() __asm__ __volatile__("": : :"memory")

/*
 * ref: lua-5.1 source code
 */
#define ceillog2(x) (luaO_log2_xx((x)-1) + 1)

static int luaO_log2_xx (unsigned int x) {
    static const int log_2[256] = {
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
    };
    int l = -1;
    while (x >= 256) { l += 8; x >>= 8; }
    return l + log_2[x];
}


static int roundup_pow_of_two(int size) {
    int round_log_of_two;
    round_log_of_two = ceillog2(size);
    return 1 << round_log_of_two;
}


int kfifo_new(struct kfifo *buf, int size) {
    int ret = 0;
    if (buf) {
        size = roundup_pow_of_two(size);
        buf->size = size;
        buf->in = 0;
        buf->out = 0;
        buf->data = NULL;

        if (size > 0) {
            buf->data = (unsigned char *)malloc(sizeof(char) * size);
            if (!buf->data) ret = -1;
        }
    }
    else ret = -1;

    return ret;
}


int kfifo_delete(struct kfifo *buf) {
    int ret;
    if (buf) {
        if (buf->data) {
            free(buf->data);
            buf->data = NULL;
            memset(buf, 0, sizeof(struct kfifo));
            ret = 0;
        }
        else ret = -1;
    }
    else ret = -1;

    return ret;
}


int kfifo_push(struct kfifo *buf, const char *data, int len) {

    unsigned int l;
    len = min(len, buf->size - buf->in + buf->out);

    // smp_mb();
    barrier();

    l = min(len, buf->size - (buf->in & (buf->size - 1)));
    memcpy(buf->data + (buf->in & (buf->size - 1)), data, l);
    memcpy(buf->data, data + l, len - l);

    //smp_wmb();
    barrier();

    buf->in += len;
    return len;
}


int kfifo_pop(struct kfifo *buf, char *data, int len) {

    unsigned int l;
    len = min(len, buf->in - buf->out);

    //smp_rmb();
    barrier();
    l = min(len, buf->size - (buf->out & (buf->size - 1)));

    memcpy(data, buf->data + (buf->out & (buf->size - 1)), l);
    memcpy(data + l, buf->data, len - l);

    //smp_mb();
    barrier();

    buf->out += len;
    return len;
}


int kfifo_empty(struct kfifo *buf) {

    buf->in= buf->out=0;

    return 0;
}
