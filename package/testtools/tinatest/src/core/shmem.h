/*
 * shm: 共享内存(share memory)
 * 共享内存用于tinatest以及各个子进程之间的数据共享
 */
#ifndef __SHMEM_H
#define __SHMEM_H

#include <pthread.h>
#include <stdbool.h>

// 此处共享内存并使用互斥锁，原因如下：
// 1. 在tinatest的架构中，同一时间只有1个线程会对共享内存写，无需互斥
// 2. 共享主要用于限制子进程，然而限制子进程随时会被kill，且不能调用信号不安全接口，因此不能申请锁
#pragma pack(1)
struct shmem_testcase {
    pid_t pgid;             //测试用例的进程组号，用于超时kill
    bool quit;              //测试用例超时
};
struct shmem_tinatest {
    volatile bool quit;          //退出TinaTest的标志
};
#pragma pack()

// 申请和初始化共享内存
extern struct shmem_tinatest *shmem_init(int _max_num);
// 释放共享内存
extern int shmem_free();
// 寻找或创建测试用例共享内存结构
extern struct shmem_testcase *shmem_probe(char *keypath);

#endif
