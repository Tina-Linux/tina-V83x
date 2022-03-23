#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include "recorder_int.h"
#include "rec_list.h"

static unsigned char collide_f00 = 0; /* Record whether the previous file needs to be saved */
static unsigned char collide_f01 = 0; /* Record whether the current file needs to be saved */
static unsigned char collide_f02 = 0; /* Record whether the next file needs to be saved */

static unsigned char collide_f10 = 0; /* Record whether the previous file needs to be saved */
static unsigned char collide_f11 = 0; /* Record whether the current file needs to be saved */
static unsigned char collide_f12 = 0; /* Record whether the next file needs to be saved */

static unsigned char cur_collide_f0 = 0; /* Record whether the file currently being recorded is a saved file */
static unsigned char cur_collide_f1 = 0; /* Record whether the file currently being recorded is a saved file */

static unsigned char pre_collide_f0 = 0;
static unsigned char pre_collide_f1 = 0;

static unsigned char first_file_f0 = 0;
static unsigned char first_file_f1 = 0;

#define _GNU_SOURCE
#include <fcntl.h>
#define FALLOC_FL_KEEP_SIZE	0x01 /* default is extend size */

#define RESERVED_SPACE (300*1024*1024)
#define REC_LIST_LEN_MAX 150

#define	WM_POS_X_F		(0)
#define	WM_POS_Y_F		(0)

static struct timeval time0; /* Record the first video recording time */
static struct timeval time1; /* Record the second video recording time */
static list_node_t *rec_list_A = NULL;
static list_node_t *rec_list_B = NULL;
static __dv_core_t *dv_core;
static char *file_name[5] = { ".ts", ".mov", ".jpg", ".aac", ".mp3" };
static char PARTH_A[64];
static char PARTH_B[64];

/* By name order */
int add_name_to_list(list_node_t * list, char *name) {
	list_node_t *tmp_list = NULL;
	list_node_t *tmp_list1 = list;
	list_node_t *new_node = NULL;
	tmp_list = list->next;
	while (tmp_list) {
		if (strcmp(tmp_list->filename, name) > 0) {
			break;
		}
		tmp_list1 = tmp_list;
		tmp_list = tmp_list->next;
	}
	new_node = (list_node_t *) malloc(sizeof(list_node_t));
	if (NULL == new_node)
		return -1;
	memset(new_node, 0, sizeof(list_node_t));
	strcpy(new_node->filename, name);
	printf("%s\n", name);

	tmp_list1->next = new_node;
	new_node->next = tmp_list;
	return 0;
}

/* Core: fallocate right after creating the file */
int CreateMyFile(char * szFileName, int nFileLength) {
	int fd;
	fd = open(szFileName, O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		close(fd);
		return -1;
	}
	printf("fallocate start!\n");
	if (fallocate(fd, FALLOC_FL_KEEP_SIZE, (off_t) 0, (off_t) nFileLength)) {
		fprintf(stderr, "%s\n", strerror(errno));
		close(fd);
		return -2;
	}
	fsync(fd);
	close(fd);
	return 0;
}

