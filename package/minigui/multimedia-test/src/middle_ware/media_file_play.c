#include "media_file_play.h"
#include "common.h"
/* #define MEDIA_PARTH_A  "/mnt/SDCARD/DCIMA" */
/* #define MEDIA_PARTH_B  "/mnt/SDCARD/DCIMB" */

static list_head_t *media_list;
static int count = 1;
static int count1 = 0;
static disp_rect d_rect;
static int show_mode;
static dispOutPort *disp_out_port;
static int show_f;
static JpegDecoder* jpegdecoder;
static TRetriever* metedataretriever;
static char MEDIA_PARTH_A[64];

/* Judge whether it is a special directory */
static int is_special_dir(const char *path) {
	return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}
int reorder_media_list(void) {
	count1 = 1;
	return 0;
}
int noreorder_media_list(void) {
	count1 = 0;
	return 0;
}

int sort_list(list_head_t *head) {
	list_node_t *temp_node;
	list_node_t *next_node;
	char name1[64];
	char name11[64];
	char name2[64];
	char name22[64];
	int i, j, m;
	s32 total;

	if (NULL == head)
		return NULL;

	total = media_get_total_num();

	if (0 == total) {
		return 0;
	}

	/* Using bubble sort, according to the video sequence of time */
	for (j = 0; j < total - 1; j++) {
		for (i = 0; i < total - 1 - j; i++) {
			temp_node = head->first_node;
			for (m = 0; m < i; m++) {
				temp_node = temp_node->next;
			}
			next_node = temp_node->next;

			memset(name1, 0, sizeof(name1));
			memset(name11, 0, sizeof(name11));
			head->current_node = temp_node;
			media_get_current_file_name(name1);
			strncpy(name11, name1 + 2, 16);

			memset(name2, 0, sizeof(name2));
			memset(name22, 0, sizeof(name22));
			head->current_node = next_node;
			media_get_current_file_name(name2);
			strncpy(name22, name2 + 2, 16);

			if (strcmp(name11, name22) < 0) {
				temp_node->pre->next = next_node;
				next_node->pre = temp_node->pre;
				temp_node->next = next_node->next;
				next_node->next->pre = temp_node;
				next_node->next = temp_node;
				temp_node->pre = next_node;
				if (0 == i) {
					head->first_node = next_node;
				}
			}
		}
	}
	head->current_node = head->first_node;

	return 0;
}

