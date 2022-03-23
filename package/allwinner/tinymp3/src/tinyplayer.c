#include <stdio.h>
#include <mp3player.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
/*
 * argv[1]: Mp3 file path
 * argv[2]: Speaker Function only for sunxicodec
 * argv[3]: headphone volume control volume only for sunxicodec
 */
static void get_file_type(struct stat *st, char *type, int len)
{
    switch (st->st_mode & S_IFMT) {
    case S_IFBLK:
        strncpy(type, "block device", len);
        break;
    case S_IFCHR:
        strncpy(type, "character device", len);
        break;
    case S_IFDIR:
        strncpy(type, "directory", len);
        break;
    case S_IFIFO:
        strncpy(type, "FIFO/pipe", len);
        break;
    case S_IFLNK:
        strncpy(type, "symlink", len);
        break;
    default:
        strncpy(type, "unknown file", len);
    }
}

static int filldata(char *buf, int size, void *user)
{
    return read((int)user, buf, size);
}
static off_t seekdata(off_t offset, int whence, void *user)
{
    return lseek((int)user, offset, whence);
}

tiny_mp3_ops_t ops = {
    .read = filldata,
    .lseek = seekdata,
};
struct thread_args
{
    tinymp3_ctx_t *ctx;
    const char *file;
};

void *thread1(void *junk)
{
    struct thread_args *args = (struct thread_args *)junk;
    tinymp3_ctx_play(args->ctx);
}

pthread_t create_thread(tinymp3_ctx_t *ctx, const char *file)
{
    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->ctx = ctx;
    args->file = file;
    pthread_t tid;
    pthread_create(&tid, NULL, thread1, (void *)args);
    return tid;
}

void test_set_volume(tinymp3_ctx_t *ctx, const char *file)
{
    pthread_t tid = create_thread(ctx, file);
    int volume = 0;
    while(volume <= 40){
        tinymp3_ctx_setvolume(ctx, volume++);
        usleep(50*1000);
    }
    pthread_join(tid, NULL); //join thread
}

void test_stop(tinymp3_ctx_t *ctx, const char *file)
{
    tinymp3_ctx_setvolume(ctx, 10);
    pthread_t tid = create_thread(ctx, file);

    usleep(1500*1000);

    tinymp3_ctx_play(ctx); // fail, return -1
    tinymp3_ctx_destroy(&ctx); // fail, return -1
    tinymp3_ctx_stop(ctx); //block for wait play finish
    tinymp3_ctx_play(ctx); //play new

    pthread_join(tid, NULL); //join thread
}

int main(int argc, char *argv[])
{
    char *mp3_file = argv[1];
    struct stat stat;

    printf("create\n");
    tinymp3_ctx_t *ctx = tinymp3_ctx_create(NULL, NULL);
    printf("play\n");

    tinymp3_ctx_prepare_file(ctx, mp3_file);
    tinymp3_ctx_play_file(ctx);

    tinymp3_ctx_stop(ctx);
    tinymp3_ctx_destroy(&ctx);
    /*
    tinymp3_ctx_t *ctx = tinymp3_ctx_create();

    test_set_volume(ctx, argv[1]);
    test_stop(ctx, argv[1]);

    tinymp3_ctx_destroy(&ctx);
    */
	return 0;
}