/* Create a file, select the file name and maintain the linked list postfix suffix */
int create_new_file(int index, long size, list_node_t *list, char *postfix,
		char *compete_path) {
	time_t timep;
	struct tm *p;
	time(&timep);
	char name[32];
	p = localtime(&timep); /*Get local time*/
	/* printf("%d-%02d-%02d ", (1900+p->tm_year),(1 + p->tm_mon), p->tm_mday); */
	/* printf("%d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec); */
	memset(name, 0, sizeof(name));
	if (index == 0) {
		sprintf(name, "AW_%d%02d%02d_%02d%02d%02dA%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_A, name);
	} else {
		sprintf(name, "AW_%d%02d%02d_%02d%02d%02dB%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_B, name);
	}
	/* printf("%s\n", compete_path); */
	/* create_file */
	if ((CreateMyFile(compete_path, size)) < 0) {
		return -1;
	}
	list_add(list, name);
	return 0;
}

int rename_new_file(int index, long size, list_node_t *list, char *postfix,
		char *compete_path) {
	list_node_t *new_node;
	char name[32];
	char old_compete_path[64];
	time_t timep;
	int fsize1 = 0;
	int fsize2 = 0;
	struct tm *p;
	struct stat sta;
	time(&timep);
	p = localtime(&timep); /*Get local time*/
	/* printf("%d-%02d-%02d ", (1900+p->tm_year),(1 + p->tm_mon), p->tm_mday); */
	/* printf("%d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec); */
	memset(name, 0, sizeof(name));
	memset(old_compete_path, 0, sizeof(old_compete_path));
	new_node = list_del(list);
	if (!new_node)
		return -1;
	if (index == 0) {
		sprintf(old_compete_path, "%s/%s", PARTH_A, new_node->filename);
		sprintf(name, "AW_%d%02d%02d_%02d%02d%02dA%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_A, name);

	} else {
		sprintf(old_compete_path, "%s/%s", PARTH_B, new_node->filename);
		sprintf(name, "AW_%d%02d%02d_%02d%02d%02dB%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_B, name);
	}
	free(new_node);

	stat(old_compete_path, &sta);
	/* printf("st_blksize = %d, st_blocks = %d\n", sta.st_blksize, sta.st_blocks); */
	fsize1 = 512 * sta.st_blocks;

	/* printf("size = %ld\n", size); */
	/* printf("fsize1 = %d\n", fsize1); */
	if (fsize1 >= size) {
		if ((rename(old_compete_path, compete_path)) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			return -1;
		}
		printf("truncate64 function start!/n");
		if (truncate64(compete_path, 1)) {
			fprintf(stderr, "%s\n", strerror(errno));

			return -1;
		}
		list_add(list, name);
	} else {
		fsize2 += fsize1;
		remove(old_compete_path);

		while (fsize2 < size) {
			new_node = list_del(list);
			if (!new_node)
				return -1;
			if (index == 0) {
				sprintf(old_compete_path, "%s/%s", PARTH_A, new_node->filename);
				/* sprintf(name, "AW_%d%02d%02d_%02d%02d%02dA%s", (1900 + p->tm_year), \ */
				/* 	(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, postfix); */
				/* sprintf(compete_path, "%s/%s", PARTH_A, name); */

			} else {
				sprintf(old_compete_path, "%s/%s", PARTH_B, new_node->filename);
				/* sprintf(name, "AW_%d%02d%02d_%02d%02d%02dB%s", (1900 + p->tm_year), \ */
				/* 	(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, postfix); */
				/* sprintf(compete_path, "%s/%s", PARTH_B, name); */
			}
			free(new_node);
			stat(old_compete_path, &sta);
			fsize1 = sta.st_blksize * sta.st_blocks;
			fsize2 += fsize1;
			remove(old_compete_path);
		}

		create_new_file(index, size, list, postfix, compete_path);

	}

	return 0;
}

int create_file(char *compete_path, int theth, int size, list_node_t *list,
		int index, char *postfix) {
	if (compete_path == NULL || list == NULL) {
		return -1;
	}
	printf("size = %d\n", size);
	/* If there is enough free space */
	if (theth) {
		create_new_file(index, size, list, postfix, compete_path);
	}
	/* If there is not enough free space */
	else {
		rename_new_file(index, size, list, postfix, compete_path);
	}
	return 0;
}

int create_new_file_no_list(int index, long size, char *postfix,
		char *compete_path) {
	time_t timep;
	struct tm *p;
	time(&timep);
	char name[32];
	p = localtime(&timep); /*Get local time*/
	printf("%d-%02d-%02d ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
	printf("%d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec);
	memset(name, 0, sizeof(name));
	if (index == 0) {
		sprintf(name, "ZW_%d%02d%02d_%02d%02d%02dA%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_A, name);
	} else {
		sprintf(name, "ZW_%d%02d%02d_%02d%02d%02dB%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_B, name);
	}
	printf("%s\n", compete_path);
	/* create_file */
	if ((CreateMyFile(compete_path, size)) < 0) {
		return -1;
	}
	return 0;
}

int rename_new_file_no_list(int index, long size, list_node_t *list,
		char *postfix, char *compete_path) {
	list_node_t *new_node;
	char name[32];
	char old_compete_path[64];
	time_t timep;
	int fsize1 = 0;
	int fsize2 = 0;
	struct tm *p;
	struct stat sta;
	time(&timep);
	p = localtime(&timep); /*Get local time*/
	printf("%d-%02d-%02d ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
	printf("%d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec);
	memset(name, 0, sizeof(name));
	memset(old_compete_path, 0, sizeof(old_compete_path));
	new_node = list_del(list);
	if (!new_node)
		return -1;
	if (index == 0) {
		sprintf(old_compete_path, "%s/%s", PARTH_A, new_node->filename);
		sprintf(name, "ZW_%d%02d%02d_%02d%02d%02dA%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_A, name);

	} else {
		sprintf(old_compete_path, "%s/%s", PARTH_B, new_node->filename);
		sprintf(name, "ZW_%d%02d%02d_%02d%02d%02dB%s", (1900 + p->tm_year),
				(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
				postfix);
		sprintf(compete_path, "%s/%s", PARTH_B, name);
	}
	free(new_node);

	stat(old_compete_path, &sta);
	/* printf("st_blksize = %d, st_blocks = %d\n", sta.st_blksize, sta.st_blocks); */
	fsize1 = 512 * sta.st_blocks;

	/* printf("size = %ld\n", size); */
	/* printf("fsize1 = %d\n", fsize1); */
	if (fsize1 >= size) {
		if ((rename(old_compete_path, compete_path)) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			return -1;
		}
		printf("truncate64 function start!/n");
		if (truncate64(compete_path, 1)) {
			fprintf(stderr, "%s\n", strerror(errno));

			return -1;
		}
	} else {
		fsize2 += fsize1;
		remove(old_compete_path);

		while (fsize2 < size) {
			new_node = list_del(list);
			if (!new_node)
				return -1;
			if (index == 0) {
				sprintf(old_compete_path, "%s/%s", PARTH_A, new_node->filename);
			} else {
				sprintf(old_compete_path, "%s/%s", PARTH_B, new_node->filename);
			}
			free(new_node);
			stat(old_compete_path, &sta);
			fsize1 = sta.st_blksize * sta.st_blocks;
			fsize2 += fsize1;
			remove(old_compete_path);
		}

		create_new_file_no_list(index, size, postfix, compete_path);

	}

	return 0;
}

int create_file_no_list(char *compete_path, int theth, list_node_t *list,
		int size, int index, char *postfix) {
	if (compete_path == NULL) {
		return -1;
	}
	/* If there is enough free space */
	if (theth) {
		create_new_file_no_list(index, size, postfix, compete_path);
	}
	/* If there is not enough free space */
	else {
		rename_new_file_no_list(index, size, list, postfix, compete_path);
	}
	return 0;
}

int rename_pre_file(list_node_t *head, int index) {
	int i = 0;
	list_node_t *tmp_node;
	char compete_path[64];
	char old_compete_path[64];

	i = list_get_total(head);

	if (i < 2) {
		return -1;
	}

	i -= 1;

	tmp_node = list_del_index(head, i);

	memset(compete_path, 0, sizeof(compete_path));

	if (index == 0) {
		sprintf(old_compete_path, "%s/%s", PARTH_A, tmp_node->filename);
		tmp_node->filename[0] = 'Z';
		sprintf(compete_path, "%s/%s", PARTH_A, tmp_node->filename);
	} else {
		sprintf(old_compete_path, "%s/%s", PARTH_B, tmp_node->filename);
		tmp_node->filename[0] = 'Z';
		sprintf(compete_path, "%s/%s", PARTH_B, tmp_node->filename);
	}
	if ((rename(old_compete_path, compete_path)) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	free(tmp_node);
	return 0;
}

int rename_cur_file(list_node_t *head, int index) {
	int i = 0;
	list_node_t *tmp_node;
	char compete_path[64];
	char old_compete_path[64];

	i = list_get_total(head);

	if (i < 1) {
		return -1;
	}

	tmp_node = list_del_index(head, i);

	memset(compete_path, 0, sizeof(compete_path));

	if (index == 0) {
		sprintf(old_compete_path, "%s/%s", PARTH_A, tmp_node->filename);
		tmp_node->filename[0] = 'Z';
		sprintf(compete_path, "%s/%s", PARTH_A, tmp_node->filename);
	} else {
		sprintf(old_compete_path, "%s/%s", PARTH_B, tmp_node->filename);
		tmp_node->filename[0] = 'Z';
		sprintf(compete_path, "%s/%s", PARTH_B, tmp_node->filename);
	}
	if ((rename(old_compete_path, compete_path)) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	free(tmp_node);
	return 0;
}

/* Judge whether it is a special directory */
static int is_special_dir(const char *path) {
	return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

list_node_t * create_list_by_path(char *path) {
	DIR *dir;
	int status;
	list_node_t *list;
	struct dirent *dir_info;
	if ((dir = opendir(path)) == NULL) {
		printf("%s is not exist.\n", path);
		status = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
		if (status < 0) {
			printf("mkdir failed %s.\n", path);
			return NULL;
		}
		list = create_list();
		return list;
	}
	list = create_list();
	while ((dir_info = readdir(dir)) != NULL) {
		/*  Ignore the file. */
		if (is_special_dir(dir_info->d_name))
			continue;
		/*  Manage existing files based on file name and insert existing files into linked list */
		/*  File Name Structure: aw_20170101_905023A.mp4 / ts */
		if (strcmp(
				dir_info->d_name + strlen(dir_info->d_name)
						- strlen(file_name[dv_core->mToutputFormat[0]]),
				file_name[dv_core->mToutputFormat[0]]) == 0
				&& strncmp(dir_info->d_name, "AW_", 3) == 0) {
			add_name_to_list(list, dir_info->d_name);
		}
	}
	closedir(dir);            /* Close the directory */
	return list;
}

int CallbackFromTRecorder0(void* pUserData, int msg, void* param) {
	__dv_core_t *dv_core = (__dv_core_t *) pUserData;
	char parth[128];
	int theth = 1;
	int f_size = 0;
	memset(parth, 0, sizeof(parth));
	switch (msg) {
	case T_RECORD_ONE_FILE_COMPLETE:
		printf("T_RECORD_ONE_FILE_COMPLETE 0\n");
		memset(parth, 0, sizeof(parth));
		f_size = ((dv_core->mTencoderBitRate[0] + 1000000) / 1000 / 1000) * 1024
				* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[0] / 1000);
		if (get_disk_free(2)
				< ((RESERVED_SPACE + f_size) >> 10)|| list_get_total(rec_list_A) >= REC_LIST_LEN_MAX) {
			theth = 0;
		} else {
			theth = 1;
		}
		pthread_mutex_lock(&dv_core->mutex0[0]);
		if (first_file_f0 < 2)
			first_file_f0++;
		/* Collision save */
		if (collide_f00 == 1) {
			rename_pre_file(rec_list_A, 0);
			collide_f00 = 0;
		}
		if (collide_f01 == 1) {
			rename_cur_file(rec_list_A, 0);
			collide_f01 = 0;
		}
		if (collide_f02 == 1) {
			create_file_no_list(parth, theth, rec_list_A, f_size, 0,
					file_name[dv_core->mToutputFormat[0]]);
			collide_f02 = 0;
			pre_collide_f0 = cur_collide_f0;
			cur_collide_f0 = 1;
		} else {
			create_file(parth, theth, f_size, rec_list_A, 0,
					file_name[dv_core->mToutputFormat[0]]);
			pre_collide_f0 = cur_collide_f0;
			cur_collide_f0 = 0;
		}
		pthread_mutex_unlock(&dv_core->mutex0[0]);
		TRchangeOutputPath(dv_core->mTrecorder[0], parth);
		break;
	default:
		printf("warning: unknown callback from trecorder\n");
		break;
	}
	return 0;
}

int CallbackFromTRecorder1(void* pUserData, int msg, void* param) {
	__dv_core_t *dv_core = (__dv_core_t *) pUserData;
	char parth[128];
	int theth = 1;
	int f_size = 0;
	memset(parth, 0, sizeof(parth));
	switch (msg) {
	case T_RECORD_ONE_FILE_COMPLETE:
		printf("T_RECORD_ONE_FILE_COMPLETE 1\n");
		memset(parth, 0, sizeof(parth));
		f_size = ((dv_core->mTencoderBitRate[1] + 1000000) / 1000 / 1000) * 1024
				* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[1] / 1000);
		if (get_disk_free(2)
				< ((RESERVED_SPACE + f_size) >> 10)|| list_get_total(rec_list_B) >= REC_LIST_LEN_MAX) {
			theth = 0;
		} else {
			theth = 1;
		}
		pthread_mutex_lock(&dv_core->mutex0[1]);
		if (first_file_f1 < 2)
			first_file_f1++;
		/* Collision save */
		if (collide_f10 == 1) {
			rename_pre_file(rec_list_B, 1);
			collide_f10 = 0;
		}
		if (collide_f11 == 1) {
			rename_cur_file(rec_list_B, 1);
			collide_f11 = 0;
		}
		if (collide_f12 == 1) {
			create_file_no_list(parth, theth, rec_list_B, f_size, 1,
					file_name[dv_core->mToutputFormat[0]]);
			collide_f12 = 0;
			pre_collide_f1 = cur_collide_f1;
			cur_collide_f1 = 1;
		} else {
			create_file(parth, theth, f_size, rec_list_B, 1,
					file_name[dv_core->mToutputFormat[0]]);
			pre_collide_f1 = cur_collide_f1;
			cur_collide_f1 = 0;
		}
		pthread_mutex_unlock(&dv_core->mutex0[1]);
		TRchangeOutputPath(dv_core->mTrecorder[1], parth);
		break;
	default:
		printf("warning: unknown callback from trecorder\n");
		break;
	}
	return 0;
}

/**
 * Initialize the camera / microphone
 *
 * @index 		Camera / Microphone 0 front 1 rear only use the front
 * @jusAudio	0 Voice recording 1 voice recording
 * @return		0 success -1 failed
 */
int recorder_init(int index, int justAudio) {
	int ret = 0;
	if (dv_core->recordSta[index] != RECORD_UNINIT) {
		return -1;
	}
	dv_core->mTrecorder[index] = CreateTRecorder();
	if (dv_core->mTrecorder[index] == NULL) {
		sm_error("CreateTRecorder err0\n");
		return -1;
	}
	ret = TRreset(dv_core->mTrecorder[index]);

	/* Set recording data */
	if (justAudio == 1) {
		dv_core->mToutputFormat[index] = T_OUTPUT_MP3;
		dv_core->mTaudioEncodeFormat[index] = T_AUDIO_AAC;
		dv_core->mTcameraIndex[index] = T_CAMERA_DISABLE;
		dv_core->mTdispIndex[index] = T_DISP_LAYER_DISABLE;
	}

	ret = TRsetCamera(dv_core->mTrecorder[index],
			dv_core->mTcameraIndex[index]);
	ret = TRsetAudioSrc(dv_core->mTrecorder[index], dv_core->mTmicIndex[index]);
	ret = TRsetPreview(dv_core->mTrecorder[index], dv_core->mTdispIndex[index]);

	if (justAudio == 1) {
		if (index == 0) {
			ret = TRsetOutput(dv_core->mTrecorder[index],
					"/mnt/SDCARD/recorder_save0.mp3");
		} else if (index == 1) {
			ret = TRsetOutput(dv_core->mTrecorder[index],
					"/mnt/SDCARD/recorder_save1.mp3");
		}
	} else {
		if (index == 0) {
			ret = TRsetOutput(dv_core->mTrecorder[index],
					"/mnt/SDCARD/recorder_save0.mp4");
		} else if (index == 1) {
			ret = TRsetOutput(dv_core->mTrecorder[index],
					"/mnt/SDCARD/recorder_save1.mp4");
		}
	}

	ret = TRsetCameraEnableWM(dv_core->mTrecorder[index], WM_POS_X_F,
	WM_POS_Y_F, dv_core->mTcameraEnableWM[index]);

	ret = TRsetOutputFormat(dv_core->mTrecorder[index],
			dv_core->mToutputFormat[index]);
	ret = TRsetVideoEncoderFormat(dv_core->mTrecorder[index],
			dv_core->mTvideoEncodeFormat[index]);
	ret = TRsetAudioEncoderFormat(dv_core->mTrecorder[index],
			dv_core->mTaudioEncodeFormat[index]);

	ret = TRsetMaxRecordTimeMs(dv_core->mTrecorder[index],
			dv_core->mTmaxRecordTimeMs[index]);
	ret = TRsetRecorderEnable(dv_core->mTrecorder[index], 1);
	ret = TRsetEncoderBitRate(dv_core->mTrecorder[index],
			dv_core->mTencoderBitRate[index]);
	ret = TRsetEncodeFramerate(dv_core->mTrecorder[index],
			dv_core->mTencodeFramerate[index]);
	ret = TRsetVideoEncodeSize(dv_core->mTrecorder[index],
			dv_core->mTvideoEncodeSize[index].width,
			dv_core->mTvideoEncodeSize[index].height);

	if (justAudio != 1) {
		ret = TRsetCameraEnable(dv_core->mTrecorder[index], 1);
		ret = TRsetCameraInputFormat(dv_core->mTrecorder[index],
				dv_core->mTcameraFormat[index]);
		ret = TRsetCameraFramerate(dv_core->mTrecorder[index],
				dv_core->mTencodeFramerate[index]);
		ret = TRsetCameraCaptureSize(dv_core->mTrecorder[index],
				dv_core->mTcameraCaptureSize[index].width,
				dv_core->mTcameraCaptureSize[index].height);
	}

	ret = TRsetMICEnable(dv_core->mTrecorder[index], 1);
	ret = TRsetMICInputFormat(dv_core->mTrecorder[index],
			dv_core->mTmicFormat[index]);
	ret = TRsetMICSampleRate(dv_core->mTrecorder[index],
			dv_core->mTmicSampleRate[index]);
	ret = TRsetMICChannels(dv_core->mTrecorder[index],
			dv_core->mTmicChannels[index]);

	ret = TRsetAudioMute(dv_core->mTrecorder[index],
			dv_core->mTaudioMute[index]);

	if (index == 0)
		ret = TRsetRecorderCallback(dv_core->mTrecorder[index],
				CallbackFromTRecorder0, (void*) dv_core);
	else
		ret = TRsetRecorderCallback(dv_core->mTrecorder[index],
				CallbackFromTRecorder1, (void*) dv_core);

	if (justAudio != 1) {
		ret = TRsetPreviewRoute(dv_core->mTrecorder[index],
				dv_core->mTRoute[index]);
		ret = TRsetPreviewEnable(dv_core->mTrecorder[index], 1);
		ret = TRsetPreviewRotate(dv_core->mTrecorder[index],
				dv_core->mTrotateDegree[index]);
		if (dv_core->mTRoute[index] == T_ROUTE_VE)
			ret = TRsetVEScaleDownRatio(dv_core->mTrecorder[index],
					dv_core->mTscaleDownRatio[index]);

		ret = TRsetPreviewRect(dv_core->mTrecorder[index],
				&dv_core->mTpreviewRect[index]);
		ret = TRsetPreviewZorder(dv_core->mTrecorder[index],
				dv_core->mTZorder[index]);
	}

	ret = TRprepare(dv_core->mTrecorder[index]);
	if (ret == 0)
		dv_core->recordSta[index] = RECORD_STOP;
	return ret;
}

/* @justAudio 0 Voice recording 1 voice recording */
int recorder_exit(int index, int justAudio) {
	int i = 0;
	if (dv_core->recordSta[index] == RECORD_UNINIT) {
		return -1;
	}
	if (justAudio != 1)
		TRsetPreviewEnable(dv_core->mTrecorder[index], 0);
	TRstop(dv_core->mTrecorder[index], T_ALL);
	TRrelease(dv_core->mTrecorder[index]);
	dv_core->mTrecorder[index] = NULL;
	dv_core->recordSta[index] = RECORD_UNINIT;
	return 0;
}

int recorder_reserve_size(void) {
	int f_size0, f_size1;
	f_size0 = ((dv_core->mTencoderBitRate[0] + 1000000) / 1000 / 1000) * 1024
			* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[0] / 1000);
	f_size1 = ((dv_core->mTencoderBitRate[1] + 1000000) / 1000 / 1000) * 1024
			* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[1] / 1000);
	return (f_size0 + f_size1 + RESERVED_SPACE + 1024 * 1024) >> 10;
}

/**
 * Can delete the file is enough.
 *
 * @return 1 enough with 0 not enough -1 other errors
 */
int recorder_ish_deleted_file(void) {
	int flag, i;
	char compete_path[128];
	list_node_t *tmp_node;
	int tmp_size;
	int f_size;
	struct stat sta;
	CHECK_NULL_POINTER(rec_list_A);
	CHECK_NULL_POINTER(rec_list_B);
	memset(compete_path, 0, sizeof(compete_path));
	tmp_node = rec_list_A->next;
	tmp_size = 0;
	flag = 0;
	f_size = ((dv_core->mTencoderBitRate[0] + 1000000) / 1000 / 1000) * 1024
			* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[0] / 1000);
	while (tmp_node) {
		sprintf(compete_path, "%s/%s", PARTH_A, tmp_node->filename);
		stat(compete_path, &sta);
		tmp_size += (512 * sta.st_blocks);
		if (tmp_size > f_size) {
			flag = 1;
			break;
		}
		tmp_node = tmp_node->next;
	}
	if (flag == 0) {
		return 0;
	}
	tmp_node = rec_list_B->next;
	tmp_size = 0;
	flag = 0;
	f_size = ((dv_core->mTencoderBitRate[1] + 1000000) / 1000 / 1000) * 1024
			* 1024 / 8 * (dv_core->mTmaxRecordTimeMs[1] / 1000);
	while (tmp_node) {
		sprintf(compete_path, "%s/%s", PARTH_B, tmp_node->filename);
		stat(compete_path, &sta);
		tmp_size += (512 * sta.st_blocks);
		if (tmp_size > f_size) {
			flag = 1;
			break;
		}
		tmp_node = tmp_node->next;
	}
	if (flag == 0) {
		return 0;
	}
	return 1;
}

int recorder_start_preview(int index) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);

	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] == RECORD_UNINIT) {
		printf("index %d is not init.\n", index);
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	TRstart(dv_core->mTrecorder[index], T_PREVIEW);
	return 0;
}

int recorder_stop_preview(int index) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);
	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] == RECORD_UNINIT) {
		printf("index %d is not init.\n", index);
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	TRstop(dv_core->mTrecorder[index], T_PREVIEW);
	return 0;
}

/* Switch display layer */
int recorder_preview_en(int index, Bool onoff) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);
	TRsetPreviewEnable(dv_core->mTrecorder[index], onoff);
	return 0;
}

int recorder_start_recording(int index) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);
	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] != RECORD_STOP) {
		printf("not prepared or stop state.\n");
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	if (!rec_list_A || !rec_list_B) {
		printf("list is not create!\n");
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}

	if (index == 0) {
		collide_f00 = 0;
		collide_f01 = 0;
		collide_f02 = 0;
		cur_collide_f0 = 0;
		pre_collide_f0 = 0;
		first_file_f0 = 0;
	} else {
		collide_f10 = 0;
		collide_f11 = 0;
		collide_f12 = 0;
		cur_collide_f1 = 0;
		pre_collide_f1 = 0;
		first_file_f1 = 0;
	}
	/* Start recording */
	TRstart(dv_core->mTrecorder[index], T_RECORD);
	if (index == 0) {
		gettimeofday(&time0, NULL);
	} else {
		gettimeofday(&time1, NULL);
	}
	pthread_mutex_lock(&dv_core->mutex1[index]);
	dv_core->recordSta[index] = RECORD_START;
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	return 0;
}

