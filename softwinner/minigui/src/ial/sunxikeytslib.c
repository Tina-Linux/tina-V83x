#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <errno.h>

#ifdef _MGIAL_SUNXIKEYTSLIB
#include <tslib.h>

#include "ial.h"
#include "sunxikeytslib.h"

#include "tslibial.h"


#define PRESS_INSISTENTLY 1
#define PRESS_QUICKLY 0

#define TIME_USE(end, start)(((end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))/1000)

//the touch screen
static struct tsdev *ts = NULL;
static int mousex = 0;
static int mousey = 0;
static int button = 0;

/* The orgin of coordinates need to be reversed
 * from the bottom right corner to the top left corner.
 * yangy
 */
static int reversed = 0;
static int screen_width  = 0;
static int screen_height = 0;

static unsigned char state[NR_KEYS];
static int fd_key = -1;
static int fd_key_power = -1;
static int mts_fd = -1;

static int cur_key = -2;

/************************  Low Level Input Operations **********************/
/*
 * Mouse/touch screen operations -- Event
 */
static int mouse_update (void)
{
    static int last_pressure = 0;
    static int ignore = 0;
    struct ts_sample samp;
   // fprintf(stderr, "mouse_update ts is 0000\n");
    if(!ts){
        fprintf(stderr, "mouse_update ts is NULL\n");
    }
    if (ts_read (ts, &samp, 1) > 0) {
        if(samp.x == 0 && samp.y == 0 && samp.pressure == 0)
            return 0;
        char *env = NULL;
        int delay = 25;
        if ((env = getenv("TS_DELAY")) != NULL) {
            delay = atoi(env);
        }
        if (delay != 0) {
            if (last_pressure == samp.pressure) {
                if (!ignore) {
                    ignore = 1;
                } else {
                    ignore = 0;
                    return 0;
                }
            }
            last_pressure = samp.pressure;
        }

        if (samp.pressure > 0) {
			if (reversed) {
				mousex = screen_width  - samp.x;
				mousey = screen_height - samp.y;
			} else {
				mousex = samp.x;
				mousey = samp.y;
                //fprintf(stderr,"habo  ---> sunxikeytslib   x = %d  y = %d \n",mousex,mousey);
			}
        }

        button = (samp.pressure > 0) ? IAL_MOUSE_LEFTBUTTON : 0;
        // 过滤掉up 消息后面带来的一个down 消息
#if 0
        static int up_flag = 0;
        if(button == 0 && up_flag == 0)
        {
            up_flag = 1;
        }
        else if(button == 1 && up_flag == 1)
        {
            up_flag = 0;
            button = 0;
        }
#endif
        return 1;
    }else{
        return 0;
    }
}


static void mouse_getxy (int* x, int* y)
{
    *x = mousex;
    *y = mousey;
}

static int mouse_getbutton(void)
{
	return button;
}

/*
 * Key operations -- Event
 */
static int castkey2scancode(void)
{
	int i;

	for (i=0; i<SUNXI_KEY_CNT; i++) {
		if(cur_key == keyts_set[i].keyCode) {
			return keyts_set[i].scancode;
		}
	}
	return -1;
}
static int unknown_key_down = 0;
static int keyboard_update(void)
{
    //fprintf(stderr, "IAL: unknown key pressed 11<%d>\n", cur_key);

	if (cur_key < 0)
	{
			memset(state, 0, sizeof(state));
			cur_key = -2;
	}
	else
	{
		    int scancode = castkey2scancode();
			if(scancode < 0)
			{
					fprintf(stderr, "IAL: unknown key pressed 22<%d>\n", cur_key);
					cur_key = -1;
					unknown_key_down = 1;
					return -1;
			}
			else
			{
					state[scancode] = 1;
			}
	}
    return NR_KEYS;
}

static const char* keyboard_getstate(void)
{
        //fprintf(stderr,"sunxikeytslib keyboard_getstate run here\n");
        return (char *) state;
}


static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
	fd_set rfds;
    int e;
	int fd;
	int retvalue = 0;

    /**********key ial event**************/
    struct input_event key;
    if(!in)
    {
        in = &rfds;
        FD_ZERO(in);
    }
    if(which & IAL_KEYEVENT)
    {
        if (fd_key > 0) {
            FD_SET(fd_key, in);
            if(fd_key > maxfd) maxfd = fd_key;
        }

        if (fd_key_power > 0) {
            FD_SET(fd_key_power, in);
            if(fd_key_power > maxfd) maxfd = fd_key_power;
        }
    }

    /**********mouse/touch_screen ial event**************/
    //if (ts) {
        if ((which & IAL_MOUSEEVENT) && mts_fd >= 0) {
            FD_SET (mts_fd, in);
            if (mts_fd > maxfd) maxfd = mts_fd;
        }
    //}

    e = select (maxfd+1, in, out, except, timeout);
    if (e < 0) {
        fprintf(stderr, "[file]:%s  [fun]:%s [line]:%d select failed, %s\n", __FILE__, __func__, __LINE__, strerror(errno));
        return -1;
    }

    if (e > 0) {
        /*********normal key************/
        if (fd_key > 0) {
            if(FD_ISSET(fd_key, in)) {
                FD_CLR(fd_key, in);
                read(fd_key, &key, sizeof(key));
                if (0 == key.code) {
                    return -1;
                } else if (key.value > 0){
                    cur_key = key.code;
                    fprintf(stderr, "the current key value = %d\n",cur_key);
                } else {
                    if (unknown_key_down) {
                        unknown_key_down = 0;
                        return -1;
                    }
                    cur_key = -1;
                }
                retvalue |= IAL_KEYEVENT;
            }
        }

        /*********power key************/
        if (fd_key_power > 0) {
            if(FD_ISSET(fd_key_power, in))
            {
                FD_CLR(fd_key_power, in);
                read(fd_key_power, &key, sizeof(key));
                if (0 == key.code) {
                    return -1;
                } else if (key.value > 0){
                    cur_key = key.code;
                } else {
                    cur_key = -1;
                }
                retvalue |= IAL_KEYEVENT;
            }
        }
        /*********touch screen************/
        if (ts) {
            if (mts_fd > 0 && FD_ISSET (mts_fd, in)) {
                retvalue |= IAL_MOUSEEVENT;
            }
        }
        return retvalue;
    }
}

