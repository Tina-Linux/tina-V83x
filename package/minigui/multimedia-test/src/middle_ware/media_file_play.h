#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <jpegdecode.h>
#include "sunxi_display_v2.h"
#include "mid_list.h"
#include "videoOutPort.h"
#include <tmetadataretriever.h>

typedef enum SHOW_PIC_MODE_T {
	/* To the image itself is scaled to full window display, the picture is not distorted */
	SHOW_PIC_WIN_BESTSHOW = 0x00,
	/* The original size of the image in the window, can not overflow the window */
	SHOW_PIC_WIN_ORIGINAL,
	/* Zooming a window to a full window display may be distorted */
	SHOW_PIC_WIN_FULLSCN,
	/* Trimming mode, in the srcFrame area and then cut off the top and bottom black edge, */
	/* trimming, the bestshow mode display */
	SHOW_PIC_WIN_CUTEDGE,
	/* Do not care about the ratio of the picture display to the currently set ratio */
	SHOW_PIC_WIN_NOTCARE,
	/* To the image itself is scaled to full window display, the picture remains unchanged, */
	/* the picture exceeds the part of the cut out */
	SHOW_PIC_WIN_ORIG_CUTEDGE_FULLSCN,
	SHOW_PIC_WIN_UNKNOWN
} show_pic_mode_t;

int reorder_media_list(void);
int noreorder_media_list(void);
/* int create_media_list(void); */
int create_media_list_player(void);
int create_media_list_music(void);
int create_media_list_picture(void);
int destroy_media_list(void);
int show_current_file(void);
int show_next_file(void);
int show_pre_file(void);
int de_show(void);
int skip_to_head();
media_file_type_t media_get_current_file_type(void);
int media_get_current_file_name(char *name);
int media_get_current_file_path(char *path);
int skip_to_index(int index, int direction);
int media_get_next_name_path(char *name, char *path);
int media_get_total_num(void);
int media_get_cur_index(void);
int media_del_cur_file(void);
int media_play_init(disp_rect rect, show_pic_mode_t mode);
int media_play_exit(void);
