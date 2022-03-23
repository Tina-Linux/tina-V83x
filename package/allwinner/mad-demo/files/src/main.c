#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#include "formats.h"

#include <alsa/asoundlib.h>

/*#define DEBUG*/

#ifdef DEBUG
#define DLOG(fmt, arg...)		printf("[%s:%d] "fmt"", __FUNCTION__, __LINE__, ##arg)
#else
#define DLOG(fmt, arg...)
#endif

#define FORMAT_DEFAULT          -1
#define FORMAT_RAW              0
#define FORMAT_WAVE             1

#define PERIOD_FRAMES_DEFAULT	(1024)
#define BUFFER_FRAMES_DEFAULT	(PERIOD_FRAMES_DEFAULT*4)

static struct {
        snd_pcm_format_t format;
        unsigned int channels;
        unsigned int rate;
} g_hwparams;
static snd_pcm_t *g_handle;
static snd_output_t *g_log;
static snd_pcm_uframes_t g_period_frames = PERIOD_FRAMES_DEFAULT;
static snd_pcm_uframes_t g_buffer_frames = BUFFER_FRAMES_DEFAULT;

static unsigned int g_standby_count = 0;
static unsigned int g_pause_count = 0;
static unsigned int g_overrun_count = 0;
static int g_time_interval;
static int g_frame_interval;
static snd_pcm_uframes_t g_chunk_size;
static size_t g_chunk_bytes, g_bits_per_frame;
static int g_file_type = FORMAT_WAVE;
static off64_t g_fdcount;
static volatile sig_atomic_t g_in_aborting;
static volatile sig_atomic_t g_enter_mad;
static u_char *g_audiobuf = NULL;
static int g_pause_test = 0;
static int g_stop_delay = 0;

static void begin_wave(int fd, size_t count);
static void end_wave(int fd);
static const struct fmt_capture {
        void (*start) (int fd, size_t count);
        void (*end) (int fd);
        char *what;
        long long max_filesize;
} fmt_rec_table[] = {
        {       NULL,           NULL,           "raw data",         9223372036854775807LL},
        {       begin_wave,     end_wave,       "WAVE",             2147483648LL },
};

static void begin_wave(int fd, size_t cnt)
{
	WaveHeader h;
	WaveFmtBody f;
	WaveChunkHeader cf, cd;
	int bits;
	u_int tmp;
	u_short tmp2;

	/* WAVE cannot handle greater than 32bit (signed?) int */
	if (cnt == (size_t)-2)
		cnt = 0x7fffff00;

	bits = 8;
	switch ((unsigned long) g_hwparams.format) {
	case SND_PCM_FORMAT_U8:
		bits = 8;
		break;
	case SND_PCM_FORMAT_S16_LE:
		bits = 16;
		break;
	case SND_PCM_FORMAT_S32_LE:
        case SND_PCM_FORMAT_FLOAT_LE:
		bits = 32;
		break;
	case SND_PCM_FORMAT_S24_LE:
	case SND_PCM_FORMAT_S24_3LE:
		bits = 24;
		break;
	default:
		fprintf(stderr, "Wave doesn't support %s format...\n", snd_pcm_format_name(g_hwparams.format));
		exit(EXIT_FAILURE);
	}
	h.magic = WAV_RIFF;
	tmp = cnt + sizeof(WaveHeader) + sizeof(WaveChunkHeader) + sizeof(WaveFmtBody) + sizeof(WaveChunkHeader) - 8;
	h.length = LE_INT(tmp);
	h.type = WAV_WAVE;

	cf.type = WAV_FMT;
	cf.length = LE_INT(16);

        if (g_hwparams.format == SND_PCM_FORMAT_FLOAT_LE)
                f.format = LE_SHORT(WAV_FMT_IEEE_FLOAT);
        else
                f.format = LE_SHORT(WAV_FMT_PCM);
	f.channels = LE_SHORT(g_hwparams.channels);
	f.sample_fq = LE_INT(g_hwparams.rate);

	tmp2 = g_hwparams.channels * snd_pcm_format_physical_width(g_hwparams.format) / 8;
	f.byte_p_spl = LE_SHORT(tmp2);
	tmp = (u_int) tmp2 * g_hwparams.rate;

	f.byte_p_sec = LE_INT(tmp);
	f.bit_p_spl = LE_SHORT(bits);

	cd.type = WAV_DATA;
	cd.length = LE_INT(cnt);

	if (write(fd, &h, sizeof(WaveHeader)) != sizeof(WaveHeader) ||
	    write(fd, &cf, sizeof(WaveChunkHeader)) != sizeof(WaveChunkHeader) ||
	    write(fd, &f, sizeof(WaveFmtBody)) != sizeof(WaveFmtBody) ||
	    write(fd, &cd, sizeof(WaveChunkHeader)) != sizeof(WaveChunkHeader)) {
		fprintf(stderr, "write error\n");
		exit(EXIT_FAILURE);
	}
}