int zw_sort_list(list_head_t *head) {

	list_node_t *temp_node;
	list_node_t *sort_node;
	list_node_t *temp_node_count;
	static int countfirstnode = 0;
	/* Used to record whether the first file is a "ZW" file */
	static int countfirstnode1 = 0;
	s32 total;
	char name1[64];
	char name11[64];

	if (NULL == head)
		return NULL;

	total = media_get_total_num();

	if (0 == total) {
		return 0;
	}

	/* First find the file that is not in "ZW" format */
	temp_node = head->first_node;

	memset(name1, 0, sizeof(name1));
	memset(name11, 0, sizeof(name11));
	head->current_node = temp_node;
	media_get_current_file_name(name1);
	strncpy(name11, name1, 2);

	while (strcmp(name11, "ZW") == 0) {
		countfirstnode1 = 1;
		temp_node = temp_node->next;

		memset(name1, 0, sizeof(name1));
		memset(name11, 0, sizeof(name11));
		head->current_node = temp_node;
		media_get_current_file_name(name1);
		strncpy(name11, name1, 2);
	}
	sort_node = temp_node;

	/* Traverse the list, the "ZW" format file into the first not "ZW" format file in front of */
	temp_node = temp_node->next;
	while (temp_node != head->first_node) {
		memset(name1, 0, sizeof(name1));
		memset(name11, 0, sizeof(name11));
		head->current_node = temp_node;
		media_get_current_file_name(name1);
		strncpy(name11, name1, 2);

		if (strcmp(name11, "ZW") == 0) {
			countfirstnode++;
			if (countfirstnode == 1 && countfirstnode1 == 0) {
				head->first_node = temp_node;
			}
			temp_node_count = temp_node->pre;

			temp_node->pre->next = temp_node->next;
			temp_node->next->pre = temp_node->pre;
			sort_node->pre->next = temp_node;
			temp_node->pre = sort_node->pre;
			temp_node->next = sort_node;
			sort_node->pre = temp_node;

			temp_node = temp_node_count;
		}
		temp_node = temp_node->next;
	}
	countfirstnode = 0;
	head->current_node = head->first_node;

	return 0;
}
static void get_filelist_path(void) {
	int ret;
	char mountPathA[64];
	ret = get_mount_path(&mountPathA);
	if (ret == -1) {
		strcpy(MEDIA_PARTH_A, "/mnt/SDCARD/DCIMA");
	} else {
		strcat(mountPathA, "/DCIMA");
		strcpy(MEDIA_PARTH_A, mountPathA);
	}

}
int create_media_list_player(void) {
	DIR *dir;
	int status;
	struct dirent *dir_info;
	char compete_path[64];
	list_node_t *count_node;
	int a;
	if (media_list) {
		printf("list is exist!\n");
		return -1;
	}
	media_list = create_list();
	get_filelist_path();
	if ((dir = opendir(MEDIA_PARTH_A)) == NULL) {
		printf("%s is not exist.\n", MEDIA_PARTH_A);
		return 0;
	}
	while ((dir_info = readdir(dir)) != NULL) {
		/*  Ignore the file. */
		if (is_special_dir(dir_info->d_name))
			continue;
		if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4, ".mp4")
				== 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);
		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 3,
				".ts") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);
		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4,
				".mov") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);

		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4,
				".mkv") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);

		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4,
				".avi") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);

		}

	}

	closedir(dir); /* Close the directory */
#if 0

			if((dir = opendir(MEDIA_PARTH_B)) == NULL) {
				printf("%s is not exist.\n", MEDIA_PARTH_B);
				return 0;
			}
			while((dir_info = readdir(dir)) != NULL) {
				/* Ignore the file. */
				if(is_special_dir(dir_info->d_name))
				continue;
				if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".mp4") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path,FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_VIDEO);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-3, ".ts") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_VIDEO);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".mov") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_VIDEO);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".jpg") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".png") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".mp3") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".wav") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".aac") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".mkv") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".avi") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-5, ".jpeg") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".gif") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				} else if(strcmp(dir_info->d_name+strlen(dir_info->d_name)-4, ".bmp") == 0) {
					memset(compete_path, 0, sizeof(compete_path));
					snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_B, dir_info->d_name);
					list_append(media_list,compete_path, MEDIA_F_TYPE_PICTURE);
				}

			}
			closedir(dir); /* Close the directory */
#endif
	/* sort_list(media_list);		Sort by time */
	/* zw_sort_list(media_list);	Put the zw file in front */

	if (count1 == 1) {
		count_node = media_list->first_node;
		a = count;
		for (int i = 1; i < count; i++)
			count_node = count_node->next;
		media_list->current_node = count_node;
		media_list->cur_index = a;
	}

	return 0;
}

