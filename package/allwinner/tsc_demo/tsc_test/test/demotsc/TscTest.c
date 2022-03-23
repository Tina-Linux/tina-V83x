#define LOG_NDEBUG 0
#define LOG_TAG "tsc_test"
#include <CDX_Debug.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <tsc.h>

typedef int int32_t;


#define VIDEO_PID       8192

typedef struct filter
{
	int32_t pid;
	int32_t fd;
	int32_t chan;
	void * tsc_handle;
	pthread_t thread_id;
	int32_t thread_exit;
}filter_t;


static void * data_receive_thread(void* arg)
{
	tsf_data_t tsf_data;

	filter_t* filter = (filter_t *) arg;
	ts_dev_t* tsc_handle = (ts_dev_t *)filter->tsc_handle;

	chan_register_t chan_register;
	chan_register.pid = filter->pid;
	chan_register.chan_type = CHAN_TYPE_LIVE;
	chan_register.stream_type = TS_STREAM_VIDEO;

	filter->chan = tsc_handle->open_channel(tsc_handle->cookie, &chan_register);
	if (filter->chan < 0)
	{
		ALOGE("open tsc pid channel fail, pid = %d.", filter->pid);
		return NULL;
	}

	while (!filter->thread_exit)
	{
		//get data buffer pointer
		int32_t result = tsc_handle->request_channel_data(tsc_handle->cookie, filter->chan, &tsf_data);
		if (result != 0)
			break;

		printf("tsf_data.pkt_num: %d\n", tsf_data.pkt_num);
		//handle packets
		if (tsf_data.pkt_num)
		{
			write(filter->fd, tsf_data.data, tsf_data.pkt_num * 188);
		}

		printf("tsf_data.ring_pkt_num: %d\n", tsf_data.pkt_num);

		if (tsf_data.ring_pkt_num)
		{
			write(filter->fd, tsf_data.ring_data, tsf_data.ring_pkt_num * 188);
		}

		tsc_handle->flush_channel_data(tsc_handle->cookie, filter->chan,
				tsf_data.pkt_num + tsf_data.ring_pkt_num);
		usleep(10 * 1000);
	}

	tsc_handle->close_channel(tsc_handle->cookie, filter->chan);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, void** argv)
{
    printf(" argc = %d \n",argc);
    if(argc != 2){
        printf("the argc is wrong,please input like this:\n");
        printf("./tscTest /mnt/UDISK/save_ts_stream.ts\n");
        return 0;
    }
	ts_dev_t * tsc = (ts_dev_t*)tsc_dev_open();
    if(tsc == NULL){
        printf("tsc is NULL\n");
    }
	assert(tsc != NULL);

	filter_t video_filter;
    printf("save the ts stream in this path : %s\n",argv[1]);
	video_filter.fd = open(argv[1], O_CREAT|O_RDWR, 0644);
	printf("video_filter.fd: %d\n",video_filter.fd);
    if(video_filter.fd == -1){
	    printf("video_filter.fd: %d, errno: %d,err_str=%s\n",  video_filter.fd, errno,strerror(errno));
    }
	assert(video_filter.fd >= 0);

	video_filter.pid = VIDEO_PID;
	video_filter.thread_exit = 0;
	video_filter.tsc_handle = tsc;

	int err = pthread_create(&video_filter.thread_id, NULL, data_receive_thread,
			(void*)&video_filter);
	assert(err == 0);

	int test_exit = 0;
	while (!test_exit)
	{
		ALOGV("waiting for command:");
		char c = getchar();
		ALOGV("get command %c", c);
		switch (c)
		{
			case 'Q':
			case 'q':
				test_exit = 1;
				//set exit flag for all thread
				video_filter.thread_exit = 1;
				break;

			default:
				break;
		}
		sleep(1);
	};

	if (video_filter.thread_id > 0)
	{
		err = pthread_join(video_filter.thread_id, NULL);
	}
	close(video_filter.fd);
	tsc_dev_close(tsc);
	ALOGV("exit main function");
	return 0;
}
