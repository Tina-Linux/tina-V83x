#ifndef __OUTLOG_H
#define __OUTLOG_H

#include "mjson.h"
#include "task.h"
#include "syskey.h"
#include "interact.h"

#define OUTLOG_PATH "/sys/global/info/outlog"

typedef int (*before_all) (struct list_head *TASK_LIST); //所有任务开始前
typedef int (*before_one) (struct task *task);          //单个任务开始前
typedef int (*after_one_begin) (struct task *task);     //单个任务开始后
typedef int (*after_one_once) (struct task *task);      //单个任务执行一次后
typedef int (*after_one_end) (struct task *task);       //单个任务完成后
typedef int (*after_all) (struct list_head *TASK_LIST);  //所有任务完成后

int outlog_register(
        before_all b_all,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all);

int outlog_register_ex(
        before_all b_all,
        before_one b_one,
        after_one_begin a_one_begin,
        after_one_once a_one_once,
        after_one_end a_one_end,
        after_all a_all);

// 下列函数由 tinatest 调用
int outlog_init(void);      //根据tinatest.json注册插件列表
int outlog_call_before_all(struct list_head *TASK_LIST);     //调用'所有任务开始前'注册的函数
int outlog_call_before_one(struct task *task);               //调用'单个任务开始前'注册的函数
int outlog_call_after_one_begin(struct task *task);          //调用'单个任务开始后'注册的函数
int outlog_call_after_one_once(struct task *task);           //调用'单个任务执行一次后'注册的函数
int outlog_call_after_one_end(struct task *task);            //调用'单个任务完成后'注册的函数
int outlog_call_after_all(struct list_head *TASK_LIST);      //调用'所有任务完成后'注册的函数

#endif