static void end_wave(int fd)
{                               /* only close output */
        WaveChunkHeader cd;
        off64_t length_seek;
        off64_t filelen;
        u_int rifflen;

        length_seek = sizeof(WaveHeader) +
                      sizeof(WaveChunkHeader) +
                      sizeof(WaveFmtBody);
        cd.type = WAV_DATA;
        cd.length = g_fdcount > 0x7fffffff ? LE_INT(0x7fffffff) : LE_INT(g_fdcount);
        filelen = g_fdcount + 2*sizeof(WaveChunkHeader) + sizeof(WaveFmtBody) + 4;
        rifflen = filelen > 0x7fffffff ? LE_INT(0x7fffffff) : LE_INT(filelen);
        if (lseek64(fd, 4, SEEK_SET) == 4)
                write(fd, &rifflen, 4);
        if (lseek64(fd, length_seek, SEEK_SET) == length_seek)
                write(fd, &cd, sizeof(WaveChunkHeader));
        if (fd != 1)
                close(fd);
}

int create_path(const char *path)
{
        char *start;
        mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

        if (path[0] == '/')
                start = strchr(path + 1, '/');
        else
                start = strchr(path, '/');

        while (start) {
                char *buffer = strdup(path);
                buffer[start-path] = 0x00;

                if (mkdir(buffer, mode) == -1 && errno != EEXIST) {
                        fprintf(stderr, "Problem creating directory %s", buffer);
                        perror(" ");
                        free(buffer);
                        return -1;
                }
                free(buffer);
                start = strchr(start + 1, '/');
        }
        return 0;
}

static int safe_open(const char *name)
{
        int fd;

        fd = open(name, O_WRONLY | O_CREAT, 0644);
        if (fd == -1) {
                if (errno != ENOENT)
                        return -1;
                if (create_path(name) == 0)
                        fd = open(name, O_WRONLY | O_CREAT, 0644);
        }
        return fd;
}

static void signal_handler(int sig)
{
	if (g_in_aborting)
		return ;
	g_in_aborting = 1;
	printf("Aborted by signal %s...\n", strsignal(sig));
	if (g_handle)
		snd_pcm_abort(g_handle);
}

static void signal_handler_enter_mad(int sig)
{
	g_enter_mad = !g_enter_mad;
}

static void TEMP_pcm_status()
{
	snd_pcm_status_t *status;
	snd_pcm_status_alloca(&status);
	DLOG("dealy %d \n", snd_pcm_status_get_delay(status));
	DLOG("avail %d frames in buffer\n", snd_pcm_status_get_avail(status));
}

static void xrun(void) {
	snd_pcm_status_t *status;
	int res;

	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(g_handle, status)) < 0) {
		fprintf(stderr, "snd_pcm_status failed. return %d", res);
		exit(-1);
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
#if 1
		struct timeval now, diff, tstamp;

		DLOG("avail %d bytes in buffer\n", snd_pcm_status_get_avail(status));
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		g_overrun_count++;
		printf(" %s!!  (at least %.3f ms long)\n", "overrun",
						diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
#endif
		if ((res = snd_pcm_prepare(g_handle)) < 0) {
			fprintf(stderr, " prepare error:%d\n", res);
			int count = 10;
			while(count>0) {
				count--;
				usleep(500000);
				res = snd_pcm_prepare(g_handle);
				if (res == -EBUSY) {
					DLOG("still busy\n");
				}
				if (res == 0)
					break;
			}
			/*exit(-1);*/
		}
		return;
	} else if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
		fprintf(stderr, "snd_pcm_status_get_state = SND_PCM_STATE_DRAINING !!!");
	}
	fprintf(stderr, "FAILED!!! write error.");
	exit(-1);
}

