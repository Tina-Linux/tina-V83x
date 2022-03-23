#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "speed.h"
#include "log.h"

static struct timeval start;
static struct timeval end;

int speed_start(void)
{
    if (gettimeofday(&start, NULL)) {
        ERROR("gettimeofday failed - %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

float speed_end(unsigned long len)
{
    unsigned long start_ms, end_ms;
    float ms;

    if (gettimeofday(&end, NULL)) {
        ERROR("gettimeofday failed - %s\n", strerror(errno));
        return -1;
    }

    start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
    end_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
    ms = (double)end_ms - (double)start_ms;
    return (len / (ms / 1000));
}
