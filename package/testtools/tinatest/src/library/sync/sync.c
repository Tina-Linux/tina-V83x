#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "task.h"

// These functions is based on code in book
// <<Advanced Programming in the UNIX Environment>>
// To realize synchronous communication between father process and child process

static struct list_head SYNC_LIST;

struct msync{
    int id;
    int father_fd[2]; //father read, child write
    int child_fd[2]; //child read, fahter write

    struct list_head lnode;
};

int TELL_WAIT(int id)
{
    if (SYNC_LIST.prev == NULL || SYNC_LIST.next == NULL)
        INIT_LIST_HEAD(&SYNC_LIST);

    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            ERROR("id %d is allready existed\n", id);
            return -1;
        }
    }

    msync = NULL;
    msync = malloc(sizeof(struct msync));
    if (msync == NULL) {
        ERROR("malloc failed - %s\n", strerror(errno));
        goto out1;
    }

    if (pipe(msync->father_fd) < 0 || pipe(msync->child_fd) < 0) {
        ERROR("pipe create error - %s\n", strerror(errno));
        goto out1;
    }

    list_add_tail(&msync->lnode, &SYNC_LIST);
    msync->id = id;
    goto out;

out1:
    close(msync->father_fd[0]);
    close(msync->father_fd[1]);
    close(msync->child_fd[0]);
    close(msync->child_fd[1]);
    free(msync);
    return -1;
out:
    return 0;
}

int TELL_PARENT(int id)
{
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (write(msync->father_fd[1], "c", 1) != 1)
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int WAIT_PARENT(int id)
{
    char c;
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (read(msync->child_fd[0], &c, 1) != 1)
                goto out1;

            if (c != 'p')
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int TELL_CHILD(int id)
{
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (write(msync->child_fd[1], "p", 1) != 1)
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}

int WAIT_CHILD(int id)
{
    char c;
    struct msync *msync = NULL;
    list_for_each_entry(msync, &SYNC_LIST, lnode) {
        if (msync->id == id) {
            if (read(msync->father_fd[0], &c, 1) != 1)
                goto out1;

            if (c != 'c')
                goto out1;
            goto out;
        }
    }

out1:
    return -1;
out:
    return 0;
}