static ssize_t pcm_read(u_char *data, size_t rcount)
{
	ssize_t r;
	size_t count = rcount;
	/*DLOG("count=%u, g_chunk_size=%u\n", count, g_chunk_size);*/
	if (count != g_chunk_size)
		count = g_chunk_size;

	while (count > 0 && !g_in_aborting) {
		/*TEMP_pcm_status();*/
		r = snd_pcm_readi(g_handle, data, count);
		if (r != count)
			DLOG("snd_pcm_read return %u...\n", r);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
#if 0
			DLOG("snd_pcm_wait...\n");
			snd_pcm_wait(g_handle, 10);
#else
			DLOG("snd_pcm_read only %d frames, total need %u frames.\n", r, count);
#endif
		} else if (r == -EPIPE) {
			xrun();
			continue;
		} else if (r == -ESTRPIPE) {
			printf("snd_pcm_resume...\n");
			while(snd_pcm_resume(g_handle) == -EAGAIN)
				sleep(1);
		} else if (r < 0) {
			printf("read error..:%s\n", snd_strerror(r));
			exit(-1);
		}
		if (r > 0) {
			/*result += r;*/
			count -= r;
			data += r * g_bits_per_frame / 8;
		}
	}

	return rcount;
}

static int file_read(const char *path, char *str, unsigned int *len)
{
	int fd;
	unsigned int size;

	if (!len)
		return -1;
	size = *len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	*len = read(fd, str, size);
	close(fd);
	return 0;
}

static int file_write(const char *path, char *str)
{
	int fd;

	fd = open(path, O_RDWR);
	if (fd < 0)
		return -1;
	write(fd, str, strlen(str));
	close(fd);
	return 0;
}

static bool lpsd_status_is_idle(void)
{
	char buf[16];
	unsigned int len = sizeof(buf);

	memset(buf, 0, sizeof(buf));
	file_read("/sys/devices/platform/soc/mad/sunxi_mad_audio/lpsd_status",
		buf, &len);
	printf("lpsd_status:%s", buf);
	if (atoi(buf) == 0)
		return true;

	return false;
}

static void mad_standby(void)
{
	char buf[16];
	unsigned int len = sizeof(buf);

	file_read("/sys/power/wakeup_count", buf, &len);
	file_write("/sys/power/wakeup_count", buf);

	g_standby_count++;
	printf("#######################\n");
	printf("stanby_count = %u\n", g_standby_count);
	printf("overrun_count = %u\n", g_overrun_count);
	printf("#######################\n");
	file_write("/sys/power/state", "mem");
}