int recorder_stop_recording(int index) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);
	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] != RECORD_START) {
		printf("record not start!\n");
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	/* Stop recording */
	TRstop(dv_core->mTrecorder[index], T_RECORD);
	/* Collision save */
	if (index == 0) {
		if (collide_f00 == 1) {
			rename_pre_file(rec_list_A, 0);
			collide_f00 = 0;
		}
		if (collide_f01 == 1) {
			rename_cur_file(rec_list_A, 0);
			collide_f01 = 0;
		}
		cur_collide_f0 = 0;
		pre_collide_f0 = 0;
		first_file_f0 = 0;
	} else {
		if (collide_f10 == 1) {
			rename_pre_file(rec_list_B, 1);
			collide_f10 = 0;
		}
		if (collide_f11 == 1) {
			rename_cur_file(rec_list_B, 1);
			collide_f11 = 0;
		}
		cur_collide_f1 = 0;
		pre_collide_f1 = 0;
		first_file_f1 = 0;
	}
	pthread_mutex_lock(&dv_core->mutex1[index]);
	dv_core->recordSta[index] = RECORD_STOP;
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	return 0;
}

int recorder_take_picture(int index) {
	int status;
	DIR *dir;
	TCaptureConfig config;
	CHECK_NULL_POINTER(dv_core);
	if ((dir = opendir(PARTH_A)) == NULL) {
		printf("%s is not exist.\n", PARTH_A);
		status = mkdir(PARTH_A, S_IRWXU | S_IRWXG | S_IRWXO);
		if (status < 0) {
			printf("mkdir failed %s.\n", PARTH_A);
			return -1;
		}
	} else {
		closedir(dir);
	}
	if ((dir = opendir(PARTH_B)) == NULL) {
		printf("%s is not exist.\n", PARTH_B);
		status = mkdir(PARTH_B, S_IRWXU | S_IRWXG | S_IRWXO);
		if (status < 0) {
			printf("mkdir failed %s.\n", PARTH_B);
			return -1;
		}
	} else {
		closedir(dir);
	}
	memset(&config, 0, sizeof(TCaptureConfig));
	/* Check if it is ready */
	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] == RECORD_UNINIT) {
		printf("record not init!\n");
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); /*Get local time*/
	/* printf("%d%d%d ", (1900+p->tm_year),(1 + p->tm_mon), p->tm_mday); */
	/* printf("%d:%d:%d\n", p->tm_hour, p->tm_min, p->tm_sec); */
	if (index == 0)
		sprintf(config.capturePath, "%s/AW_%d%02d%02d_%02d%02d%02dA.jpg",
				PARTH_A, (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday,
				p->tm_hour, p->tm_min, p->tm_sec);
	else
		sprintf(config.capturePath, "%s/AW_%d%02d%02d_%02d%02d%02dB.jpg",
				PARTH_B, (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday,
				p->tm_hour, p->tm_min, p->tm_sec);
	config.captureFormat = dv_core->mTcaptureFormat[index];
	config.captureWidth = dv_core->mTcaptureSize[index].width;
	config.captureHeight = dv_core->mTcaptureSize[index].height;
	int ret;
	ret = TRCaptureCurrent(dv_core->mTrecorder[index], &config);
	return ret;
}

int recorder_mute_en(int index, Bool onoff) {
	CHECK_NULL_POINTER(dv_core);
	CHECK_NULL_POINTER(dv_core->mTrecorder[index]);
	/* Check if it is ready */
	pthread_mutex_lock(&dv_core->mutex1[index]);
	if (dv_core->recordSta[index] == RECORD_UNINIT) {
		printf("record not init!\n");
		pthread_mutex_unlock(&dv_core->mutex1[index]);
		return -1;
	}
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	TRsetAudioMute(dv_core->mTrecorder[index], onoff);
	return 0;
}

int recorder_get_rec_state(int index) {
	int ret;
	CHECK_NULL_POINTER(dv_core);
	pthread_mutex_lock(&dv_core->mutex1[index]);
	ret = dv_core->recordSta[index];
	pthread_mutex_unlock(&dv_core->mutex1[index]);
	return ret;
}

int recorder_get_cur_rectime(int index) {
	double diff;
	unsigned long long t0;
	unsigned int t1;
	static struct timeval time2;
	CHECK_NULL_POINTER(dv_core);
	gettimeofday(&time2, NULL);
	if (index == 0) {
		diff = (1000000.0 * (time2.tv_sec - time0.tv_sec) + time2.tv_usec
				- time0.tv_usec) / 1000000.0;
	} else {
		diff = (1000000.0 * (time2.tv_sec - time1.tv_sec) + time2.tv_usec
				- time1.tv_usec) / 1000000.0;
	}
	t0 = round(diff);
	t1 = t0 % (dv_core->mTmaxRecordTimeMs[index] / 1000);

	return t1;
}

int collide_save_file(int index) {
	int cur_time = 0;
	CHECK_NULL_POINTER(dv_core);
	cur_time = recorder_get_cur_rectime(index);
	if (cur_time < 0) {
		return -1;
	}
	if (index == 0) {
		pthread_mutex_lock(&dv_core->mutex0[index]);
		if (cur_collide_f0 == 1) {
			if (cur_time > dv_core->mTmaxRecordTimeMs[0] / 1000 - 10)
				collide_f02 = 1;
		} else {
			if (cur_time < 10) {
				if ((first_file_f0 == 1) || (pre_collide_f0 == 1)) {
					collide_f00 = 0;
				} else {
					collide_f00 = 1;
				}
				collide_f01 = 1;
				collide_f02 = 0;
			} else if (cur_time > dv_core->mTmaxRecordTimeMs[0] / 1000 - 10) {
				collide_f00 = 0;
				collide_f01 = 1;
				collide_f02 = 1;
			} else {
				collide_f00 = 0;
				collide_f01 = 1;
				collide_f02 = 0;
			}
		}
		pthread_mutex_unlock(&dv_core->mutex0[index]);
	} else {
		pthread_mutex_lock(&dv_core->mutex0[index]);
		if (cur_collide_f1 == 1) {
			if (cur_time > dv_core->mTmaxRecordTimeMs[1] / 1000 - 10)
				collide_f12 = 1;
		} else {
			if (cur_time < 10) {
				if ((first_file_f1 == 1) || (pre_collide_f1 == 1)) {
					collide_f10 = 0;
				} else {
					collide_f10 = 1;
				}
				collide_f11 = 1;
				collide_f12 = 0;
			} else if (cur_time > dv_core->mTmaxRecordTimeMs[1] / 1000 - 10) {
				collide_f10 = 0;
				collide_f11 = 1;
				collide_f12 = 1;
			} else {
				collide_f10 = 0;
				collide_f11 = 1;
				collide_f12 = 0;
			}
		}
		pthread_mutex_unlock(&dv_core->mutex0[index]);
	}
	return 0;
}

int recorder_init_info(R_SIZE frontCamera, R_SIZE backCamera,
		R_SIZE frontRreviewRect, R_SIZE backRreviewRect) {
	int i;
	if (!dv_core) {
		dv_core = malloc(sizeof(__dv_core_t ));
		if (!dv_core) {
			printf("malloc failed!\n");
			return -1;
		}
		memset(dv_core, 0, sizeof(__dv_core_t ));
	} else {
		return 0;
	}

	if (frontCamera.x < T_CAMERA_YUV420SP || frontCamera.x > T_CAMERA_BGRA)
		frontCamera.x = T_CAMERA_YVU420SP;
	if (frontCamera.y < T_ROUTE_ISP || frontCamera.y > T_ROUTE_CAMERA)
		frontCamera.y = T_ROUTE_VE;
	if (backCamera.x < T_CAMERA_YUV420SP || backCamera.x > T_CAMERA_BGRA)
		backCamera.x = T_CAMERA_YUYV422;
	if (backCamera.y < T_ROUTE_ISP || backCamera.y > T_ROUTE_CAMERA)
		backCamera.y = T_ROUTE_CAMERA;

	for (i = 0; i < SENSOR_NUM; i++) {
		dv_core->mTcameraIndex[i] = T_CAMERA_FRONT;
		dv_core->mTmicIndex[i] = T_AUDIO_MIC0;
		dv_core->mTdispIndex[i] = T_DISP_LAYER0;
		dv_core->mTcameraEnableWM[i] = 1;
		dv_core->mToutputFormat[i] = T_OUTPUT_MOV;
		dv_core->mTvideoEncodeFormat[i] = T_VIDEO_H264;
		dv_core->mTaudioEncodeFormat[i] = T_AUDIO_AAC;
		dv_core->mTmaxRecordTimeMs[i] = 60 * 1000;
		dv_core->mTencoderBitRate[i] = 8 * 1000 * 1000;
		dv_core->mTencodeFramerate[i] = 30;
		dv_core->mTvideoEncodeSize[i].width = 640;
		dv_core->mTvideoEncodeSize[i].height = 480;
		dv_core->mTcameraFormat[i] = frontCamera.x;
		dv_core->mTcameraCaptureSize[i].width = frontCamera.width;
		dv_core->mTcameraCaptureSize[i].height = frontCamera.height;
		dv_core->mTmicFormat[i] = T_MIC_PCM;
		dv_core->mTmicSampleRate[i] = 8 * 1000;
		dv_core->mTmicChannels[i] = 2;
		dv_core->mTaudioMute[i] = 0;
		dv_core->mTRoute[i] = frontCamera.y;
		dv_core->mTrotateDegree[i] = T_ROTATION_ANGLE_0;
		dv_core->mTscaleDownRatio[i] = 2;
		dv_core->mTpreviewRect[i].x = frontRreviewRect.x;
		dv_core->mTpreviewRect[i].y = frontRreviewRect.y;
		dv_core->mTpreviewRect[i].width = frontRreviewRect.width;
		dv_core->mTpreviewRect[i].height = frontRreviewRect.height;
		dv_core->mTZorder[i] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->recordSta[i] = RECORD_STOP;
		dv_core->mTcaptureFormat[i] = T_CAPTURE_JPG;
		dv_core->mTcaptureSize[i].width = 640;
		dv_core->mTcaptureSize[i].height = 480;
		dv_core->recordSta[i] = RECORD_UNINIT;
		dv_core->pre_mode = PREVIEW_FRONT_UP;

		if (i == 1) {
			dv_core->mTcameraIndex[i] = T_CAMERA_BACK;
			dv_core->mTmicIndex[i] = T_AUDIO_MIC1;
			dv_core->mTdispIndex[i] = T_DISP_LAYER1;
			dv_core->mTcameraFormat[i] = backCamera.x;
			dv_core->mTcameraCaptureSize[i].width = backCamera.width;
			dv_core->mTcameraCaptureSize[i].height = backCamera.height;
			dv_core->mTRoute[i] = backCamera.y;
			dv_core->mTpreviewRect[i].x = backRreviewRect.x;
			dv_core->mTpreviewRect[i].y = backRreviewRect.y;
			dv_core->mTpreviewRect[i].width = backRreviewRect.width;
			dv_core->mTpreviewRect[i].height = backRreviewRect.height;
			dv_core->mTZorder[i] = T_PREVIEW_ZORDER_MIDDLE;
		}
	}

	for (i = 0; i < SENSOR_NUM; i++) {
		pthread_mutex_init(&dv_core->mutex0[i], NULL);
		pthread_mutex_init(&dv_core->mutex1[i], NULL);
	}

	int ret;
	/* Initialize the card mount path */
	char mountPathA[64];
	ret = get_mount_path(&mountPathA);
	if (ret == -1) {
		strcpy(PARTH_A, "/mnt/SDCARD/DCIMA");
	} else {
		strcat(mountPathA, "/DCIMA");
		strcpy(PARTH_A, mountPathA);
	}

	char mountPathB[64];
	ret = get_mount_path(&mountPathB);
	if (ret == -1) {
		strcpy(PARTH_B, "/mnt/SDCARD/DCIMB");
	} else {
		strcat(mountPathB, "/DCIMB");
		strcpy(PARTH_B, mountPathB);
	}

	return 0;
}

int recorder_exit_info(void) {
	int i;
	CHECK_NULL_POINTER(dv_core);
	for (i = 0; i < SENSOR_NUM; i++) {
		pthread_mutex_destroy(&dv_core->mutex0[i]);
		pthread_mutex_destroy(&dv_core->mutex1[i]);
	}
	free(dv_core);
	dv_core = NULL;
	return 0;
}

int create_rec_list(void) {
	rec_list_A = create_list_by_path(PARTH_A);
	rec_list_B = create_list_by_path(PARTH_B);
	if (!rec_list_A || !rec_list_B)
		return -1;
	return 0;
}

int destroy_rec_list(void) {
	if (rec_list_A) {
		destroy_list(rec_list_A);
		rec_list_A = NULL;
	}
	if (rec_list_B) {
		destroy_list(rec_list_B);
		rec_list_B = NULL;
	}
	return 0;
}

__dv_core_t *get_dv_core_t(void) {
	return dv_core;
}
