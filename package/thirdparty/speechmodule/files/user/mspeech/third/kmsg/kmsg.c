
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "kmsg.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

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

int kmsg_new(struct kmsg *buf, int size) {
    int ret = 0;
    if (buf) {
        size = roundup_pow_of_two(size);
        buf->size = size;
        buf->in = 0;
        buf->out = 0;
        buf->data = NULL;
        buf->msg_number = 0;
        for (int i = 0; i < MAX_MSG_NUMBER; i++)
            buf->msg_part_len[i] = 0;

        if (pthread_mutex_init(&buf->mutex, NULL) < 0) return -1;

        if (size > 0) {
            buf->data = (unsigned char *)malloc(sizeof(char) * size);
            if (!buf->data) ret = -1;
        }

    }
    else ret = -1;

    return ret;
}


int kmsg_delete(struct kmsg *buf) {
    int ret;
    if (buf) {
        if (buf->data) {
            free(buf->data);
            buf->data = NULL;
            pthread_mutex_destroy(&buf->mutex);
            memset(buf, 0, sizeof(struct kmsg));
            ret = 0;
        }
        else ret = -1;
    }
    else ret = -1;

    return ret;
}


int kmsg_push(struct kmsg *buf, const char *data, int len) {

    pthread_mutex_lock(&buf->mutex);

    unsigned int l;
    assert((buf->size - buf->in + buf->out) > len); // 必须保证有足够的空间
    len = min(len, buf->size - buf->in + buf->out);

    l = min(len, buf->size - (buf->in & (buf->size - 1)));
    memcpy(buf->data + (buf->in & (buf->size - 1)), data, l);
    memcpy(buf->data, data + l, len - l);

    buf->in += len;

    assert((buf->msg_number + 1) < MAX_MSG_NUMBER);
    buf->msg_part_len[buf->msg_number] = len;
    buf->msg_number += 1;

    pthread_mutex_unlock(&buf->mutex);

    return len;
}


int kmsg_check(struct kmsg *buf) {
    if (buf->msg_number > 0) {
        // 返回第一个 msg part 的长度
        return buf->msg_part_len[0];
    }
    else return 0;
}


int kmsg_pop(struct kmsg *buf, char *data) {

    if (buf->msg_number <= 0) return 0;

    pthread_mutex_lock(&buf->mutex);

    int len;
    unsigned int l;
    len = buf->msg_part_len[0];

    l = min(len, buf->size - (buf->out & (buf->size - 1)));

    memcpy(data, buf->data + (buf->out & (buf->size - 1)), l);
    memcpy(data + l, buf->data, len - l);

    buf->out += len;

    // 对 msg part 重新排列
    buf->msg_number -= 1;
    for (int i = 0; i < buf->msg_number; i++) {
        buf->msg_part_len[i] = buf->msg_part_len[i + 1];
    }

    pthread_mutex_unlock(&buf->mutex);

    return len;
}