static void capture(char *orig_name)
{
	int fd;
	off64_t rest;
	struct stat statbuf;
	snd_pcm_uframes_t frame_loop = 0;

	if (orig_name != NULL)
		printf("record file name:%s\n", orig_name);
	else {
		orig_name = "/dev/null";
	}

	/* open a new file */
	if (!lstat(orig_name, &statbuf)) {
		if (S_ISREG(statbuf.st_mode))
			remove(orig_name);
	}

	fd = safe_open(orig_name);
	if (fd < 0) {
		perror(orig_name);
		exit(-1);
	}

	rest = fmt_rec_table[g_file_type].max_filesize;
	if (fmt_rec_table[g_file_type].start)
		fmt_rec_table[g_file_type].start(fd, rest);

	g_fdcount = 0;


	while (rest > 0 && !g_in_aborting) {
		size_t c =  (rest <= (off64_t)g_chunk_bytes) ?
			(size_t)rest : g_chunk_bytes;
		size_t f = c * 8 / g_bits_per_frame;

		if (snd_pcm_state(g_handle) != SND_PCM_STATE_RUNNING)
			DLOG("before pcm_read, state:%d\n", snd_pcm_state(g_handle));
		memset(g_audiobuf, 0, g_chunk_bytes);
		if (pcm_read(g_audiobuf, f) != f)
			break;
		if (write(fd, g_audiobuf, c) !=c) {
			perror(orig_name);
			exit(-1);
		}
		fsync(fd);
		rest -= c;
		g_fdcount += c;
		frame_loop += f;
		/* detect enter mad mode or not */
		if ((g_frame_interval/frame_loop == 0 || g_enter_mad != 0)
			&& lpsd_status_is_idle()) {
			int ret;
			DLOG("state:%d\n", snd_pcm_state(g_handle));
#if 1
			ret = snd_pcm_pause(g_handle, 1);
			if (ret < 0) {
				fprintf(stderr, "pause push error: %s\n", snd_strerror(ret));
				exit(-1);
			}
			DLOG("state:%d\n", snd_pcm_state(g_handle));
#endif
	#if 0
			ret = snd_pcm_prepare(g_handle);
			DLOG("state:%u\n", snd_pcm_state(g_handle));
	#endif
			printf("enter mad mode!\n");

			if (g_pause_test) {
				g_pause_count++;
				printf("#######################\n");
				printf("pause_count = %u\n", g_pause_count);
				printf("overrun_count = %u\n", g_overrun_count);
				printf("#######################\n");
				sleep(g_pause_test);
			} else {
				mad_standby();
				/*usleep(100000);*/
			}
			#if 1
			ret = snd_pcm_pause(g_handle, 0);
			if (ret < 0) {
				fprintf(stderr, "pause release error: %s\n", snd_strerror(ret));
				exit(-1);
			}
			#endif
			g_enter_mad = 0;
			frame_loop = 0;
			printf("exit mad mode!\n");
			DLOG("state:%d\n", snd_pcm_state(g_handle));
			if (g_in_aborting)
				break;
		}
	}

	if (fmt_rec_table[g_file_type].end)
		fmt_rec_table[g_file_type].end(fd);

	return ;
}

