#ifndef _KFIFO_H_
#define _KFIFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *
 * kfifo.h
 *
 * 循环无锁队列(单生产者 + 单消费者)
 * ref: linix/kernel/lib/kfifo.c
 */


struct kfifo {
    unsigned int size;   // buffer capcacity
    unsigned int out;
    unsigned int in;
    unsigned char *data;
};


/*
 * new the kfifo with capacity
 * return -1 if any error happen
 */
int kfifo_new(struct kfifo *buf, int size);

/*
 * delete the kfifo memory
 * return -1 if any error happen
 */
int kfifo_delete(struct kfifo *buf);

/*
 * push the len data to the kfifo
 * return real push len if push some data
 */
int kfifo_push(struct kfifo *buf, const char *data, int len);

/*
 * pop the len data from the kfifo
 * return the real pop len
 */
int kfifo_pop(struct kfifo *buf, char *data, int len);

/*
 * empty the len data from the kfifo
 * return 0
 */
int kfifo_empty(struct kfifo *buf);



#ifdef __cplusplus
}
#endif


#endif