int create_media_list_music(void) {
	DIR *dir;
	int status;
	struct dirent *dir_info;
	char compete_path[64];
	list_node_t *count_node;
	int a;
	if (media_list) {
		printf("list is exist!\n");
		return -1;
	}
	media_list = create_list();
	get_filelist_path();
	if ((dir = opendir(MEDIA_PARTH_A)) == NULL) {
		printf("%s is not exist.\n", MEDIA_PARTH_A);
		return 0;
	}
	while ((dir_info = readdir(dir)) != NULL) {
		/* Ignore the file. */
		if (is_special_dir(dir_info->d_name))
			continue;
		if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4, ".mp3")
				== 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_VIDEO);
		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4,
				".aac") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_PICTURE);
		}
	}
	closedir(dir);
	if (count1 == 1) {
		count_node = media_list->first_node;
		a = count;
		for (int i = 1; i < count; i++)
			count_node = count_node->next;
		media_list->current_node = count_node;
		media_list->cur_index = a;
	}

	return 0;
}
int create_media_list_picture(void) {
	DIR *dir;
	int status;
	struct dirent *dir_info;
	char compete_path[64];
	list_node_t *count_node;
	int a;
	if (media_list) {
		printf("list is exist!\n");
		return -1;
	}
	media_list = create_list();
	get_filelist_path();
	if ((dir = opendir(MEDIA_PARTH_A)) == NULL) {
		printf("%s is not exist.\n", MEDIA_PARTH_A);
		return 0;
	}
	while ((dir_info = readdir(dir)) != NULL) {
		/* Ignore the file. */
		if (is_special_dir(dir_info->d_name))
			continue;
		if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4, ".jpg")
				== 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_PICTURE);
		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 4,
				".bmp") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_PICTURE);
		} else if (strcmp(dir_info->d_name + strlen(dir_info->d_name) - 5,
				".jpeg") == 0) {
			memset(compete_path, 0, sizeof(compete_path));
			snprintf(compete_path, FILE_PATH_MAXT_LEN, "%s/%s", MEDIA_PARTH_A,
					dir_info->d_name);
			list_append(media_list, compete_path, MEDIA_F_TYPE_PICTURE);
		}
	}
	closedir(dir);
	if (count1 == 1) {
		count_node = media_list->first_node;
		a = count;
		for (int i = 1; i < count; i++)
			count_node = count_node->next;
		media_list->current_node = count_node;
		media_list->cur_index = a;
	}

	return 0;
}

int destroy_media_list(void) {
	if (media_list != NULL) {
		count = media_list->cur_index;
		destroy_list(media_list);
		media_list = NULL;
	}
	return 0;
}

