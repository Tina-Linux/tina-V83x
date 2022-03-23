#include <bluetooth_socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG_MUSIC_INFO 0

static int last_status = 0;
static int status = 0;
static int playing = 0;
static int hs_calling = 0;
static int call_status = 0;
c_bt c;

void bt_event_f(BT_EVENT event, void *reply, int *len)
{
    switch(event)
    {
	  case BT_AVK_CONNECTED_EVT:
	  {
		  printf("Media audio connected!\n");
		  status = 1;
		  break;
	  }

	  case BT_AVK_DISCONNECTED_EVT:
	  {
		  printf("Media audio disconnected!\n");
          printf("link down reason %d\n", *(int *)reply);
		  status = 0;
		  break;
	  }

	  case BT_AVK_START_EVT:
	  {
		  printf("Media start playing!\n");
                  playing = 1;
		  break;
	  }

	  case BT_AVK_STOP_EVT:
	  {
	      printf("Media stop playing!\n");
              playing = 0;
	      break;
	  }

	  case BT_HS_CONNECTED_EVT:
	  {
	      printf("HS and HF connected!\n");
	      break;
	  }

	  case BT_HS_DISCONNECTED_EVT:
	  {
	      printf("HS and HF disconnected!\n");
	      break;
	  }

	  case BT_HS_RING_EVT:
	  {
	      printf("HS ring call!\n");
	      hs_calling = 1;
	      break;
	  }

	  case BT_HS_OK_EVT:
	  {
	      printf("HS OK evt!\n");
	      break;
	  }
	  default:
	      break;
    }
}


int main(int argc, char *args[]){
    int times = 0;
    int save_fd = -1, fd = -1;
    int ret = 0;
	int rand_num = 1;
	char bt_name[15]={'\0'};
    unsigned char bd_addr[6]={0};
    tBT_AVK_MUSIC_INFO music_info;

    c.set_callback(bt_event_f);

    printf("bt off before on\n");
    c.bt_off();

    last_status = 0;
    status = 0;
    hs_calling = 0;
    call_status = 0;
    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL);
    }
	snprintf(bt_name,14,"aw-bt-test-%d",rand()%90+10);
    c.set_bt_name(bt_name);

    if((ret = c.get_bd_addr(bd_addr)) < 0)
	printf("Get bd address fault!\n");
    printf("The bd address is:%02x:%02x:%02x:%02x:%02x:%02x\n",
			bd_addr[0],bd_addr[1],bd_addr[2],
			bd_addr[3],bd_addr[4],bd_addr[5]);

    while(1){
	usleep(2000*1000);

	/* connected */
	if ((last_status == 0) && (status == 1)){
	    c.set_dev_discoverable(0);
	    c.set_dev_connectable(0);
	    last_status = 1;
	}

	/* disconnected */
	if ((last_status == 1) && (status == 0)){
	    c.set_dev_discoverable(1);
	    c.set_dev_connectable(1);
	    last_status = 0;
	}

	/* calling */
	if ((hs_calling == 1) && (call_status == 0)) {
			c.hs_pick_up();
			hs_calling = 0;
			call_status = 1;
	}

	if ((hs_calling == 1) && (call_status == 1)) {
			c.hs_hung_up();
			hs_calling = 0;
			call_status = 0;
	}

#if(DEBUG_MUSIC_INFO == 1)
	if(playing == 1){
	    c.avk_get_music_info(&music_info);
	    printf("Title: %s\n", music_info.title);
	    printf("Artist: %s\n", music_info.artist);
	    printf("Album: %s\n", music_info.album);
	    //printf("Time: %s\n", music_info.playing_time);
	}
#endif

    }
}
