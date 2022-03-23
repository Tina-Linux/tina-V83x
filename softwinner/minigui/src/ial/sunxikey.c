#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

#ifdef _MGIAL_SUNXIKEY
#include "ial.h"
#include "sunxikey.h"

#define PRESS_INSISTENTLY 1
#define PRESS_QUICKLY 0

#define TIME_USE(end, start)(((end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))/1000)

//static struct input_event key;
static int mouse_x, mouse_y, mouse_button;
static unsigned char state[NR_KEYS];
static int fd_key;
static int fd_key_power;
static int cur_key = -2;

static int mouse_update (void)
{
    return 1;
}


static void mouse_getxy (int* x, int* y)
{
	*x = mouse_x;
    *y = mouse_y;
}

static int mouse_getbutton(void)
{
	return mouse_button;
}

static int castkey2scancode(void)
{
	int i;

	for (i=0; i<SUNXI_KEY_CNT; i++) {
		if(cur_key == key_set[i].keyCode) {
			return key_set[i].scancode;
		}
	}

	return -1;
/*
    switch(cur_key)
    {
	case SUNXI_KEY_LEFT:
		return SCANCODE_SUNXILEFT;
	case SUNXI_KEY_RIGHT:
		return SCANCODE_SUNXIRIGHT;
	case SUNXI_KEY_MODE:
		return SCANCODE_SUNXIMODE;
	case SUNXI_KEY_OK:
		return SCANCODE_SUNXIOK;
	case SUNXI_KEY_POWER:
		return SCANCODE_SUNXIPOWER;
	case SUNXI_KEY_MENU:
		return SCANCODE_SUNXIMENU;
	default:
		return -1;
	}
    */
}
static int unknown_key_down = 0;
static int keyboard_update(void)
{
	
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
					fprintf(stderr, "IAL: unknown key pressed <%d>\n", cur_key);
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
        return (char *) state;
}

static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
    fd_set rfds;
    int e;
	struct input_event key;
	
    if(which & IAL_KEYEVENT)
    {
            if(!in)
            {
                    in = &rfds;
                    FD_ZERO(in);
            }

            FD_SET(fd_key, in);
            if(fd_key > maxfd) maxfd = fd_key;
	    if (fd_key_power > 0) {
		FD_SET(fd_key_power, in);
                if(fd_key_power > maxfd) maxfd = fd_key_power;
	    }
    }	

    e = select (maxfd+1, in, out, except, timeout);
	if (e < 0) {
		return -1;
	}
	if(e > 0)
    {
        if(FD_ISSET(fd_key, in))
        {
                FD_CLR(fd_key, in);
                read(fd_key, &key, sizeof(key));
				//printf("keycode:%d key.value:%d\n", key.code, key.value);
				if (0 == key.code) {
					return -1;
				} else if (key.value > 0){
                    cur_key = key.code;
                } else {
                	if (unknown_key_down) {
						unknown_key_down = 0;
						return -1;
					}
					cur_key = -1;
				}
				return IAL_KEYEVENT;
                
        }
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
            	return IAL_KEYEVENT;
            }
	    }
    }
    return -1;
}

#define STREQUAL(str1, str2) (!strcmp(str1, str2))

static int isKeyName(const char *name)
{
	int i;
	for (i=0; i<SUNXI_KEY_CNT; i++) {
		if(STREQUAL(name, key_set[i].name)) {
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
			key_set[idx].keyCode = iCode;
		}
    }
    fclose(pRead);
}

BOOL InitSUNXIKeyInput(INPUT* input, const char* mdev, const char* mtype)
{
    fd_key = open(mdev, O_RDONLY);
    if (fd_key < 0) {
        _MG_PRINTF ("IAL>%s: Can not open touch screen device: %s!\n", __FILE__, mdev);
        return FALSE;
    }
    fd_key_power = open("/dev/input/event1", O_RDONLY);
    if (fd_key_power < 0) {
        _MG_PRINTF ("IAL>%s: Can not open touch screen device: %s!\n", __FILE__, "/dev/input/event1");
        //return FALSE;
    }
    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    input->set_leds = NULL;

    input->wait_event = wait_event;
    
    mouse_x = 0;
    mouse_y = 0;
    mouse_button= 0;
 	initKeyCode();
    return TRUE;
}

void TermSUNXIKeyInput (void)
{
	close(fd_key);
}
#endif