int show_current_file(void) {
	VoutRect dis_rect;
	videoParam vparam;
	list_node_t *current_node;

#if 1
	dis_rect.x = d_rect.x;
	dis_rect.y = d_rect.y;
	dis_rect.width = d_rect.width;
	dis_rect.height = d_rect.height;
#endif

	current_node = list_get_current_node(media_list);

	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	if (!current_node) {
		printf("current node is null!\n");
		return -1;
	}

	printf("current path = %s\n", current_node->path);

	if (current_node->type == MEDIA_F_TYPE_VIDEO) {
		/* Play thumbnail */
		metedataretriever = TRetrieverCreate();
		if (NULL == metedataretriever) {
			printf("create metedataretriever failed\n");
			return -1;
		}
		TRetrieverSetDataSource(metedataretriever, current_node->path,
				TMETADATA_RETRIEVER_SCALE_DOWN_2,
				TmetadataRetrieverOutputDataYV12);
		VideoFrame* videoFrame = TRetrieverGetFrameAtTime(metedataretriever,
				-1);	/* 0 */
		if (videoFrame == NULL) {
			printf("TRetrieverGetFrameAtTime fail\n");
			TRetrieverDestory(metedataretriever);
			return -1;
		} else {
			printf(
					"TRetrieverGetFrameAtTime successfully,videoFrame->mWidth = %d,videoFrame->mHeight = %d,videoFrame->mDisplayWidth = %d,videoFrame->mDisplayHeight = %d\n",
					videoFrame->mWidth, videoFrame->mHeight,
					videoFrame->mDisplayWidth, videoFrame->mDisplayHeight);
			printf("videoFrame->mYuvData = %p,videoFrame->mYuvSize = %d\n",
					videoFrame->mYuvData, videoFrame->mYuvSize);
			printf(
					"videoFrame->mRGB565Data = %p,videoFrame->mRGB565Size = %d\n",
					videoFrame->mRGB565Data, videoFrame->mRGB565Size);
#if 0
			{
				FILE *fp = NULL;
				char path[128] = {0};

				printf("debug retriever data %dx%d_%d\n", videoFrame->mWidth, videoFrame->mHeight, videoFrame->mYuvSize);
				sprintf(path, "/mnt/SDCARD/re_%dx%d_%d.bat", videoFrame->mWidth, videoFrame->mHeight, videoFrame->mYuvSize);
				printf("path:%s\n", path);
				fp = fopen(path, "wb+");
				printf("open file %p(%s)\n", fp, path);
				if (fp)
				{
					fwrite(videoFrame->mYuvData, videoFrame->mYuvSize, 1, fp);
					fclose(fp);
				}
			}
#endif
		}

		disp_out_port = CreateVideoOutport(0);
		if (!disp_out_port) {
			printf("CreateVideoOutport failed!\n");
			return -1;
		}

		disp_out_port->init(disp_out_port, 1, ROTATION_ANGLE_0, &dis_rect);
		disp_out_port->setRoute(disp_out_port, VIDEO_SRC_FROM_FILE);

		vparam.isPhy = 0;
		/* fixme,set right width and height here */
		vparam.srcInfo.w = videoFrame->mDisplayWidth;
		vparam.srcInfo.h = videoFrame->mDisplayHeight;
		vparam.srcInfo.crop_x = 0;
		vparam.srcInfo.crop_y = 0;
		vparam.srcInfo.crop_w = videoFrame->mDisplayWidth;
		vparam.srcInfo.crop_h = videoFrame->mDisplayHeight;
		vparam.srcInfo.format = VIDEO_PIXEL_FORMAT_YV12;
		vparam.srcInfo.color_space = DISP_BT601;
		disp_out_port->allocateVideoMem(disp_out_port, &vparam);
		disp_out_port->writeData(disp_out_port, videoFrame->mYuvData,
				videoFrame->mYuvSize, &vparam);
		show_f = 1;
	} else {
		/* Play the picture */
		jpegdecoder = JpegDecoderCreate();
		if (NULL == jpegdecoder) {
			printf("create jpegdecoder failed\n");
			return -1;
		}
		JpegDecoderSetDataSource(jpegdecoder, current_node->path,
				JPEG_DECODE_SCALE_DOWN_4, JpegDecodeOutputDataYV12);
		ImgFrame* imgFrame = JpegDecoderGetFrame(jpegdecoder);
		if (imgFrame == NULL) {
			printf("JpegDecoderGetFrame fail\n");
			JpegDecoderDestory(jpegdecoder);
			return -1;
		} else {
			printf(
					"JpegDecoderGetFrame successfully,imgFrame->mDisplayWidth = %d,imgFrame->mDisplayHeight = %d,imgFrame->mYuvData = %p,imgFrame->mYuvSize = %d\n",
					imgFrame->mDisplayWidth, imgFrame->mDisplayHeight,
					imgFrame->mYuvData, imgFrame->mYuvSize);
			printf("mWidth = %d, mHeight = %d\n", imgFrame->mWidth,
					imgFrame->mHeight);
			printf("imgFrame->mRGB565Data = %p,imgFrame->mRGB565Size = %d\n",
					imgFrame->mRGB565Data, imgFrame->mRGB565Size);
		}
#if 1
		disp_out_port = CreateVideoOutport(0);
		if (!disp_out_port) {
			printf("CreateVideoOutport failed!\n");
			return -1;
		}
		disp_out_port->init(disp_out_port, 1, ROTATION_ANGLE_0, &dis_rect);
		disp_out_port->setRoute(disp_out_port, VIDEO_SRC_FROM_FILE);
#endif
		vparam.isPhy = 0;
		/* fixme,set right width and height here */
		vparam.srcInfo.w = imgFrame->mDisplayWidth;
		vparam.srcInfo.h = imgFrame->mDisplayHeight;
		vparam.srcInfo.crop_x = 0;
		vparam.srcInfo.crop_y = 0;
		vparam.srcInfo.crop_w = imgFrame->mDisplayWidth;
		vparam.srcInfo.crop_h = imgFrame->mDisplayHeight;
		vparam.srcInfo.format = VIDEO_PIXEL_FORMAT_YV12;
		vparam.srcInfo.color_space = DISP_BT601;
		disp_out_port->allocateVideoMem(disp_out_port, &vparam);
		disp_out_port->writeData(disp_out_port, imgFrame->mYuvData,
				imgFrame->mYuvSize, &vparam);
		show_f = 2;
	}

	return 0;
}

