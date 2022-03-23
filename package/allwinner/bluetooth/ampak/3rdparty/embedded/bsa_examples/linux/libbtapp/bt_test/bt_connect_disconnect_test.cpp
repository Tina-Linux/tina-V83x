#include <bluetooth_socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static int status = 0;
static int playing = 0;
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

	  default:
	      break;
    }
}


int main(int argc, char *args[]){
    int times = 0;
    int i = 0;

    c.set_callback(bt_event_f);

    printf("bt off before on\n");
    c.bt_off();

    if(argc >= 2){
       c.bt_on(args[1]);
    } else {
       c.bt_on(NULL);
    }

    c.set_bt_name("aw bt connect_disconnect_test");

    while(1){
        /* waiting phone connect to */
        i = 0;
        while(status == 0){
            usleep(100*1000);
            i++;
        }
        printf("wait connected event %d\n", i);

	c.set_dev_discoverable(0);
	c.set_dev_connectable(0);

        printf("Call disconnect\n");
        c.disconnect();

        i = 0;
        while(status == 1){
            usleep(1000);
            i++;
        }
        printf("wait disconnect event %d ms\n", i);

	c.set_dev_discoverable(1);
	c.set_dev_connectable(1);

        printf("Call connect\n");
        c.connect_auto();
    }

}
