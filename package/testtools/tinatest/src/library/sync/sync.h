#ifndef __MSYNC_H
#define __MSYNC_H

#define TASK_ADD_RECYCLE_ID 1
#define COLLECTD_DO_ID 2

// These functions is based on code in book
// <<Advanced Programming in the UNIX Environment>>
// To realize synchronous communication between father process and child process
int TELL_WAIT(int id);
int TELL_PARENT(int id);
int WAIT_PARENT(int id);
int TELL_CHILD(int id);
int WAIT_CHILD(int id);

#endif