#define STREQUAL(str1, str2) (!strcmp(str1, str2))

static int isKeyName(const char *name)
{
	int i;
	for (i=0; i<SUNXI_KEY_CNT; i++) {
		if(STREQUAL(name, keyts_set[i].name)) {
			return i;
		}
	}
	return -1;
}

static void initKeyCode(void)
{
    FILE *pRead;
    char sKey[16];
    char sName[32];
    int iCode;
	int idx;

    pRead = fopen(KEYCODE_FILE,"r");
    if (pRead == NULL) {
            return;
    }
    while(EOF!=fscanf(pRead, "%s%d%s", sKey, &iCode, sName)) {
		idx = isKeyName(sName);
		if (idx >= 0) {
			keyts_set[idx].keyCode = iCode;
		}
    }
    fclose(pRead);
}

BOOL    InitSUNXIKeyTSLibInput(INPUT* input, const char* mdev, const char* mtype)
{
	fprintf(stderr, "Init_TSLib&KEY_Input\n");
    char* env_value = NULL;
    char  mdev_ts [MAX_PATH + 1];

    /**************open the event of power key****************/
    fd_key_power = open(input->mdev0, O_RDONLY);

    /****************open the device event of normal key****************/
	fd_key = open(input->mdev1, O_RDONLY);

    /**********************open the event of touch screeen***************************/
    if((env_value = getenv ("MG_DOUBLEIAL_ENGINE"))) {
        strncpy (mdev_ts, env_value, MAX_PATH);
        mdev_ts [MAX_PATH] = '\0'; 
        fprintf(stderr, "[file]:%s  [fun]:%s [line]:%d the tslib medv is %s\n", __FILE__, __func__, __LINE__,mdev_ts);
    } else {
        strncpy (mdev_ts, input->mdev2, MAX_PATH);
        fprintf(stderr, "IAL>TSLib: can not find the env value, use the device node:%s from the MiniGUI.cfg\n", mdev_ts);
    }

    ts = ts_open (mdev_ts, 1);
    if(ts)
   {
        mts_fd = ts_fd (ts);
        if(mts_fd < 0){
            fprintf(stderr, "sunxikeytslib mt_fd is open filed\n");
        }
    }


    if(!ts && (fd_key < 0) && (fd_key_power < 0)) {
        printf("[file]:%s  [fun]:%s [line]:%d the all input nodes are not ready, please check the config of node!!!\n", __FILE__, __func__, __LINE__);
        input->wait_event = NULL;
    } else {
       if (!ts) {
            printf("[file]:%s  [fun]:%s [line]:%d the tslib node is not ready, please check the config of node!!!\n", __FILE__, __func__, __LINE__);
        }

        if (fd_key < 0) {
            printf("[file]:%s  [fun]:%s [line]:%d the normal key input node is not ready, please check the config of node!!!\n", __FILE__, __func__, __LINE__);
        }

        if (fd_key_power < 0) {
            printf("[file]:%s  [fun]:%s [line]:%d the power key input node is not ready, please check the config of node!!!\n", __FILE__, __func__, __LINE__);
        }

        if (ts && (!ts_config (ts)) && (fd_key >= 0) && (fd_key_power >= 0)) {
            
            printf("[file]:%s  [fun]:%s [line]:%d the all input nodes are ready to input and the ial can work normally!!!\n", __FILE__, __func__, __LINE__);
        }
        
       // input->wait_event = wait_event;
    }

	/**********the function for touch screen************/
	input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

	/******************the function for key************/
    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    input->set_leds = NULL;
    input->wait_event = wait_event;

	screen_width  = __gal_screen->w;
	screen_height = __gal_screen->h;
 	initKeyCode();
	const char *need_reversed = NULL;
	if ((need_reversed = getenv ("MG_TS_REVERSED")) != NULL) {
        reversed = 1;
    }
	fprintf(stderr, "Init_TSLib&KEY_Input finish\n");
    return TRUE;
}

void TermSUNXIKeyTSLibInput (void)
{
    if (fd_key > 0) {
        close(fd_key);
        fd_key = -1;
    }
    if (fd_key_power > 0) {
        close(fd_key_power);
        fd_key_power = -1;
    }
	if (ts) {
        ts_close(ts);
        ts = NULL;
    }
}
#endif
