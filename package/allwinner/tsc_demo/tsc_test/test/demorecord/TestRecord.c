#define LOG_NDEBUG 0
#define LOG_TAG "record_test"
#include <CDX_Debug.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "tsc.h"
#include "tsdemux.h"

#define  RECORD_BUFFER_SIZE		188*1024
typedef int int32_t;

typedef struct recorder_info
{
	uint8_t *buffer;
	int32_t buf_size;
	int32_t fd;
	int32_t pid;
} recorder_info;

int32_t request_buffer(void * arg, void *cookie)
{
    recorder_info *info = (recorder_info*)cookie;
    md_buf_t *pbuffer = ( md_buf_t*) arg;
    assert(pbuffer != NULL);
    assert(info != NULL);

	pbuffer->buf = info->buffer;
	pbuffer->buf_size = info->buf_size;
	return 0;
}

int32_t update_data(void *arg, void *cookie)
{
    recorder_info *info = (recorder_info*)cookie;
    md_data_info_t *data_info = ( md_data_info_t*) arg;
    //assert(pbuffer != NULL);
    assert(info != NULL);
    assert(data_info->data_len <= info->buf_size);
    write(info->fd, info->buffer, data_info->data_len);
    return 0;
}

int32_t open_recorder(void *demux_handle, recorder_info *recoder)
{
    char path[32];
    sprintf(&path[0], "/mnt/record-%04d.ts", recoder->pid);

    recoder->fd = open(&path[0], O_CREAT | O_TRUNC | O_RDWR, 0644);
    assert(recoder->fd >= 0);

    demux_recoder_param_t param;
    param.cookie = recoder;
    param.count = 1;
    param.request_buffer_cb = request_buffer;
    param.update_data_cb = update_data;
		param.pids[0] = 8192;//All PID

    int32_t ret = ts_demux_open_recorder(demux_handle, &param);
    if(ret != 0) {
	return -1;
    }
    return 0;
}

int32_t main(int argc, char *argv[])
{
    int32_t         err;
    int32_t         result;
	int32_t			ret;
    pthread_t       sectiontask;

	ALOGD("record test start");
    void *demux_handle = ts_demux_open(DEMUX_TYPE_LIVE_AND_RECORDE);
    ALOGV("demux handle %p", demux_handle);
    if(demux_handle == NULL)
    {
	return -1;
    }
    recorder_info recorder_info;
	memset(&recorder_info, 0, sizeof(recorder_info));
    recorder_info.buffer =(uint8_t *)malloc(RECORD_BUFFER_SIZE);
    assert( recorder_info.buffer != NULL);
    recorder_info.buf_size = RECORD_BUFFER_SIZE;
    recorder_info.pid = 8192;
	memset(recorder_info.buffer, 0, recorder_info.buf_size*sizeof(uint8_t));

    if(open_recorder(demux_handle, &recorder_info))
    {
	return -1;
    }

	int32_t test_exit = 0;
	while(!test_exit)
	{
		ALOGV("waiting for command:");
		char c = getchar();
		switch(c) {
		case 'Q':
		case 'q':
			test_exit = 1;
			break;

		default:
			break;
		}
		sleep(1);
	};

	ts_demux_close_recorder(demux_handle);
    if(recorder_info.buffer)
    {
        free(recorder_info.buffer);
    }
    ts_demux_close(demux_handle);
    ALOGV("exit main function");
    return 0;
}
