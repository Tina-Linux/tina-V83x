#include <bluetooth_socket.h>
#include <bluetooth_disc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG_MUSIC_INFO 1

static int last_status = 0;
static int status = 0;
static int playing = 0;
static int disc_cmpl = 0;
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

void bt_disc_event(tBT_DISC_EVT event)
{
    switch(event)
    {
	case BT_DISC_NEW_EVT:
	{
	    printf("Having found new device!\n");
	    break;
	}
	case BT_DISC_CMPL_EVT:
	{
	    printf("End of Discovery!\n");
	    disc_cmpl = 1;
	    break;
	}
	case BT_DISC_DEV_INFO_EVT:
	{
	    printf("Device info discovery event!\n");
	    break;
	}
	case BT_DISC_REMOTE_NAME_EVT:
	{
	    printf("Having read remote device name event!\n");
	    break;
	}
	default:
	    break;
    }
}
/*
int convert_str_to_ad(unsigned char bd_ad[], char *pts)
{
    int i = 0, j = 0;
    unsigned char var, var_lo, var_hg;
    printf("The bd_address is:%02x:%02x:%02x:%02x:%02x:%02x\n", bd_ad[0],bd_ad[1],bd_ad[2],bd_ad[3],bd_ad[4],bd_ad[5]);
    if(strlen(pts) != 17)
    {
        printf("The input address is not suitable!\n");
        return -1;
    }

    for(i = 0; *(pts+i) != '\0'; i++)
    {
        if(*(pts+i) == ':')
                continue;
        if((*(pts+i) >= '0') && (*(pts+i) <= '9'))
        {
                var = *(pts+i) - 48;
        }
        else if((*(pts+i) >= 'A') && (*(pts+i) <= 'F'))
                var = *(pts+i) - 55;
        else if((*(pts+i) >= 'a') && (*(pts+i) <= 'f'))
                var = *(pts+i) - 87;
        else
        {
                printf("ERROR: wrong charactor in address:%c\n",*(pts+i));
        }

        if(i%3 == 0)
        {
            var_hg = var;
        }
        else if(i%3 == 1)
        {
            var_lo = var;
            var = (unsigned char)((int)var_hg*16+(int)var_lo);
            j = i/3;
            bd_ad[j] = (unsigned char)var;
        }
    }

    printf("Get the bd_address:%02x:%02x:%02x:%02x:%02x:%02x!\n", bd_ad[0], bd_ad[1], bd_ad[2], bd_ad[3], bd_ad[4], bd_ad[5]);
}

*/
int main(int argc, char *argv[])
{
    int times = 0;
    int save_fd = -1, fd = -1;
    int ret = 0;
    unsigned char bd_addr[6]={0};  /*The divice mac address to set*/
    tBT_AVK_MUSIC_INFO music_info;
    char discovery_results[DISC_BUFF_LEN] = {0};
    int disc_buff_len = sizeof(discovery_results);
    int opt;
    unsigned char bd_address[6] = {0};  /*set the address of the device to discover/search*/
    char bd_name[BT_NAME_LEN] = {0};
    int is_address_set = 0;  /* whether the adddress is set or not*/
    int is_name_set = 0;  /*whether the name is set or not*/
    int sleep_time = 0;

    while((opt = getopt(argc, argv, "hn:b:")) != EOF)
    {
        switch(opt)
        {
            case 'h':    /* Help message*/
                printf("Usage:opt_test [-h] [-n name] [-b bd_address]!\n");
                return -1;
            case 'n':
                printf("The name input is: %s\n", optarg);
		snprintf(bd_name, BT_NAME_LEN, "%s", optarg);
                printf("The name to discovery is: %s\n", bd_name);
		is_name_set = 1;
                break;
            case 'b':
                printf("The input bd_address is %s\n", optarg);
		if(c.convert_str_to_ad(bd_address,optarg) < 0)
			return -1;
		is_address_set = 1;
                break;
            case '?':
                printf("unrecognized option: -%c",optopt);
        }

    }


    /*Initial discovery parameters*/
    c.initial_disc_param();

    /*set the callback function*/
    c.set_discovery_callback((void *)bt_disc_event);

    if(is_address_set == 1)
        c.set_discovery_bd_addr(bd_address);

    if(is_name_set == 1)
	c.set_discovery_name(bd_name);

    c.set_callback(bt_event_f);

    printf("bt off before on\n");
    c.bt_off();

    last_status = 0;
    status = 0;
    if(argc >= 2){
       c.bt_on(argv[1]);
    } else {
       c.bt_on(NULL);
    }

    c.set_bt_name("aw bt test001");

    if((ret = c.get_bd_addr(bd_addr)) < 0)
	printf("Get bd address fault!\n");
    printf("The bd address is:%02x:%02x:%02x:%02x:%02x:%02x\n",
			bd_addr[0],bd_addr[1],bd_addr[2],
			bd_addr[3],bd_addr[4],bd_addr[5]);

    while(1){
	usleep(2000*1000);

	/*start discovery*/
	c.start_discovery_with_get_rssi();

	while(disc_cmpl != 1 && sleep_time < 30)
	{
	    usleep(1000*1000);
	    sleep_time++;
	}
	sleep_time = 0;

	if(disc_cmpl == 1)
	{
	    c.get_disc_results_with_rssi(discovery_results, &disc_buff_len);
	    printf("=================discovery results========================\n");
	    printf("%s\n",discovery_results);
	    printf("=================  The     end    ========================\n");
	    bzero(discovery_results,sizeof(discovery_results));
	    disc_cmpl = 0;
	}

    }
}