int show_next_file(void) {
	list_node_t *current_node;
	list_node_t *current_node1;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	current_node1 = list_get_next_node(media_list);
	if (current_node == current_node1) {
		/* return -1; */
	}
	printf("show current file\n");
	show_current_file();
	printf("show current file11\n");
	return 0;
}

int show_pre_file(void) {
	list_node_t *current_node;
	list_node_t *current_node1;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	current_node1 = list_get_pre_node(media_list);
	if (current_node == current_node1) {
		return -1;
	}
	show_current_file();
	return 0;
}

int de_show(void) {
	if (show_f == 1) {
		/* disp_out_port->setEnable(disp_out_port, 1); */
		disp_out_port->freeVideoMem(disp_out_port);
		disp_out_port->deinit(disp_out_port);
		DestroyVideoOutport(disp_out_port);
		disp_out_port = NULL;
		TRetrieverDestory(metedataretriever);
		metedataretriever = NULL;
	} else if (show_f == 2) {
		/* disp_out_port->setEnable(disp_out_port, 1); */
		disp_out_port->freeVideoMem(disp_out_port);
		disp_out_port->deinit(disp_out_port);
		DestroyVideoOutport(disp_out_port);
		disp_out_port = NULL;
		JpegDecoderDestory(jpegdecoder);
		jpegdecoder = NULL;
	}
	show_f = 0;
	return 0;
}
media_file_type_t media_get_current_file_type(void) {
	list_node_t *current_node;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	if (!current_node) {
		printf("current node is null!\n");
		return -1;
	}
	return current_node->type;
}

int media_get_current_file_name(char *name) {
	list_node_t *current_node;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	if (!current_node) {
		printf("current node is null!\n");
		return -1;
	}
	char *p = strrchr(current_node->path, '/');
	p = p + 1;
	strncpy(name, p, FILE_NAME_MAXT_LEN);
	return 0;
}
int media_get_current_file_path(char *path) {
	list_node_t *current_node;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	if (!current_node) {
		printf("current node is null!\n");
		return -1;
	}
	strncpy(path, current_node->path, FILE_PATH_MAXT_LEN);
	return 0;
}
int skip_to_head() {
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	printf("skip to head\n");
	media_list->current_node = media_list->first_node;
	media_list->cur_index = 1;
	printf("cur_index = 1\n");
}
int skip_to_index(int index, int direction) {
	int i = 0;
	list_node_t *next_node;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	if (direction == 1) {
		for (i = 0; i < index; ++i) {
			next_node = list_get_next_node(media_list);
			if (!next_node) {
				printf("next node is null!\n");
				return -1;
			}
		}
	} else if (direction == -1) {
		for (i = index; i > 0; --i) {
			next_node = list_get_pre_node(media_list);
			if (!next_node) {
				printf("next node is null!\n");
				return -1;
			}
		}
	}
}
int media_get_next_name_path(char *name, char *path)
{
	list_node_t *next_node;
	if (!media_list) {
		printf("media_list is null!\n");
		 return -1;
	}
	next_node = list_get_next_node(media_list);
	if(!next_node){
		printf("next node id null!\n");
		return -1;
	}
	char *p = strrchr(next_node->path, '/');
	p = p + 1;
	strncpy(name, p, FILE_NAME_MAXT_LEN);
	strncpy(path, next_node->path, FILE_PATH_MAXT_LEN);
}
int media_get_total_num(void) {
	return list_get_total_num(media_list);
}

int media_get_cur_index(void) {
	return list_get_cur_index(media_list);
}

int media_del_cur_file(void) {
	list_node_t *current_node;
	if (!media_list) {
		printf("media_list is null!\n");
		return -1;
	}
	current_node = list_get_current_node(media_list);
	if (!current_node) {
		printf("current node is null!\n");
		return -1;
	}
	remove(current_node->path);
	list_del_cur_node(media_list);
	return 0;
}

int media_play_init(disp_rect rect, show_pic_mode_t mode) {
	VoutRect dis_rect;
	d_rect = rect;
	show_mode = mode;
	return 0;
}

int media_play_exit(void) {
	return 0;
}