static void set_params(void)
{
	int ret;
	snd_pcm_hw_params_t *params;
	size_t bits_per_sample;
	snd_pcm_uframes_t stop_threshold;

	snd_pcm_hw_params_alloca(&params);
	ret = snd_pcm_hw_params_any(g_handle, params);
	if (ret < 0) {
		printf("cannot open audio device.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_access(g_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		printf("access type not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_format(g_handle, params, g_hwparams.format);
	if (ret < 0) {
		printf("format not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_channels(g_handle, params, g_hwparams.channels);
	if (ret < 0) {
		printf("channels count not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_rate_near(g_handle, params, &g_hwparams.rate, 0);
	if (ret < 0) {
		printf("rate not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_period_size_near(g_handle, params, &g_period_frames, 0);
	if (ret < 0) {
		printf("period frames not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params_set_buffer_size_near(g_handle, params, &g_buffer_frames);
	if (ret < 0) {
		printf("buffer frames not available.\n");
		exit(-1);
	}
	ret = snd_pcm_hw_params(g_handle, params);
	if (ret < 0) {
		printf("Unable to install hw params:\n");
		snd_pcm_hw_params_dump(params, g_log);
		exit(-1);
	}
#ifdef DEBUG
	snd_pcm_dump(g_handle, g_log);
#endif

	if (g_chunk_size == 0)
		snd_pcm_hw_params_get_period_size(params, &g_chunk_size, 0);

	bits_per_sample = snd_pcm_format_physical_width(g_hwparams.format);
	g_bits_per_frame = bits_per_sample * g_hwparams.channels;
	g_chunk_bytes = g_chunk_size * g_bits_per_frame / 8;
	g_audiobuf = malloc(g_chunk_bytes);

	/*g_frame_interval = time_to_frame(g_time_interval);*/
	g_frame_interval = g_hwparams.rate * g_time_interval;

	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);
	snd_pcm_sw_params_current(g_handle, swparams);
	snd_pcm_sw_params_set_tstamp_mode(g_handle, swparams, SND_PCM_TSTAMP_ENABLE);
	snd_pcm_sw_params_set_tstamp_type(g_handle, swparams, SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY);
	if (!g_stop_delay) {
		if (g_period_frames < 1024) {
			/* set stop threshold 500ms delay */
			stop_threshold = g_hwparams.rate/2;
		} else {
			/* set stop threshold 2*buffer_size */
			stop_threshold =g_buffer_frames*2;
		}
	} else {
		stop_threshold = (double) g_hwparams.rate * g_stop_delay / 1000000;
	}
	snd_pcm_sw_params_set_stop_threshold(g_handle, swparams, stop_threshold);
	snd_pcm_sw_params(g_handle, swparams);

}

void usage()
{
	printf("Usage:\n"
		"\n"
		"-h, --help             help\n"
		"-D, --device           select PCM by name\n"
		"-f, --format           sample format\n"
		"-r, --rate             sample rate\n"
		"-d, --duration         enter standby after # seconds\n"
		"-c, --channels         channels\n"
		"-p, --period-size      period size\n"
		"-b, --buffer-size      buffer size\n"
		"-l, --loop-frames      pcm_read size each time\n"
		"-t, --pause-test       pause test\n"
		"-T, --stop-delay       delay for automatic PCM stop is # microseconds from xrun\n"
		"\n"
		);
}

int main(int argc, char *argv[])
{
	const struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"device", required_argument, NULL, 'D'},
		{"format", required_argument, NULL, 'f'},
		{"rate", required_argument, NULL, 'r'},
		{"duration", required_argument, NULL, 'd'},
		{"channels", required_argument, NULL, 'c'},
		{"period-size", required_argument, NULL, 'p'},
		{"buffer-size", required_argument, NULL, 'b'},
		{"loop-frames", required_argument, NULL, 'l'},
		{"pause-test", required_argument, NULL, 't'},
		{"stop-delay", required_argument, NULL, 'T'},
	};
	int opt;
	int ret;
	char *pcm_name = "default";

	if (argc == 1) {
		usage();
		return 0;
	}

	g_hwparams.format = SND_PCM_FORMAT_S16;
	g_hwparams.rate = 8000;
	g_hwparams.channels = 2;

	while ((opt = getopt_long(argc, argv, "hD:f:r:d:c:p:b:l:t:T:", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'h':
			usage();
			return 0;
		case 'D':
			pcm_name = optarg;
			break;
		case 'f':
			g_hwparams.format = snd_pcm_format_value(optarg);
			if (g_hwparams.format == SND_PCM_FORMAT_UNKNOWN) {
				printf("wrong format...\n");
				return -1;
			}
			break;
		case 'r':
			g_hwparams.rate = strtol(optarg, NULL, 0);
			if (g_hwparams.rate < 8000 || g_hwparams.rate > 192000) {
				printf("wrong rate...\n");
				return -1;
			}
			break;
		case 'd':
			g_time_interval = strtol(optarg, NULL, 0);
			break;
		case 'c':
			g_hwparams.channels = strtol(optarg, NULL, 0);
			break;
		case 'p':
			g_period_frames = strtol(optarg, NULL, 0);
			break;
		case 'b':
			g_buffer_frames = strtol(optarg, NULL, 0);
			break;
		case 'l':
			g_chunk_size = strtol(optarg, NULL, 0);
			break;
		case 't':
			g_pause_test = strtol(optarg, NULL, 0);
			break;
		case 'T':
			g_stop_delay = strtol(optarg, NULL, 0);
			break;
		default:
			printf("unknown command:%c\n", opt);
			return -1;
		}
	}

	ret = snd_output_stdio_attach(&g_log, stderr, 0);
	assert(ret >= 0);

	ret = snd_pcm_open(&g_handle, pcm_name, SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0) {
		printf("cannot open audio device.\n");
		return -1;
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGUSR1, signal_handler_enter_mad);

	set_params();

	if (optind > argc - 1)
		capture(NULL);
	else
		capture(argv[optind++]);

	printf("%s exit.\n", argv[0]);
	if (g_audiobuf != NULL)
		free(g_audiobuf);
	snd_pcm_close(g_handle);
	snd_output_close(g_log);
	return 0;
}
