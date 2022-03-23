// #include <stdarg.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <endian.h>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "watcher/EventWatcher.h"
#include "threading/Executor.h"

using namespace AW;

int main(int argc, char *argv[]) {

    int a = 1;
    int b = 2;
    int c = 10;
    printf("main tid: %ld\n", syscall(SYS_gettid));
    auto executor = Executor::create(10);

    for(int i = 0; i< 10; i++){
        executor->submit([c]() {
            printf("a = %d tid:%ld\n", c, syscall(SYS_gettid));
            sleep(2);
            return 0;
        });
        a++;
    }

    printf("end!!\n");

    while(1);
}
